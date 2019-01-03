
#include <QDebug>
#include <QCompleter>
#include <QAbstractListModel>
#include <QModelIndex>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QPixmap>
#include <QMessageBox>
#include <QMenu>
#include <QProcess>
#include <QLightDM/UsersModel>
#include <QMetaMethod>
#include <QNetworkInterface>
#include <QTimer>
#include "qquickview.h"
#include <QWebEngineView>
#include <QPushButton>
#include <QMovie>
#include <QShortcut>
#include <QToolButton>
#include <QPropertyAnimation>

#include "loginform.h"
#include "ui_loginform.h"
#include "settings.h"
#include "dialog_webview.h"
#include <pwd.h>
#include "settingsform.h"
#include "mainwindow.h"




const int KeyRole = QLightDM::SessionsModel::KeyRole;

int rows(QAbstractItemModel& model) {
    return model.rowCount(QModelIndex());
}

QString displayData(QAbstractItemModel& model, int row, int role)
{
    QModelIndex modelIndex = model.index(row, 0);
    return model.data(modelIndex, role).toString();
}

LoginForm::LoginForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginForm),
    m_Greeter(),
    power(this),
    sessionsModel(),
    userModel(this)

{
    if (!m_Greeter.connectSync()) {
        qCritical() << "Failed to connect lightdm";
        close();
    }


    ui->setupUi(this);
    initialize();
}
LoginForm::~LoginForm()
{
    delete ui;
    delete mv;
}

void LoginForm::setFocus(Qt::FocusReason reason)
{
    if (ui->userInput->text().isEmpty()) {
        ui->userInput->setFocus(reason);
    } else {
        ui->passwordInput->setFocus(reason);

    }
}


void LoginForm::initialize()
{
    ui->hostnameLabel->setText(m_Greeter.hostname());
    mv = new QMovie(":/resources/load1.gif");

    QString imagepath = Settings().logopath();

    if(imagepath.isNull()){

        QPixmap pxmap = QIcon("/usr/share/icons/pardus/48x48/status/avatar-default.svg").pixmap(ui->logolabel->width(),ui->logolabel->height());

        if(pxmap.isNull()){
            pxmap = QIcon("/usr/share/icons/pardus-xfce-icon-theme-dark/48x48/status/avatar-default.svg").pixmap(ui->logolabel->width(),ui->logolabel->height());
        }

        ui->logolabel->setPixmap(pxmap);
    }else{

        QPixmap logoicon(imagepath);
        ui->logolabel->setPixmap(logoicon.scaled(ui->logolabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // setCurrentSession(m_Greeter.defaultSessionHint());

    connect(&m_Greeter, SIGNAL(showPrompt(QString, QLightDM::Greeter::PromptType)), this, SLOT(onPrompt(QString, QLightDM::Greeter::PromptType)));
    connect(&m_Greeter, SIGNAL(showMessage(QString,QLightDM::Greeter::MessageType)), this, SLOT(onMessage(QString,QLightDM::Greeter::MessageType)));
    connect(&m_Greeter, SIGNAL(authenticationComplete()), this, SLOT(authenticationComplete()));

    //ui->passwordInput->setEnabled(false);
    ui->passwordInput->clear();


    initializeUserList();

    checkPasswordResetButton();

    if (! m_Greeter.hideUsersHint()) {
        QLightDM::UsersModel usersModel;
        for (int i = 0; i < usersModel.rowCount(QModelIndex()); i++) {
            knownUsers << usersModel.data(usersModel.index(i, 0), QLightDM::UsersModel::NameRole).toString();
        }

        ui->userInput->setCompleter(new QCompleter(knownUsers));
        ui->userInput->completer()->setCompletionMode(QCompleter::InlineCompletion);
    }

    QString user = Cache().getLastUser();

    if (user.isEmpty()) {
        user = m_Greeter.selectUserHint();
    }
    ui->userInput->setText(user);
    cancelLogin();

    messageReceived = false;
    needPasswordChange = false;
    needReenterOldPassword = false;
    loginStartFlag = false;
    resetStartFlag = false;
    loginTimeot = 0;
    networkOK = false;

    loginTimer = new QTimer();
    resetTimer = new QTimer();
    animationTimer = new QTimer();

    connect(loginTimer, SIGNAL(timeout()), this, SLOT(LoginTimerFinished()));
    connect(resetTimer, SIGNAL(timeout()), this, SLOT(passwordResetTimerFinished()));
    connect(animationTimer, SIGNAL(timeout()), this, SLOT(animationTimerFinished()));

    ui->giflabel->setAttribute(Qt::WA_NoSystemBackground);
    ui->giflabel->setMovie(mv);

    if(Settings().waittimeout() == 0){

        if(Cache().getLastUser() != NULL){
            pageTransition(ui->userspage);

        }else{
            qInfo() << "No cached last user is found. No need to show users page";
            pageTransition(ui->loginpage);
            ui->userInput->setFocus();
        }
    }
    else{
        pageTransition(ui->waitpage);
        qInfo() << "Waiting for network and services";

    }


    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_F8), this);
    QObject::connect(shortcut, SIGNAL(activated()), this, SLOT(on_pwShowbutton_pressed));


    QWidget::setTabOrder(ui->userInput, ui->passwordInput);
    QWidget::setTabOrder(ui->oldpasswordinput, ui->newpasswordinput);
    QWidget::setTabOrder(ui->newpasswordinput, ui->newpasswordconfirminput);



}

void LoginForm::cancelLogin()
{

    if (m_Greeter.inAuthentication()) {
        m_Greeter.cancelAuthentication();
        qWarning() << "Authentication is canceled";
        ui->messagelabel->clear();
    }

}


void LoginForm::startLogin()
{

    if(ui->userInput->text().isEmpty())
        return;

    ui->messagelabel->clear();
    tmpPassword = ui->passwordInput->text();
    oldPassword = tmpPassword;
    ui->passwordInput->setEnabled(false);
    ui->userInput->setEnabled(false);

    loginTimer->setTimerType(Qt::TimerType::CoarseTimer);
    loginTimer->setSingleShot(false);
    loginTimer->setInterval(10);
    loginTimerState = 0;
    loginTimer->start();

}


void LoginForm::onPrompt(QString prompt, QLightDM::Greeter::PromptType promptType)
{
    promptFlag = true;
    lastPrompt = prompt;

    //"password: "
    //"Enter new password: "
    //"Enter it again: "

    qInfo() << "Received Prompt: " + prompt + " type: " + QString::number(promptType);

    if((prompt.compare("Enter new password: ") == 0 || prompt.compare("New password: ") == 0 || prompt.compare("(current) UNIX password: ") == 0 || prompt.compare("Current Password: ") == 0) && needPasswordChange != 1 && !resetStartFlag){

        if(prompt.compare("(current) UNIX password: " ) == 0 || prompt.compare("Current Password: ") == 0)
            needReenterOldPassword = 1;

        needPasswordChange = true;
        pageTransition(ui->warningpage);
        oldPassword = tmpPassword;
        ui->warninglabel->setText(tr("Your Password is Expired. You have to change it"));
        ui->newpasswordinput->setEnabled(true);
        ui->newpasswordconfirminput->setEnabled(true);
        ui->newpasswordinput->clear();
        ui->newpasswordconfirminput->clear();
        ui->oldpasswordinput->setText(oldPassword);
        ui->oldpasswordinput->setEnabled(true);
        ui->acceptbutton->setFocus();

    }
    else if((prompt.compare("Enter new password: ") == 0 || prompt.compare("New password: ") == 0 )&& resetStartFlag && !needReenterOldPassword){

        needPasswordChange = true;
        pageTransition(ui->resetpage);
        ui->oldpasswordinput->setEnabled(true);
        ui->newpasswordinput->setEnabled(true);
        ui->newpasswordconfirminput->setEnabled(true);
        ui->newpasswordinput->clear();
        ui->newpasswordconfirminput->clear();
        ui->newpasswordinput->setFocus();

        resetStartFlag = false;
        loginStartFlag = false;
    }
}

void LoginForm::onMessage(QString prompt, QLightDM::Greeter::MessageType messageType){
    QString type = NULL;

    qInfo() << "Received Message: " + prompt + " type: " + QString::number(messageType);

    if(messageType == QLightDM::Greeter::MessageTypeError){
        type = tr("Error");
    }
    else if(messageType == QLightDM::Greeter::MessageTypeInfo){
        type = tr("Info");
    }
    else{
        type = " ";
    }

    if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage)){
        messageReceived = true;
        ui->messagelabel->setText(type + " : " + prompt);
        ui->warninglabel->setText(type + " : " + prompt);
    }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->warningpage)){
        ui->warninglabel->setText(type + " : " + prompt);
    }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->resetpage) || resetStartFlag){
        ui->rstpwdmessagelabel->setText(type + " : " + prompt);
    }else{
        ui->messagelabel->setText(type + " : " + prompt);

    }

}


QString LoginForm::currentSession()
{
    QModelIndex index = sessionsModel.index(0, 0, QModelIndex());
    return sessionsModel.data(index, QLightDM::SessionsModel::KeyRole).toString();
}


void LoginForm::setCurrentSession(QString session)
{
#if 0
    for (int i = 0; i < ui->sessionCombo->count(); i++) {
        if (session == sessionsModel.data(sessionsModel.index(i, 0), KeyRole).toString()) {
            ui->sessionCombo->setCurrentIndex(i);
            return;
        }
    }
#endif
}


void LoginForm::authenticationComplete()
{

    if (m_Greeter.isAuthenticated()) {

        qInfo() << "Authentication is completed for  " + ui->userInput->text();

        needPasswordChange = 0;
        Cache().setLastUser(ui->userInput->text().trimmed());
        Cache().setLastSession(ui->userInput->text(), currentSession());
        addUsertoCache(ui->userInput->text().trimmed());
        Cache().sync();
        m_Greeter.startSessionSync(currentSession());
        needPasswordChange = false;

    }else if(loginStartFlag == true || resetStartFlag == true) {


        qWarning() <<  "Authentication error for  " + ui->userInput->text();

        if(loginStartFlag)
            pageTransition(ui->loginpage);

        if(resetStartFlag)
            pageTransition(ui->resetpage);

        ui->passwordInput->clear();

        if(!needPasswordChange)
            cancelLogin();

        if(!messageReceived)
            ui->messagelabel->setText(tr("Error : Login Incorrect"));

        if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage)){
            ui->passwordInput->setEnabled(true);
            ui->userInput->setEnabled(true);
            ui->passwordInput->setFocus();
            ui->passwordInput->clear();
        }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->resetpage)){
            ui->newpasswordinput->clear();
            ui->newpasswordconfirminput->clear();
            ui->newpasswordinput->setEnabled(true);
            ui->newpasswordconfirminput->setEnabled(true);
            ui->oldpasswordinput->setEnabled(true);
            ui->resetpasswordButton->setEnabled(true);
            ui->newpasswordinput->setFocus();
            needPasswordChange = false;
        }
    }


    messageReceived = false;
    loginStartFlag = false;
    resetStartFlag = false;


}

void LoginForm::addUsertoCache(QString user_name){


    int i;
    int nullfound = 0;

    for(i = 0; i< 5; i++){
        if(userList[i].compare(user_name) == 0)
            userList[i].clear();

        if(i >= Settings().cachedusercount())
            userList[i].clear();

        if(userList[i].isNull() || userList[i].isEmpty())
            nullfound++;
    }

    if(userList[0].isNull() || userList[0].isEmpty()){
        userList[0] = user_name;
    }else{

        if(nullfound > 0){
            for(i = 4; i >= 0; i--){
                if((userList[i].isNull() || userList[i].isEmpty()) && i != 0){

                    userList[i] = userList[i-1];
                    userList[i-1].clear();

                }

            }

            userList[0] = user_name;

        }else{
            userList[4] = userList[3];
            userList[3] = userList[2];
            userList[2] = userList[1];
            userList[1] = userList[0];
            userList[0] = user_name;
        }

    }

    for(i = 0; i< Settings().cachedusercount(); i++){
        if(!userList[i].isNull() && !userList[i].isEmpty())
            Cache().setLastUsertoIndex(userList[i], i);
        else
            Cache().setLastUsertoIndex("", i);

    }


}


void LoginForm::on_pushButton_resetpwd_clicked()
{
    qInfo() << "Password reset webpage is opening";

    Dialog_webview dwview;
    dwview.setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);

    //dwview.setModal(true);
    dwview.exec();
}




void LoginForm::checkPasswordResetButton(){

    uint password_key_selection = 0;

    QSettings greeterSettings(CONFIG_FILE, QSettings::IniFormat);

    if (greeterSettings.contains(PASSWORD_WEB_RESET_KEY)) {
        password_key_selection = greeterSettings.value(PASSWORD_WEB_RESET_KEY).toUInt();

    }

    if(!password_key_selection)
    {
        ui->pushButton_resetpwd->setVisible(false);
        ui->pushButton_resetpwd->setEnabled(false);
    }

}


void LoginForm::initializeUserList(){

    total_user_count = 0;

    for(int i = 0; i < Settings().cachedusercount(); i++){
        userList[i] = Cache().getLastUserfromIndex(i);

        if(!userList[i].isNull() && !userList[i].isEmpty() )
            total_user_count++;
    }



    for(int i = 0; i < 5; i++){
        if((userList[i].isNull() || userList[i].isEmpty()) && i < 4){

            userList[i] = userList[i + 1];
            userList[i + 1].clear();

        }

    }

    qInfo() <<  (QString::number(total_user_count) + " users found for last users cache");


    QString username;
    QString imagepath;
    QModelIndex modelIndex;

    for(int i = 0; i< total_user_count; i++){

        for(int j = 0; j < userModel.rowCount(QModelIndex()); j++){

            modelIndex = userModel.index(j, 0);
            username = userModel.data(modelIndex, QLightDM::UsersModel::NameRole).toString();

            if(username.compare(userList[i]) == 0){
                imagepath = userModel.data(modelIndex, QLightDM::UsersModel::ImagePathRole).toString();
            }


        }


        toolButtons[i] = new QToolButton(ui->usersframe);

        toolButtons[i]->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        if(imagepath.isNull() || imagepath.isEmpty()){


            QPixmap pxmap = QIcon("/usr/share/icons/pardus/48x48/status/avatar-default.svg").pixmap(ui->logolabel->width(),ui->logolabel->height());

            if(pxmap.isNull()){
                pxmap = QIcon("/usr/share/icons/pardus-xfce-icon-theme-dark/48x48/status/avatar-default.svg").pixmap(ui->logolabel->width(),ui->logolabel->height());
            }

            toolButtons[i]->setIcon(pxmap);

        }else{
            QPixmap iconx(imagepath);
            toolButtons[i]->setIcon(iconx);
        }



        QSize sz;
        sz.setHeight(100);
        sz.setWidth(100);
        toolButtons[i]->setIconSize(sz);

        toolButtons[i]->setText(userList[i]);
        toolButtons[i]->setObjectName(QString("toolbutton%1").arg(i));

        connect(toolButtons[i], SIGNAL(clicked()), this, SLOT(userButtonClicked()));
    }


    toolButtons[0]->setStyleSheet("background-color:rgba(200, 200, 200,0.9);\ncolor:black;\nborder:2px solid rgb(255,202,8);");

    usersbuttonReposition();

    current_user_button = 1;
    currentUserIndex = 0;


}


void LoginForm::usersbuttonReposition(){


    int middlepoint = ui->usersframe->width() / 2;

    int defaultframelength = ui->usersframe->width() / 5;

    int totalframelength = defaultframelength * total_user_count;

    int framestart = middlepoint - (totalframelength / 2);

    int buttonx = 0;



    centralButtonPoint.setX(middlepoint - 60);
    //centralButtonPoint.y;



    for(int i = 0; i< total_user_count; i++){

        buttonx = middlepoint - 60;
        toolButtons[i]->setText(userList[i]);
        toolButtons[i]->setGeometry(buttonx ,100, 60, 75);
        toolButtons[i]->show();

        anim1[i] = new QPropertyAnimation(toolButtons[i], "geometry");
        anim1[i]->setDuration(250);
        anim1[i]->setStartValue(QRect(buttonx + 20 ,100, 80, 100));



    }


    animGroup = new QParallelAnimationGroup;

    for(int i = 0; i< total_user_count; i++){


        buttonx = framestart + ((defaultframelength - 120) / 2) + (i * defaultframelength);

        //toolButtons[i]->setGeometry(buttonx ,100, 120, 150);


        anim1[i]->setEndValue(QRect(buttonx ,100, 120, 150));

        animGroup->addAnimation(anim1[i]);



    }

    animGroup->start();

}


void LoginForm::userButtonClicked(){


    int i = 0;

    QObject *senderObj = sender(); // This will give Sender object
    QString senderObjName = senderObj->objectName();

    for(i = 0; i< total_user_count; i++){

        if(senderObjName.compare(QString("toolbutton%1").arg(i)) == 0){
            break;
        }

    }


    int middlepoint = ui->usersframe->width() / 2;
    int buttonx = middlepoint - 60;

    animGroup->clear();
    animGroup = new QParallelAnimationGroup;

    for(int j = 0; j < total_user_count; j++){

        //delete anim1[j];

        anim1[j] = new QPropertyAnimation(toolButtons[j], "geometry");

        anim1[j]->setDuration(150);

        if(j == i){
            anim1[j]->setStartValue(QRect(toolButtons[j]->x(), toolButtons[j]->y(), toolButtons[j]->width(), toolButtons[j]->height()));
            anim1[j]->setEndValue(QRect(buttonx ,100, 120, 150));

        }else{

            anim1[j]->setStartValue(QRect(toolButtons[j]->x(), toolButtons[j]->y(), toolButtons[j]->width(), toolButtons[j]->height()));
            anim1[j]->setEndValue(QRect(buttonx ,100, 120, 150));

        }


        animGroup->addAnimation(anim1[j]);

    }

    animGroup->start();
    currentUserIndex = i;

    for(int i = 0; i< total_user_count; i++){

        if(i != currentUserIndex){
            toolButtons[i]->setStyleSheet("background-color:rgba(50, 50, 50,0.7)");
        }else{
            toolButtons[currentUserIndex]->setStyleSheet("background-color:rgba(200, 200, 200,0.9);\ncolor:black;\nborder:2px solid rgb(255,202,8);");
        }
    }




    animationTimer->setTimerType(Qt::TimerType::CoarseTimer);
    animationTimer->setSingleShot(false);
    animationTimer->setInterval(150);
    animationTimerState = 0;
    animationTimer->start();


}


void LoginForm::animationTimerFinished(){

    switch(animationTimerState){
    case 0:

        animGroup->clear();

        for(int i = 0; i< total_user_count; i++){

            if(i != currentUserIndex)
                toolButtons[i]->hide();
            else
                toolButtons[i]->setStyleSheet("background-color:rgba(50, 50, 50,0.7)");

            toolButtons[i]->setText("");
        }

        animationTimerState++;
        animationTimer->stop();
        animationTimer->start();

        anim1[0] = new QPropertyAnimation(toolButtons[currentUserIndex], "geometry");
        anim1[0]->setDuration(150);

        anim1[0]->setStartValue(QRect(toolButtons[currentUserIndex]->x(), toolButtons[currentUserIndex]->y(), toolButtons[currentUserIndex]->width(), toolButtons[currentUserIndex]->height()));
        anim1[0]->setEndValue(QRect(ui->loginframe->x(), ui->loginframe->y(), ui->loginframe->width(), ui->loginframe->height()));
        anim1[0]->start();
        break;

    case 1:

        pageTransition(ui->loginpage);
        ui->userspage->setFocus();
        userSelectStateMachine(0, currentUserIndex);
        ui->passwordInput->setFocus();
        capsLockCheck();
        animationTimerState = 0;
        animationTimer->stop();

        break;

    case 2:
        break;
    }

}


void LoginForm::userSelectStateMachine(int key, int button){


    if(key == Qt::Key_Left)
        currentUserIndex--;
    else if(key == Qt::Key_Right)
        currentUserIndex++;

    if(currentUserIndex < 0)
        currentUserIndex = 0;

    if(currentUserIndex >= total_user_count)
        currentUserIndex = total_user_count - 1;

    if(button >= total_user_count)
        button = total_user_count - 1;

    if(button >= 0)
        currentUserIndex  = button;


    for(int i = 0; i< total_user_count; i++){

        toolButtons[i]->setStyleSheet("background-color:rgba(50, 50, 50,0.7)");
    }

    toolButtons[currentUserIndex]->setStyleSheet("background-color:rgba(200, 200, 200,0.9);\ncolor:black;\nborder:2px solid rgb(255,202,8);");

    ui->userInput->setText(userList[currentUserIndex]);
    ui->usersframe->setFocus();

    ui->passwordInput->setFocus();
}


void LoginForm::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->userspage)){
           // pageTransition(ui->loginpage);
            //ui->passwordInput->setFocus();
            //capsLockCheck();

            animationTimer->setTimerType(Qt::TimerType::CoarseTimer);
            animationTimer->setSingleShot(false);
            animationTimer->setInterval(150);
            animationTimerState = 0;
            animationTimer->start();

        }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->resetpage))
            on_resetpasswordButton_clicked();
        else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->warningpage)){
            on_acceptbutton_clicked();
        }
        else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage)){

            if (ui->userInput->text().isEmpty()){
                ui->userInput->setFocus();
            }else if( ui->userInput->hasFocus()){
                ui->passwordInput->setFocus();
            }else{
                startLogin();
            }

        }
    }
    else if (event->key() == Qt::Key_Escape) {

        if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->userspage)){
            cancelLogin();
            pageTransition(ui->loginpage);
            ui->userInput->clear();
            ui->passwordInput->clear();
            ui->userInput->setFocus();
            capsLockCheck();
            needPasswordChange = 0;

        }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage) /*&& !loginStartFlag*/){

            needPasswordChange = 0;
            cancelLogin();

            if(Cache().getLastUser() != NULL){
                pageTransition(ui->userspage);
                userSelectStateMachine(0, currentUserIndex);
            }

        }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->resetpage) && !resetStartFlag){
            needPasswordChange = 0;
            cancelLogin();
            pageTransition(ui->loginpage);
            ui->newpasswordconfirminput->clear();
            ui->newpasswordinput->clear();
            ui->oldpasswordinput->clear();
            ui->userInput->setEnabled(true);
            ui->passwordInput->setEnabled(true);
            ui->userInput->setFocus();
            ui->passwordInput->clear();
            capsLockCheck();
            ui->rstpwdmessagelabel->clear();
        }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->waitpage) && loginStartFlag && !resetStartFlag){
            // m_Greeter.cancelAuthentication();
            //pageTransition(ui->loginpage);

        }
    }
    else if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right && ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->userspage)) {
        userSelectStateMachine(event->key(), -1);
    }else {
        QWidget::keyPressEvent(event);
    }

}


void LoginForm::keyReleaseEvent(QKeyEvent *event){

    if(event->key() == Qt::Key_CapsLock){
        QWidget::keyReleaseEvent(event);
        capsLockCheck();
    }else{
        QWidget::keyReleaseEvent(event);
    }

}


void LoginForm::on_resetpasswordButton_clicked()
{
    ui->rstpwdmessagelabel->clear();

    if(ui->oldpasswordinput->text().isEmpty()){
        ui->rstpwdmessagelabel->setText(tr("Old password is wrong"));
        return;
    }

    if(ui->newpasswordinput->text().isEmpty() || ui->newpasswordconfirminput->text().isEmpty()){
        ui->rstpwdmessagelabel->setText(tr("New passwords are not same"));
        return;
    }

    if(ui->oldpasswordinput->text().compare(oldPassword)){

        ui->rstpwdmessagelabel->setText(tr("Old password is wrong"));

    }else if(ui->newpasswordconfirminput->text().compare(ui->newpasswordinput->text())){

        ui->rstpwdmessagelabel->setText(tr("New passwords are not same"));

    }else{
        /* everything is ok */
        resetTimer->setTimerType(Qt::TimerType::CoarseTimer);

        resetTimer->setSingleShot(false);

        if(needPasswordChange == false){
            cancelLogin();
            needPasswordChange = true;
            resetTimerState = 0;
            resetTimer->setInterval(10);
            qInfo() << "Reset password operation started again for " + ui->userInput->text().trimmed();


        }else if(needReenterOldPassword){
            //needReenterOldPassword = 0;
            resetTimerState = 3;
            resetTimer->setInterval(10);
            qInfo() << "Reset password operation started for " + ui->userInput->text().trimmed();
        }else{

            resetTimerState = 4;
            qInfo() <<  "Reset password operation started for " + ui->userInput->text().trimmed();
        }

        resetTimer->start();

        promptFlag = 0;
        ui->resetpasswordButton->setEnabled(false);
        ui->newpasswordinput->setEnabled(false);
        ui->newpasswordconfirminput->setEnabled(false);
        ui->oldpasswordinput->setEnabled(false);
    }
}

void LoginForm::LoginTimerFinished(){

    bool endFlag = 0;
    static int promptCheckCounter;
    static int userCheckCounter;


    switch(loginTimerState){

    case 0:

        qInfo() << "Login start for " + ui->userInput->text().trimmed();

        ui->waitlabel->setText(tr("Authenticating"));
        pageTransition(ui->waitpage);
        cancelLogin();
        userCheckCounter = 0;
        loginTimerState = 1;
        lastPrompt.clear();
        break;

    case 1:
        //enter user
        m_Greeter.authenticate(ui->userInput->text().trimmed());
        loginTimerState = 2;
        promptCheckCounter = 0;
        lastPrompt.clear();
        break;

    case 2:
        if(lastPrompt.compare("Password: ") == 0 || lastPrompt.compare("password: ") == 0){
            m_Greeter.respond(ui->passwordInput->text());
            endFlag = 1;
        }else if(promptCheckCounter > 200){
            pageTransition(ui->loginpage);
            endFlag = 1;
            ui->passwordInput->setEnabled(true);
            ui->userInput->setEnabled(true);
            ui->passwordInput->setFocus();
            ui->passwordInput->clear();
            cancelLogin();
            messageReceived = 0;
            loginStartFlag = false;

            if(networkOK){
                ui->messagelabel->setText(tr("Error : User is unknown"));
            }else{
                ui->messagelabel->setText(tr("Error : Network is not connected"));
            }

            qWarning() << "No password prompt received";
            qWarning() << "Authentication is canceled";

        }else{
            promptCheckCounter++;
        }
        break;
    }

    if(endFlag == 1){
        loginTimerState = 0;
        loginTimer->stop();
    }else{
        loginTimer->stop();
        loginTimer->setInterval(10);
        loginTimer->start();
        loginStartFlag = true;
        promptFlag = false;
    }
}

void LoginForm::passwordResetTimerFinished(){

    bool endFlag = 0;
    static int opcheckcounter;

    switch(resetTimerState){

    case 0:
        pageTransition(ui->waitpage);
        cancelLogin();
        resetTimerState = 1;
        break;

    case 1:
        //enter user
        m_Greeter.authenticate(ui->userInput->text().trimmed());
        resetTimerState = 2;
        break;

    case 2:
        //enter password
        m_Greeter.respond(oldPassword);
        resetTimerState = 3;
        break;

    case 3:
        //old password
        pageTransition(ui->waitpage);
        if(lastPrompt.compare("(current) UNIX password: ") == 0 || lastPrompt.compare("Current Password: ") == 0)
            m_Greeter.respond(oldPassword);

        resetTimerState = 4;
        break;

    case 4:
        //"Enter new password: "
        pageTransition(ui->waitpage);
        m_Greeter.respond(ui->newpasswordinput->text());
        resetTimerState = 5;
        break;

    case 5:
        //"Enter it again: "
        m_Greeter.respond(ui->newpasswordinput->text());
        //resetTimerState = 0;
        resetTimerState = 6;
        opcheckcounter = 0;

        // endFlag = 1;
        break;

    case 6:
        if(opcheckcounter < 6){

            opcheckcounter++;
            resetTimerState = 6;

        }else{
            opcheckcounter = 0;
            endFlag = true;
            resetStartFlag = false;
            loginStartFlag = false;

            //pageTransition(ui->loginpage);

        }
        break;

    }

    if(endFlag == true){

        resetTimerState = 0;
        resetTimer->stop();
    }else{
        resetTimer->stop();
        resetTimer->setInterval(500);
        resetTimer->start();
        resetStartFlag = true;
    }

}

void LoginForm::on_acceptbutton_clicked()
{
    pageTransition(ui->resetpage);
    ui->newpasswordinput->setFocus();

}


QString LoginForm::getValueOfString(QString data, QString value){

    QString result;

    int indx = data.indexOf(value);
    int endx = 0;

    if(indx == -1){
        return NULL;
    }

    indx += value.size();
    result = data.mid(indx);

    if(result.at(0) != QChar(':') && result.at(0) != QChar('=') ){

        if(result.at(1) != QChar(':') && result.at(1) != QChar('=') )
            return NULL;
        else
            indx = 2;

    }else{
        indx = 1;
    }


    while(result.at(indx) == ' ' || result.at(indx) == '\n'){
        indx++;
    }

    endx = indx;

    while(result.at(endx) != ' ' && result.at(endx) != '\n'){
        endx++;
    }



    result = result.mid(indx, (endx - indx));

    return result;
}


bool LoginForm::capsOn()
{
    FILE *fp;
    char data[128];
    bool ret = false;


    fp = popen("xset q | grep Caps", "r");
    if (fp == NULL) {
        qWarning() << "Unable to open capslock status" ;
        return false;
    }


    if(fread(data, sizeof(data), 1, fp) < 0){
        qWarning() << " Unable to read capslock status" ;

        /* close */
        pclose(fp);
        return false;
    }

    if(getValueOfString(QString::fromLocal8Bit(data), QString("Caps Lock")).compare("off") == 0)
        ret = false;
    else
        ret = true;

    pclose(fp);
    return ret;

}

void LoginForm::on_userInput_editingFinished()
{

    capsLockCheck();

}


void LoginForm::capsLockCheck(){


    //check caps lock position
    if(capsOn()){
        if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage))
            ui->messagelabel->setText(tr("Caps Lock is on"));
        else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->resetpage))
            ui->rstpwdmessagelabel->setText(tr("Caps Lock is on"));
    }else{


        if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage)){
            if(ui->messagelabel->text().compare(tr("Caps Lock is on")) == 0)
                ui->messagelabel->clear();
        }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->resetpage)){
            if(ui->rstpwdmessagelabel->text().compare(tr("Caps Lock is on")) == 0)
                ui->rstpwdmessagelabel->clear();
        }


    }

}

void LoginForm::stopWaitOperation(const bool& networkstatus){


    networkOK = networkstatus;

    if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->waitpage) && !loginStartFlag && !resetStartFlag){

        if(networkstatus == false && loginTimeot < Settings().waittimeout()){
            loginTimeot += 5;
        }else{

            if(networkstatus == false)
                timeoutFlag = true;
            else
                timeoutFlag = false;

            if(Cache().getLastUser() != NULL){
                pageTransition(ui->userspage);
                ui->userspage->setFocus();
                ui->userspage->setFocusPolicy(Qt::FocusPolicy::WheelFocus);

            }else{
                pageTransition(ui->loginpage);
            }

            loginTimeot = 0;
        }

    }else if(ui->stackedWidget->currentIndex() != ui->stackedWidget->indexOf(ui->waitpage) && ui->stackedWidget->currentIndex() != ui->stackedWidget->indexOf(ui->userspage) && networkstatus == false
             && Settings().waittimeout() > 0 && !timeoutFlag &&!loginStartFlag && !resetStartFlag){
        ui->waitlabel->setText(tr("Waiting for Network and Services"));
        pageTransition(ui->waitpage);
        cancelLogin();
        loginTimeot = 0;
        loginStartFlag = false;
        resetStartFlag = false;

    }

}


void LoginForm::on_loginbutton_clicked()
{
    startLogin();
}


void LoginForm::pageTransition(QWidget *Page){


    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(Page));


    if(Page == ui->waitpage){
        ui->waitlabel->setFocus();
        mv->start();
    }
    else if(Page == ui->userspage){
        usersbuttonReposition();
        ui->userspage->setFocus();
        ui->userspage->setFocusPolicy(Qt::FocusPolicy::WheelFocus);
        ui->passwordInput->setEnabled(false);
        ui->userInput->setEnabled(false);
        mv->stop();
    }
    else if(Page == ui->loginpage){


        // ui->logolabel->move(centralButtonPoint);


        if(ui->userInput->text().isEmpty())
            ui->userInput->setFocus();
        else
            ui->passwordInput->setFocus();

        ui->passwordInput->setEnabled(true);
        ui->userInput->setEnabled(true);

        ui->passwordInput->clear();
        ui->messagelabel->clear();

        mv->stop();
    }
    else if(Page == ui->warningpage){

        mv->stop();
    }
    else if(Page == ui->resetpage){

        mv->stop();
    }


}


void LoginForm::on_pwShowbutton_pressed()
{
    ui->passwordInput->setEchoMode(QLineEdit::EchoMode::Normal);
}

void LoginForm::on_pwShowbutton_released()
{
    ui->passwordInput->setEchoMode(QLineEdit::EchoMode::Password);
    ui->pwShowbutton->clearFocus();

}

void LoginForm::on_backButton_clicked()
{
    needPasswordChange = 0;
    cancelLogin();

    if(Cache().getLastUser() != NULL){
        pageTransition(ui->userspage);
        userSelectStateMachine(0, currentUserIndex);
    }
}
