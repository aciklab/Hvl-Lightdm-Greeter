
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
    sessionsModel()

{
    if (!m_Greeter.connectSync()) {
        close();
    }


    ui->setupUi(this);
    initialize();
}
LoginForm::~LoginForm()
{
    delete ui;
    delete user_frame;
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
    QPixmap icon(":/resources/pardus.png");
    ui->iconLabel->setPixmap(icon.scaled(ui->iconLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->hostnameLabel->setText(m_Greeter.hostname());
    mv = new QMovie(":/resources/load1.gif");

    QString imagepath = Settings().logopath();

    if(imagepath.isNull()){
        ui->logolabel->setPixmap(icon.scaled(ui->logolabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }else{

        QPixmap logoicon(imagepath);
        ui->logolabel->setPixmap(logoicon.scaled(ui->logolabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    setCurrentSession(m_Greeter.defaultSessionHint());

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

    loginTimer = new QTimer();
    resetTimer = new QTimer();

    connect(loginTimer, SIGNAL(timeout()), this, SLOT(LoginTimerFinished()));
    connect(resetTimer, SIGNAL(timeout()), this, SLOT(passwordResetTimerFinished()));

    ui->giflabel->setAttribute(Qt::WA_NoSystemBackground);
    ui->giflabel->setMovie(mv);

    if(Settings().waittimeout() == 0){

        if(Cache().getLastUser() != NULL){
            pageTransition(ui->userspage);

        }else{
            pageTransition(ui->loginpage);
        }
    }
    else{
        pageTransition(ui->waitpage);

    }

}

void LoginForm::cancelLogin()
{

    ui->messagelabel->clear();


    if (m_Greeter.inAuthentication()) {
        m_Greeter.cancelAuthentication();
    }


}


void LoginForm::startLogin()
{
    if (ui->userInput->text().isEmpty() || ui->passwordInput->text().isEmpty())
        return;

    ui->messagelabel->clear();
    tmpPassword = ui->passwordInput->text().trimmed();
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

    if((prompt.compare("Enter new password: ") == 0 || prompt.compare("(current) UNIX password: ") == 0 ) && needPasswordChange != 1 && !resetStartFlag){

        if(prompt.compare("(current) UNIX password: ") == 0)
            needReenterOldPassword = 1;

        needPasswordChange = 1;
        pageTransition(ui->warningpage);
        oldPassword = tmpPassword;
        ui->warninglabel->setText(tr("Your Password is Expired. You have to change it"));
        ui->newpasswordinput->setEnabled(true);
        ui->newpasswordconfirminput->setEnabled(true);
        ui->newpasswordinput->clear();
        ui->newpasswordconfirminput->clear();
        ui->oldpasswordinput->setText(oldPassword.trimmed());
        ui->oldpasswordinput->setEnabled(true);
        ui->acceptbutton->setFocus();

    }
    else if(prompt.compare("Enter new password: ") == 0 && resetStartFlag && !needReenterOldPassword){

        needPasswordChange = 1;
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
        messageReceived = 1;
        ui->messagelabel->setText(type + " : " + prompt);
        ui->warninglabel->setText(type + " : " + prompt);
    }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->warningpage)){
        ui->warninglabel->setText(type + " : " + prompt);
    }else{
        ui->rstpwdmessagelabel->setText(type + " : " + prompt);
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
        needPasswordChange = 0;
        Cache().setLastUser(ui->userInput->text());
        Cache().setLastSession(ui->userInput->text(), currentSession());
        addUsertoCache(ui->userInput->text());
        Cache().sync();
        m_Greeter.startSessionSync(currentSession());
        loginStartFlag = false;
        resetStartFlag = false;
        messageReceived = false;
        needPasswordChange = false;

    }else if(loginStartFlag == true || resetStartFlag == true) {

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

        messageReceived = 0;
        loginStartFlag = false;
        resetStartFlag = false;


    }


}

void LoginForm::addUsertoCache(QString user_name){

    int i;
    int nullfound = 0;

    for(i = 0; i< 5; i++){
        if(userList[i].trimmed().compare(user_name) == 0)
            userList[i].clear();

        if(i >= Settings().cachedusercount())
            userList[i].clear();

        if(userList[i].isNull() || userList[i].isEmpty())
            nullfound++;
    }

    if(userList[0].isNull()){
        userList[0] = user_name;
    }else{

        if(nullfound > 0){
            for(i = 4; i >= 0; i--){
                if(userList[i].isNull() && i != 0){

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
        if(!userList[i].isNull())
            Cache().setLastUsertoIndex(userList[i], i);

    }

}


void LoginForm::on_pushButton_resetpwd_clicked()
{
    Dialog_webview dwview;
    dwview.setModal(true);
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

        if(!userList[i].isNull())
            total_user_count++;
    }

    ui->userbutton1->setText("");
    ui->userbutton1->setEnabled(false);

    if(userList[0] != NULL){

        ui->userbutton2->setText(" " + Cache().getLastUserfromIndex(0));
    }
    else if(Cache().getLastUser() != NULL)
    {
        ui->userbutton2->setText(" " + Cache().getLastUser());
    }
    else{
        ui->userbutton2->setText("");
        ui->userbutton2->hide();
    }


    if(userList[1] != NULL){

        ui->userbutton3->setText(" " + Cache().getLastUserfromIndex(1));
    }
    else
    {
        ui->userbutton3->setText("");
        ui->userbutton3->setEnabled(false);

    }


    current_user_button = 1;
    currentUserIndex = 0;


}


void LoginForm::userSelectStateMachine(int key, int button){


    if(key == Qt::Key_Up)
        currentUserIndex--;
    else if(key == Qt::Key_Down)
        currentUserIndex++;

    if(currentUserIndex < 0)
        currentUserIndex = 0;

    if(currentUserIndex >= total_user_count)
        currentUserIndex = total_user_count - 1;

    if(button >= total_user_count)
        button = total_user_count - 1;

    if(button >= 0)
        currentUserIndex  = button;

    ui->usersframe->setFocus();


    switch(currentUserIndex){
    case 0:

        if(!userList[0].isNull()){
            ui->userbutton1->setEnabled(false);
            ui->userbutton1->setText("");
            ui->userbutton2->setText(" " + userList[0]);
            ui->userbutton3->setText(" " + userList[1]);
        }

        if(userList[1].isNull())
            ui->userbutton3->setEnabled(false);
        else
            ui->userbutton3->setEnabled(true);

        break;

    case 1:
        if(!userList[1].isNull()){
            ui->userbutton1->setEnabled(true);
            ui->userbutton1->setText(" " + userList[0]);
            ui->userbutton2->setText(" " + userList[1]);
            ui->userbutton3->setText(" " + userList[2]);
        }

        if(userList[2].isNull())
            ui->userbutton3->setEnabled(false);
        else
            ui->userbutton3->setEnabled(true);

        break;

    case 2:
        if(!userList[2].isNull()){
            ui->userbutton1->setText(" " + userList[1]);
            ui->userbutton2->setText(" " + userList[2]);
            ui->userbutton3->setText(" " + userList[3]);
        }


        if(userList[3].isNull())
            ui->userbutton3->setEnabled(false);
        else
            ui->userbutton3->setEnabled(true);

        break;

    case 3:

        if(!userList[3].isNull()){
            ui->userbutton1->setText(" " + userList[2]);
            ui->userbutton2->setText(" " + userList[3]);
            ui->userbutton3->setText(" " + userList[4]);
        }

        if(userList[4].isNull())
            ui->userbutton3->setEnabled(false);
        else
            ui->userbutton3->setEnabled(true);

        break;

    case 4:
        if(!userList[4].isNull()){
            ui->userbutton1->setText(" " + userList[3]);
            ui->userbutton2->setText(" " + userList[4]);
            ui->userbutton3->setText("");
            ui->userbutton3->setEnabled(false);

        }

        break;
    }


    ui->userInput->setText(ui->userbutton2->text().trimmed());
    ui->passwordInput->setFocus();
}


void LoginForm::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->userspage)){
            pageTransition(ui->loginpage);
            capsLockCheck();
        }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->resetpage))
            on_resetpasswordButton_clicked();
        else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->warningpage)){
            on_acceptbutton_clicked();
        }
        else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage)){
            startLogin();
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

        }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage) && !loginStartFlag){

            needPasswordChange = 0;
            cancelLogin();

            if(Cache().getLastUser() != NULL){
                pageTransition(ui->userspage);
                ui->userInput->clear();
                ui->passwordInput->clear();
                userSelectStateMachine(0,currentUserIndex);
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
        }
    }
    else if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down && ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->userspage)) {
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

void LoginForm::on_userbutton1_clicked()
{
    userSelectStateMachine(0,currentUserIndex - 1);

}

void LoginForm::on_userbutton2_clicked()
{
    pageTransition(ui->loginpage);
    ui->userspage->setFocus();
    userSelectStateMachine(0,currentUserIndex);
    ui->passwordInput->setFocus();
    capsLockCheck();

}

void LoginForm::on_userbutton3_clicked()
{
    userSelectStateMachine(0,currentUserIndex + 1);
}


void LoginForm::Debug(QString message){

    qDebug() << message;
}


void LoginForm::on_resetpasswordButton_clicked()
{
    if(ui->oldpasswordinput->text().isEmpty()){
        ui->rstpwdmessagelabel->setText(tr("Old password is wrong"));
        return;
    }

    if(ui->newpasswordinput->text().isEmpty() || ui->newpasswordconfirminput->text().isEmpty()){
        ui->rstpwdmessagelabel->setText(tr("New passwords are not same"));
        return;
    }

    if(ui->oldpasswordinput->text().trimmed().compare(oldPassword)){

        ui->rstpwdmessagelabel->setText(tr("Old password is wrong"));

    }else if(ui->newpasswordconfirminput->text().trimmed().compare(ui->newpasswordinput->text().trimmed())){

        ui->rstpwdmessagelabel->setText(tr("New passwords are not same"));

    }else{
        //everything is ok
        resetTimer->setTimerType(Qt::TimerType::CoarseTimer);

        resetTimer->setSingleShot(false);

        if(needPasswordChange == false){
            cancelLogin();
            needPasswordChange = true;
            resetTimerState = 0;
            resetTimer->setInterval(10);

        }else if(needReenterOldPassword){
            //needReenterOldPassword = 0;
            resetTimerState = 3;
            resetTimer->setInterval(10);
        }else{

            resetTimerState = 4;
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

        ui->waitlabel->setText(tr("Authenticating"));
        pageTransition(ui->waitpage);
        m_Greeter.cancelAuthentication();
        userCheckCounter = 0;
        loginTimerState = 1;
        lastPrompt.clear();
        break;

    case 1:
        //enter user
        m_Greeter.authenticate(ui->userInput->text());
        loginTimerState = 2;
        promptCheckCounter = 0;
        lastPrompt.clear();
        break;

    case 2:
        if(lastPrompt.compare("Password: ") == 0 || lastPrompt.compare("password: ") == 0){
            m_Greeter.respond(ui->passwordInput->text().trimmed());
            endFlag = 1;
        }else if(promptCheckCounter > 200){
            pageTransition(ui->loginpage);
            endFlag = 1;
            ui->passwordInput->setEnabled(true);
            ui->userInput->setEnabled(true);
            ui->passwordInput->setFocus();
            ui->passwordInput->clear();
            m_Greeter.cancelAuthentication();
            messageReceived = 0;
            loginStartFlag = false;
            ui->messagelabel->setText(tr("Error : User is unknown"));
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

    switch(resetTimerState){

    case 0:
        pageTransition(ui->waitpage);
        m_Greeter.cancelAuthentication();
        resetTimerState = 1;
        break;

    case 1:
        //enter user
        m_Greeter.authenticate(ui->userInput->text());
        resetTimerState = 2;
        break;

    case 2:
        //enter password
        m_Greeter.respond(oldPassword.trimmed());
        resetTimerState = 3;
        break;

    case 3:
        //old password
        pageTransition(ui->waitpage);
        if(lastPrompt.compare("(current) UNIX password: ") == 0)
            m_Greeter.respond(oldPassword.trimmed());
        resetTimerState = 4;
        break;

    case 4:
        //"Enter new password: "
        pageTransition(ui->waitpage);
        m_Greeter.respond(ui->newpasswordinput->text().trimmed());
        resetTimerState = 5;
        break;

    case 5:
        //"Enter it again: "
        m_Greeter.respond(ui->newpasswordinput->text().trimmed());
        resetTimerState = 0;
        endFlag = 1;
        break;
    }

    if(endFlag == 1){

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
        qDebug() << tr("Unable to open capslock status\n") ;
        return false;
    }


    if(fread(data, sizeof(data), 1, fp) < 0){
        qDebug() << "Unable to read capslock status" << QString::fromLocal8Bit(data);

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

    }else if(ui->stackedWidget->currentIndex() != ui->stackedWidget->indexOf(ui->waitpage) && networkstatus == false
             && Settings().waittimeout() > 0 && !timeoutFlag){
        ui->waitlabel->setText(tr("Waiting for Network and Services"));
        pageTransition(ui->waitpage);
        m_Greeter.cancelAuthentication();
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
        mv->start();
    }
    else if(Page == ui->userspage){

        ui->userspage->setFocus();
        ui->userspage->setFocusPolicy(Qt::FocusPolicy::WheelFocus);
        mv->stop();
    }
    else if(Page == ui->loginpage){

        mv->stop();
    }
    else if(Page == ui->warningpage){

        mv->stop();
    }
    else if(Page == ui->resetpage){

        mv->stop();
    }


}
