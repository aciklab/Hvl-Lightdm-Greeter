
#include "main.h"
#include <QDebug>
#include <QCompleter>
#include <QAbstractListModel>


#include <QModelIndex>
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
#include <QWebEngineView>
#include <QPushButton>
#include <QMovie>
#include <QShortcut>
#include <QToolButton>
#include <QPropertyAnimation>
#include <QProcess>
#include <qcoreevent.h>
#include <unistd.h>
#include <signal.h>
#include<sys/wait.h>


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
    sessionsModel(QLightDM::SessionsModel::LocalSessions,this),
    userModel(this)

{
    m_Greeter.setResettable(false);

    if (!m_Greeter.connectSync()) {
        qCritical() << "Failed to connect lightdm";
        close();
    }

    ui->setupUi(this);
    initialize();
    ctrlClicked = false;

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
#ifdef SCREENKEYBOARD
    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(focusChanged(QWidget*, QWidget*)));
#endif
    // ui->pushButton_sr->hide();

    mv = new QMovie(":/resources/load1.gif");
    mv->setScaledSize(QSize(ui->giflabel->width(), ui->giflabel->height()));






    connect(&m_Greeter, SIGNAL(showPrompt(QString, QLightDM::Greeter::PromptType)), this, SLOT(onPrompt(QString, QLightDM::Greeter::PromptType)));
    connect(&m_Greeter, SIGNAL(showMessage(QString, QLightDM::Greeter::MessageType)), this, SLOT(onMessage(QString, QLightDM::Greeter::MessageType)));
    connect(&m_Greeter, SIGNAL(authenticationComplete()), this, SLOT(authenticationComplete()));
    connect(&m_Greeter, SIGNAL(reset()), this, SLOT(resetRequest()));


    realM = readRealm();
    ui->passwordInput->clear();

    initializeUserList();

    checkPasswordResetButton();

    if (!m_Greeter.hideUsersHint()) {
        QLightDM::UsersModel usersModel;

        for (int i = 0; i < usersModel.rowCount(QModelIndex()); i++) {
            knownUsers << usersModel.data(usersModel.index(i, 0), QLightDM::UsersModel::NameRole).toString();
        }

        ui->userInput->setCompleter(new QCompleter(knownUsers));
        ui->userInput->completer()->setCompletionMode(QCompleter::InlineCompletion);
    }

    QString user = Cache().getLastUser();

    if (user.isEmpty() || user.isNull()) {
        user = m_Greeter.selectUserHint();
        currentSessionStr = m_Greeter.defaultSessionHint();
    }else{
        currentSessionStr = Cache().getLastSession(user);
    }







    // cancelLogin();

    messageReceived = false;
    needPasswordChange = false;
    needReenterOldPassword = false;
    loginStartFlag = false;
    resetStartFlag = false;
    loginTimeot = 0;
    networkOK = false;
    loginprompt = false;
    nwcheckcount = 0;
    userResetRequest = false;
    systemResetRequest = false;

    loginTimer = new QTimer();
    resetTimer = new QTimer();
    animationTimer = new QTimer();
    userRequestResetTimer = new QTimer();


    connect(loginTimer, SIGNAL(timeout()), this, SLOT(LoginTimerFinished()));
    connect(resetTimer, SIGNAL(timeout()), this, SLOT(passwordResetTimerFinished()));
    connect(animationTimer, SIGNAL(timeout()), this, SLOT(animationTimerFinished()));
    connect(userRequestResetTimer, SIGNAL(timeout()), this, SLOT(userPasswordResetRequest()));




    ui->giflabel->setAttribute(Qt::WA_NoSystemBackground);
    ui->giflabel->setMovie(mv);

    // ui->pwShowbutton->setParent(ui->passwordInput);
    //ui->pwShowbutton->raise();
    //ui->pwShowbutton->move( ui->passwordInput->x() + ui->passwordInput->width() - 10, 5);


    if(Settings().waittimeout() == 0){

        //ui->userInput->show();
        ui->userInput->setFocus();

        pageTransition(ui->loginpage);


    }
    else{
        if(!networkOK){
            pageTransition(ui->waitpage);
            qInfo() << "Waiting for network and services";
        }
    }

    QWidget::setTabOrder(ui->userInput, ui->passwordInput);
    QWidget::setTabOrder(ui->oldpasswordinput, ui->newpasswordinput);
    QWidget::setTabOrder(ui->newpasswordinput, ui->newpasswordconfirminput);

}

void LoginForm::cancelLogin()
{

    if (m_Greeter.inAuthentication()) {
        m_Greeter.cancelAuthentication();
        qWarning() << "Authentication is canceled";
        // ui->messagelabel->clear();
    }

}


void LoginForm::startLogin()
{

    if(toolButtons[(lastuserindex + 1) % 3]->text().isEmpty())
        return;


    if(toolButtons[(lastuserindex + 1) % 3]->text().compare(tr("Other User")) == 0 && ui->userInput->text().isEmpty())
        return;

    ui->messagelabel->clear();
    tmpPassword = ui->passwordInput->text();
    oldPassword = tmpPassword;
    ui->passwordInput->setEnabled(false);
    ui->userInput->setEnabled(false);

    loginTimer->setTimerType(Qt::TimerType::CoarseTimer);
    loginTimer->setSingleShot(true);
    loginTimer->setInterval(100);
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

    qInfo() << "Received Prompt: " << prompt << " type: " << QString::number(promptType);

    if((prompt.compare("Enter new password: ") == 0 || prompt.compare("New password: ") == 0 ||
        prompt.compare("(current) UNIX password: ") == 0 || prompt.compare("Current Password: ") == 0)
            && needPasswordChange != 1 && !resetStartFlag){

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
    else if((prompt.compare("Enter new password: ") == 0 || prompt.compare("New password: ") == 0 ) &&
            resetStartFlag && !needReenterOldPassword){

        needPasswordChange = true;
        pageTransition(ui->resetpage);
        userResetRequest = false;
        ui->oldpasswordinput->setEnabled(true);
        ui->newpasswordinput->setEnabled(true);
        ui->newpasswordconfirminput->setEnabled(true);
        ui->newpasswordinput->clear();
        ui->newpasswordconfirminput->clear();
        ui->newpasswordinput->setFocus();

        resetStartFlag = false;
        loginStartFlag = false;
    }else if((prompt.compare("login:") == 0 || prompt.compare("login: ") == 0 )){

        loginprompt = true;

    }
}

void LoginForm::onMessage(QString prompt, QLightDM::Greeter::MessageType messageType){
    QString type = NULL;

    qInfo() << "Received Message: " << prompt << " type: " << QString::number(messageType);

    if(messageType == QLightDM::Greeter::MessageTypeError){
        type = tr("Error");
    }
    else if(messageType == QLightDM::Greeter::MessageTypeInfo){
        type = tr("Info");
    }
    else{
        type = " ";
    }

    messageReceived = true;

    if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage)){

        ui->messagelabel->setText(type + " : " + prompt);
        ui->warninglabel->setText(type + " : " + prompt);
    }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->warningpage)){
        ui->warninglabel->setText(type + " : " + prompt);
    }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->resetpage) || resetStartFlag){
        ui->rstpwdmessagelabel->setText(type + " : " + prompt);
    }else{
        ui->messagelabel->setText(type + " : " + prompt);

    }

    lastMessage = prompt;

}

#if 0
QString LoginForm::currentSession()
{
    return m_Greeter.defaultSessionHint();

    // QModelIndex index = sessionsModel.index(0, 0, QModelIndex());
    //return sessionsModel.data(index, QLightDM::SessionsModel::KeyRole).toString();
}
#endif

void LoginForm::setCurrentSession(QString &session)
{

    currentSessionStr = session;

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

    QString lastuser;


    lastuser = ui->userInput->text().trimmed();

    if(lastuser.isNull() || lastuser.isEmpty())
        lastuser = toolButtons[(lastuserindex + 1) % 3]->text();

    if (m_Greeter.isAuthenticated()) {



        qInfo() << "Authentication is completed for  " + lastuser;


        /* Reset to default screensaver values */

        needPasswordChange = 0;

        addUsertoCache(lastuser);
        Cache().setLastUser(lastuser);
        Cache().setLastSession(lastuser, currentSessionStr);
        Cache().sync();

        qWarning() <<  "Start session : " << currentSessionStr;
        if(!m_Greeter.startSessionSync(currentSessionStr)){

            qWarning() <<  "Failed to start session  ";

            if(!needPasswordChange)
                cancelLogin();

            if(!messageReceived)
                ui->messagelabel->setText(tr("Error : Login Incorrect"));
            else if(!lastMessage.isNull() && !lastMessage.isEmpty())
                ui->messagelabel->setText(lastMessage);
            else
                ui->messagelabel->setText(tr("Error : Login Incorrect"));
            preparetoLogin();

        }else{

        }
        needPasswordChange = false;

    }else if(loginStartFlag == true || resetStartFlag == true) {

        qWarning() <<  "Authentication error for  " + lastuser;

        qWarning() <<  "message: " + lastMessage;

        if(!needPasswordChange)
            cancelLogin();

        if(!messageReceived)
            ui->messagelabel->setText(tr("Error : Login Incorrect"));
        else if(!lastMessage.isNull() && !lastMessage.isEmpty())
            ui->messagelabel->setText(lastMessage);
        else
            ui->messagelabel->setText(tr("Error : Login Incorrect"));

        if(resetStartFlag){

            pageTransition(ui->warningpage);
            ui->warninglabel->setText(translateResetPwdMessage(lastMessage));
            resetStartFlag = true;
            userResetRequest = false;
            needPasswordChange = false;
            needReenterOldPassword = true;
        }else{
            preparetoLogin();
        }


    }

    messageReceived = false;
    loginStartFlag = false;
    resetStartFlag = false;
}


void LoginForm::preparetoLogin(){

    if(loginStartFlag)
        pageTransition(ui->loginpage);

    if(resetStartFlag){
        userResetRequest = false;
        pageTransition(ui->resetpage);
    }
    ui->passwordInput->clear();

    if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage)){
        ui->passwordInput->setEnabled(true);
        ui->userInput->setEnabled(true);
        // ui->passwordInput->setFocus();
        ui->passwordInput->clear();
    }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->resetpage)){
        ui->newpasswordinput->clear();
        ui->oldpasswordinput->clear();
        ui->newpasswordconfirminput->clear();
        ui->newpasswordinput->setEnabled(true);
        ui->newpasswordconfirminput->setEnabled(true);
        ui->oldpasswordinput->setEnabled(true);
        ui->resetpasswordButton->setEnabled(true);
        //ui->newpasswordinput->setFocus();
        needPasswordChange = false;
    }


}


void LoginForm::addUsertoCache(QString user_name){

    int i;
    int nullfound = 0;

    for(i = 0; i< 6; i++){
        if(userList[i].compare(user_name) == 0)
            userList[i].clear();

        if(i >= Settings().cachedusercount() || userList[i].compare(tr("Other User")) == 0)
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

    for(i = 0; i < Settings().cachedusercount(); i++){
        if(!userList[i].isNull() && !userList[i].isEmpty())
            Cache().setLastUsertoIndex(userList[i], i);
        else
            Cache().setLastUsertoIndex("", i);

    }

    userList[total_user_count - 1] = tr("Other User");

}


void LoginForm::on_pushButton_resetpwd_clicked()
{

    QString userid;



    if(toolButtons[(lastuserindex + 1) % 3]->text().isEmpty())
        return;


    if(toolButtons[(lastuserindex + 1) % 3]->text().compare(tr("Other User")) == 0 && ui->userInput->text().isEmpty())
        return;




    QString str;

    str = Settings().passwordresetenabled();



    if(!Settings().passwordresetwebpageurl().isNull() && !Settings().passwordresetwebpageurl().isEmpty() && (str.compare("y") == 0 || str.compare("Y") == 0)){
        Dialog_webview dwview;
        dwview.setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
        qInfo() << "Password reset webpage is opening";
        dwview.exec();
    }else if(str.compare("y") == 0 || str.compare("Y") == 0){

        pageTransition(ui->resetpage);
        userResetRequest = true;
        ui->rstpwdmessagelabel->clear();


        ui->oldpasswordinput->clear();
        ui->newpasswordinput->clear();
        ui->newpasswordconfirminput->clear();

        ui->newpasswordinput->setEnabled(true);
        ui->newpasswordconfirminput->setEnabled(true);
        ui->oldpasswordinput->setEnabled(true);

        if(!ui->passwordInput->text().isEmpty() && !ui->passwordInput->text().isNull()){
            ui->oldpasswordinput->setText(ui->passwordInput->text());
            ui->newpasswordinput->setFocus();
        }else{
            ui->oldpasswordinput->setFocus();
        }



        if(toolButtons[(lastuserindex+ 1) % 3]->text().compare(tr("Other User")) == 0){
            userid = ui->userInput->text();
        } else{
            userid = toolButtons[(lastuserindex+ 1) % 3]->text();
        }

        ui->usernameLabel->setText(tr("Password Change Operation for: ") + userid);

    }


}


void LoginForm::checkPasswordResetButton(){

    QString str;

    str = Settings().passwordresetenabled();

    if(str.compare("y")!= 0 && str.compare("Y") != 0)
    {
        ui->pushButton_resetpwd->setVisible(false);
        ui->pushButton_resetpwd->setEnabled(false);
    }

}


void LoginForm::initializeUserList(){

    total_user_count = 0;
    animationprogress = false;



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

    userList[total_user_count]  = tr("Other User");



    qInfo() <<  (QString::number(total_user_count) + " users found for last users cache");


    currentSessionStr = Cache().getLastSession( userList[0]);
    qInfo() << "currentSessionStr: " << currentSessionStr;

    total_user_count++;
    usersbuttonReposition();




    if(total_user_count == 1){

        ui->toolButtonleft->hide();
        ui->toolButtoncenter->setText(tr("Other User"));
        ui->toolButtonright->hide();
        ui->userInput->setText("");
        ui->userInput->show();
        useroffset = 0;
        olduseroffset = 0;
        return;
    }

    QString username;
    QString imagepath;
    QModelIndex modelIndex;

    for(int i = 0; i< total_user_count; i++){

        for(int j = 0; j < userModel.rowCount(QModelIndex()); j++){

            modelIndex = userModel.index(j, 0);
            username = userModel.data(modelIndex, QLightDM::UsersModel::NameRole).toString();

            if(i != total_user_count){

                if(username.compare(userList[i]) == 0){

                    imagepath = userModel.data(modelIndex, QLightDM::UsersModel::ImagePathRole).toString();
                    break;
                }
            }
        }
    }

    ui->userInput->hide();


    ui->toolButtonleft->hide();
    ui->toolButtoncenter->setText(userList[0]);
    useroffset = 0;
    olduseroffset = 0;

    if(total_user_count > 1){
        ui->toolButtonright->setText(userList[1]);
    }else
        ui->toolButtonright->setText(tr("Other User"));

    connect(ui->toolButtonleft, SIGNAL(clicked()), this, SLOT(userButtonClicked()));
    connect(ui->toolButtoncenter, SIGNAL(clicked()), this, SLOT(userButtonClicked()));
    connect(ui->toolButtonright, SIGNAL(clicked()), this, SLOT(userButtonClicked()));

    current_user_button = 1;
    currentUserIndex = 0;

}

void LoginForm::userButtonClicked(){

    QObject *senderObj = sender(); // This will give Sender object
    QString senderObjName = senderObj->objectName();

    for(int i = 0; i< total_user_count + 1; i++){

        if(senderObjName.compare(toolButtons[(lastuserindex + 0) % 3]->objectName()) == 0){//left

            userSelectStateMachine(Qt::Key_Left, -1);

        }else if(senderObjName.compare(toolButtons[(lastuserindex + 1) % 3]->objectName()) == 0){//center

            if(toolButtons[(lastuserindex + 1) % 3]->text().compare(tr("Other User")) == 0){
                ui->userInput->show();
                ui->userInput->setText("");

            }else{
                ui->userInput->hide();
                ui->userInput->setText(toolButtons[(lastuserindex + 1) % 3]->text());
            }



        }else{
            userSelectStateMachine(Qt::Key_Right, -1);
        }//toolButtonright

    }

}

void LoginForm::animationTimerFinished(){



    QFont font;

    switch(animationTimerState){
    case 0:

        animGroup = new QParallelAnimationGroup;

        animationTimerState++;
        animationTimer->stop();
        animationTimer->setInterval(ANIMATION_TIME + 30);
        animationTimer->start();

        if(lastuserindex > 0){
            toolButtons[lastuserindex % 3]->setText(userList[lastuserindex - 1]);
        }

        toolButtons[(lastuserindex + 1) % 3]->setText(userList[lastuserindex]);

        if(lastuserindex > 1 && (lastuserindex + 1) < total_user_count){
            toolButtons[(lastuserindex + 2) % 3]->setText(userList[lastuserindex + 1]);
        }

        if(lastkey == Qt::Key_Right){

            anim1[0] = new QPropertyAnimation(toolButtons[(lastuserindex) % 3], "geometry");

            anim1[0]->setDuration(ANIMATION_TIME);

            anim1[0]->setStartValue(center);

            anim1[0]->setEndValue(left);


            toolButtons[(lastuserindex) % 3]->setIcon(iconPassive);

            font = toolButtons[(lastuserindex) % 3]->font();
            font.setPointSize(8);
            toolButtons[(lastuserindex) % 3]->setFont(font);




            anim1[1] = new QPropertyAnimation(toolButtons[(lastuserindex + 1)  % 3], "geometry");


            anim1[1]->setDuration(ANIMATION_TIME);


            anim1[1]->setStartValue(right);

            anim1[1]->setEndValue(center);

            animGroup->addAnimation(anim1[0]);
            animGroup->addAnimation(anim1[1]);



            toolButtons[(lastuserindex - 1) % 3]->hide();
            toolButtons[(lastuserindex - 1) % 3]->setGeometry(right);

        }else{
            anim1[0] = new QPropertyAnimation(toolButtons[(lastuserindex + 1) % 3], "geometry");


            anim1[0]->setDuration(ANIMATION_TIME);

            anim1[0]->setStartValue(left);

            anim1[0]->setEndValue(center);


            anim1[1] = new QPropertyAnimation(toolButtons[(lastuserindex + 2)  % 3], "geometry");

            anim1[1]->setDuration(ANIMATION_TIME);

            anim1[1]->setStartValue(center);

            anim1[1]->setEndValue(right);

            if(!toolButtons[(lastuserindex + 2) % 3]->text().compare(tr("Other User")))
                toolButtons[(lastuserindex + 2) % 3]->setIcon(iconEmptyPassive);
            else
                toolButtons[(lastuserindex + 2) % 3]->setIcon(iconPassive);

            font = toolButtons[(lastuserindex + 2) % 3]->font();
            font.setPointSize(8);
            toolButtons[(lastuserindex + 2) % 3]->setFont(font);


            animGroup->addAnimation(anim1[0]);
            animGroup->addAnimation(anim1[1]);

            toolButtons[(lastuserindex) % 3]->hide();
            toolButtons[(lastuserindex) % 3]->setGeometry(left);

            if(!toolButtons[(lastuserindex) % 3]->text().compare(tr("Other User")))
                toolButtons[(lastuserindex) % 3]->setIcon(iconEmptyPassive);
            else
                toolButtons[(lastuserindex) % 3]->setIcon(iconPassive);

        }
        animGroup->start();
        break;

    case 1:
        if(lastkey == Qt::Key_Right){




            font = toolButtons[(lastuserindex + 1) % 3]->font();
            font.setPointSize(11);
            toolButtons[(lastuserindex + 1) % 3]->setFont(font);

            if(!toolButtons[(lastuserindex + 1) % 3]->text().compare(tr("Other User")))
                toolButtons[(lastuserindex + 1) % 3]->setIcon(iconEmptyActive);
            else
                toolButtons[(lastuserindex + 1) % 3]->setIcon(iconActive);


            if((lastuserindex + 1) < total_user_count){
                toolButtons[(lastuserindex - 1) % 3]->show();

                toolButtons[(lastuserindex - 1) % 3]->setText(userList[lastuserindex + 1]);//last added


                if(!toolButtons[(lastuserindex - 1) % 3]->text().compare(tr("Other User")))
                    toolButtons[(lastuserindex - 1) % 3]->setIcon(iconEmptyPassive);
                else
                    toolButtons[(lastuserindex - 1) % 3]->setIcon(iconPassive);

            }

            if(lastuserindex == total_user_count - 1)
                toolButtons[(lastuserindex + 2) % 3]->hide();

        }else{


            font = toolButtons[(lastuserindex + 1) % 3]->font();
            font.setPointSize(11);
            toolButtons[(lastuserindex + 1) % 3]->setFont(font);


            if(!toolButtons[(lastuserindex + 1) % 3]->text().compare(tr("Other User")))
                toolButtons[(lastuserindex + 1) % 3]->setIcon(iconEmptyActive);
            else
                toolButtons[(lastuserindex + 1) % 3]->setIcon(iconActive);



            if(lastuserindex >= 1)
                toolButtons[(lastuserindex)% 3]->show();


        }


        if(lastuserindex > 0){
            toolButtons[lastuserindex % 3]->setText(userList[lastuserindex - 1]);
        }

        toolButtons[(lastuserindex + 1) % 3]->setText(userList[lastuserindex]);


        if(lastuserindex > 1 && (lastuserindex + 1) < total_user_count){
            toolButtons[(lastuserindex + 2) % 3]->setText(userList[lastuserindex + 1]);

        }





        animationTimer->stop();
        animGroup->stop();

        animGroup->clear();



        capsLockCheck();
        animationprogress = false;
        animationTimerState = 0;



        if( toolButtons[(lastuserindex + 1) % 3]->text().compare(tr("Other User")) == 0){

            ui->userInput->show();
            ui->userInput->setText("");
            ui->domainnameLabel->setText("");
            emit sendCurrentUser(QString("Other User"));


        }else{
            ui->userInput->hide();
            ui->domainnameLabel->setText(getUserRealm(toolButtons[(lastuserindex + 1) % 3]->text()));
            emit sendCurrentUser(toolButtons[(lastuserindex + 1) % 3]->text());
        }


        break;
    }

}

void LoginForm::userSelectStateMachine(int key, int button){


    if(useroffset < 0 || animationprogress)
        return;


    if(key == Qt::Key_Right){
        lastuserindex++;
    }else if(key == Qt::Key_Left){

        if(lastuserindex <= 0)
            return;

        lastuserindex--;
    }
    if(lastuserindex < 0)
        lastuserindex = 0;

    if(lastuserindex >= total_user_count){
        lastuserindex = total_user_count - 1;
        return;
    }

    animationprogress = true;

    animationTimer->setTimerType(Qt::TimerType::CoarseTimer);
    animationTimer->setSingleShot(false);
    animationTimer->setInterval(10);
    animationTimer->start();
    animationTimerState = 0;
    lastkey = key;

}


void LoginForm::keyPressEvent(QKeyEvent *event)
{

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {

        ctrlClicked = false;
        if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->resetpage))
            on_resetpasswordButton_clicked();
        else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->warningpage)){
            on_acceptbutton_clicked();
        }
        else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage)){

            if (ui->userInput->text().isEmpty() && (toolButtons[(lastuserindex + 1) % 3]->text().isEmpty() || toolButtons[(lastuserindex + 1) % 3]->text().compare(tr("Other User")) == 0)){
                ui->userInput->setFocus();
            }else if(!ui->userInput->hasFocus() && !ui->userInput->isHidden() && ui->userInput->text().isEmpty()){
                ui->userInput->setFocus();
            }else if(ui->userInput->isHidden() && !ui->passwordInput->hasFocus() && ui->passwordInput->text().isEmpty()){
                ui->passwordInput->setFocus();
            } else if(ui->userInput->hasFocus() && !ui->userInput->isHidden() && ui->passwordInput->text().isEmpty()){
                ui->passwordInput->setFocus();
            }else{
                startLogin();
            }

        }
    }
    else if (event->key() == Qt::Key_Escape) {

        ctrlClicked = false;
#ifdef SCREENKEYBOARD
        emit sendKeyboardCloseRequest();
#endif

        if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage)){

            on_backButton_clicked();
            //needPasswordChange = 0;
            //cancelLogin();
            //usersbuttonReposition();

        }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->resetpage) && !resetStartFlag){
            // on_cancelResetButton_clicked();

            keyboardCloseEvent();
        }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->waitpage) &&
                 loginStartFlag && !resetStartFlag){
            // m_Greeter.cancelAuthentication();
            //pageTransition(ui->loginpage);

        }
    }
    else if ((event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) &&
             ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage)) {
        ctrlClicked = false;
        userSelectStateMachine(event->key(), -1);
    }else if (event->key() == Qt::Key_Control){
        ctrlClicked = true;
    }else if (event->key() == Qt::Key_Shift){

        if(ctrlClicked)
            emit selectKeyboard();

        ctrlClicked = false;
    }
    else {
        ctrlClicked = false;
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

    QString userid;

    if(!userResetRequest){


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
            ui->newpasswordinput->setEnabled(false);
            ui->newpasswordconfirminput->setEnabled(false);
            ui->oldpasswordinput->setEnabled(false);


        }

    }else{

        //user requested reset her/his password

        ui->rstpwdmessagelabel->clear();

        if(ui->newpasswordinput->text().isEmpty() || ui->newpasswordconfirminput->text().isEmpty()){
            ui->rstpwdmessagelabel->setText(tr("New passwords are not same"));
            return;
        }

        if(ui->newpasswordconfirminput->text().compare(ui->newpasswordinput->text())){

            ui->rstpwdmessagelabel->setText(tr("New passwords are not same"));

        }else{

            /* everything is ok */
            cancelLogin();

            if(toolButtons[(lastuserindex+ 1) % 3]->text().compare(tr("Other User")) == 0){
                userid = ui->userInput->text();
            } else{
                userid = toolButtons[(lastuserindex+ 1) % 3]->text();
            }

            userRequestResetTimer->setTimerType(Qt::TimerType::CoarseTimer);

            userRequestResetTimer->setSingleShot(false);

            userRequestTimerState = 0;

            userRequestResetTimer->setInterval(10);

            qInfo() << "Reset password operation started for " + userid;
            userRequestResetTimer->start();
            ui->resetpasswordButton->setEnabled(false);
            ui->cancelResetButton->setEnabled(false);

            ui->waitlabel->setText(tr("Password is Updating"));
            pageTransition(ui->waitpage);
            passwordChangeError = false;

        }


    }
}

void LoginForm::LoginTimerFinished(){

    bool endFlag = 0;
    static int promptCheckCounter;
    static int userCheckCounter;
    QString userid;
    static int opcheckcounter;


    switch(loginTimerState){

    case 0:

        qInfo() << "Login start for " + toolButtons[(lastuserindex + 1) % 3]->text().trimmed();

        ui->waitlabel->setText(tr("Authenticating"));
        pageTransition(ui->waitpage);
        //cancelLogin();
        userCheckCounter = 0;
        loginTimerState = 1;
        lastPrompt.clear();


        loginTimer->setInterval(100);

        break;

    case 1:
        //enter user

        if(!m_Greeter.isAuthenticated()){
            //cancelLogin();
        }


        if(toolButtons[(lastuserindex+ 1) % 3]->text().compare(tr("Other User")) == 0){
            userid = ui->userInput->text();
        } else{
            userid = toolButtons[(lastuserindex+ 1) % 3]->text();
        }

        qInfo() << "User name is sending";
        m_Greeter.authenticate(userid.trimmed());
        loginTimerState = 2;
        messageReceived = false;
        promptCheckCounter = 0;
        lastPrompt.clear();
        loginTimer->setInterval(100);
        break;

    case 2:

        if(loginprompt){
            loginTimer->setInterval(100);
            loginprompt = false;
        }else{

            if(m_Greeter.isAuthenticated()){

                qWarning() << "user already Authenticated";
                m_Greeter.startSessionSync(currentSessionStr);
                endFlag = 1;
                promptCheckCounter = 0;

            }else if((lastPrompt.compare("Password: ") == 0 || lastPrompt.compare("password: ") == 0) && m_Greeter.inAuthentication()){

                qInfo() << "password is sending";
                m_Greeter.respond(ui->passwordInput->text());
                endFlag = 1;
                promptCheckCounter = 0;
            }else if(promptCheckCounter > 40){
                pageTransition(ui->loginpage);
                endFlag = 1;
                ui->passwordInput->setEnabled(true);
                ui->userInput->setEnabled(true);
                // ui->passwordInput->setFocus();
                ui->passwordInput->clear();
                cancelLogin();
                messageReceived = 0;
                loginStartFlag = false;

                if(networkOK){
                    ui->messagelabel->setText(tr("Error : User is unknown"));
                }else{
                    ui->messagelabel->setText(tr("Error : Network is not connected"));
                }

                loginTimer->setInterval(100);

                qWarning() << "No password prompt received";
                qWarning() << "Authentication is canceled";

            }else{

                if(!messageReceived){

                    promptCheckCounter++;
                    loginTimer->setInterval(100);
                }else{

                    loginStartFlag = false;
                    endFlag = 1;
                }
            }

        }
        break;

    case 3:
        break;
    }

    if(endFlag == 1){
        loginTimerState = 0;
        loginTimer->stop();
        messageReceived = 0;
    }else{
        loginTimer->stop();

        loginTimer->start();
        loginStartFlag = true;
        promptFlag = false;
    }
}


void LoginForm::userPasswordResetRequest(){

    bool endFlag = 0;

    QString userid;


    char data[512] = {};
    QString result;
    QString received;


    char *user_data;
    char *pass_data;


    static pid_t pid = 0;
    static int inpipefd[2];
    static int outpipefd[2];
    int status;


    switch(userRequestTimerState){

    case 0:

        if(toolButtons[(lastuserindex+ 1) % 3]->text().compare(tr("Other User")) == 0){
            userid = ui->userInput->text();
        } else{
            userid = toolButtons[(lastuserindex+ 1) % 3]->text();
        }


        //send user id
        user_data = userid.toLocal8Bit().data();


        pipe(inpipefd);
        pipe(outpipefd);
        pid = fork();
        if (pid == 0)
        {
            // Child
            dup2(outpipefd[0], STDIN_FILENO);
            dup2(inpipefd[1], STDOUT_FILENO);
            dup2(inpipefd[1], STDERR_FILENO);

            //ask kernel to deliver SIGTERM in case the parent dies
            //prctl(PR_SET_PDEATHSIG, SIGTERM);

            //replace tee with your process
            //execl("/usr/bin/passwd", "passwd", user_data,  (char*) NULL);

            execl("/sbin/runuser", "runuser","-l", user_data, "-c", "passwd", (char*) NULL);
            exit(1);
        }

        ::close(outpipefd[0]);
        ::close(inpipefd[1]);

        userRequestTimerState++;
        break;

    case 1:
        //fwrite()

        read(inpipefd[0], data, 512);

        qWarning() << QString::fromUtf8(data);

        received = QString::fromUtf8(data);

        if(!strcmp(data, "Current Password: ") || !strcmp(data, "(current) UNIX password: ") || !strcmp(data, "(geçerli) parola: ") || !strcmp(data, "Current Kerberos password:")
                || !strcmp(data, "Current Kerberos password: ") || !strcmp(data, "Enter login(LDAP) password: ")){

            memset(data, 0, 512);

            pass_data = ui->oldpasswordinput->text().toLocal8Bit().data();
            write(outpipefd[1], pass_data, strlen(pass_data));
            write(outpipefd[1], "\n", 1);

        }else{
            memset(data, 0, 512);
        }

        userRequestTimerState++;
        break;

    case 2:

        pass_data = ui->newpasswordinput->text().toLocal8Bit().data();
        write(outpipefd[1], pass_data, strlen(pass_data));
        write(outpipefd[1], "\n", 1);

        userRequestTimerState++;
        break;

    case 3:
        pass_data = ui->newpasswordinput->text().toLocal8Bit().data();
        write(outpipefd[1], pass_data, strlen(pass_data));
        write(outpipefd[1], "\n", 1);

        userRequestTimerState++;
        break;

    case 4:

        memset(data, 0, 512);
        read(inpipefd[0], data, 512);
        received = QString::fromUtf8(data);

        qWarning() << QString::fromUtf8(data);



        kill(pid, SIGKILL); //send SIGKILL signal to the child process
        waitpid(pid, &status, 0);

        userRequestTimerState = 0;
        endFlag = true;
        passwordChangeError = true;

        if(strstr(data, "System is offline, password change not possible")){

            result = tr("System is offline, password change not possible");

        }else if(strstr(data, "Password change rejected") || strstr(data, "Please make sure the password meets the complexity constraints")){

            result = tr("Password change rejected: Please make sure the password meets the complexity constraints and don't use old passwords");

        }else if(strstr(data, "You must choose a longer password") || strstr(data,"Daha uzun bir parola girmelisiniz") ||  strstr(data, "The password is shorter than 8 characters")
                 || strstr(data, "Password too short")){

            result = tr("You must choose a longer password");

        }else if(strstr(data, "password is too similar") || strstr(data, "Passwords must differ")){

            result = tr("password is too similar");

        }else if(strstr(data, "Old password not accepted") || strstr(data, "LDAP Password incorrect")){

            result = tr("Old password not accepted");

        }else if(strstr(data, "The password is just rotated old one")){

            result = tr("password is too similar");

        }else if(strstr(data, "Authentication token manipulation error") || strstr(data, "Yetkilendirme anahtarı manipülasyon hatası")){

            result = tr("Authentication token manipulation error");

        }else if(strstr(data, "BAD PASSWORD: The password fails the dictionary check - it is based on a dictionary word")){
            result = tr("Password change rejected: it is based on a dictionary word");

        }
        else if(strstr(data, "password changed for") || strstr(data, "şifre başarıyla güncellendi") ||  strstr(data, "şifre başarıyla değiştirildi") || strstr(data, "password updated successfully")
                || strstr(data, "LDAP password information changed") ){

            result = tr("Password change successfull");
            passwordChangeError = false;
        }else{

            result = tr("Password change error");
        }



        // ui->oldpasswordinput->clear();
        ui->newpasswordinput->clear();
        ui->newpasswordconfirminput->clear();
        ui->oldpasswordinput->clearFocus();
        ui->newpasswordinput->clearFocus();
        ui->newpasswordconfirminput->clearFocus();
#ifdef SCREENKEYBOARD
        emit sendKeyboardCloseRequest();
#endif

        //ui->rstpwdmessagelabel->setText(result);
        ui->warninglabel->setText(result);

        break;


    }

    if(endFlag == 1){
        userRequestTimerState = 0;
        userRequestResetTimer->stop();
        ui->resetpasswordButton->setEnabled(true);
        ui->cancelResetButton->setEnabled(true);

        //ui->waitlabel->setText(tr("Password is Updating"));
        pageTransition(ui->warningpage);

    }else{
        userRequestResetTimer->stop();
        userRequestResetTimer->setInterval(500);
        userRequestResetTimer->start();
    }



}


void LoginForm::passwordResetTimerFinished(){

    bool endFlag = 0;
    static int opcheckcounter;
    bool errorflag = false;
    static QString userx;

    switch(resetTimerState){

    case 0:
        ui->resetpasswordButton->setEnabled(false);
        pageTransition(ui->waitpage);
        cancelLogin();
        resetTimerState = 1;
        break;

    case 1:
        //enter user


        if(toolButtons[(lastuserindex+ 1) % 3]->text().compare(tr("Other User")) == 0){
            userx = ui->userInput->text().trimmed();
        } else{
            userx = toolButtons[(lastuserindex+ 1) % 3]->text();
        }

        m_Greeter.authenticate(userx);
        resetTimerState = 2;
        break;

    case 2:
        //enter password
        m_Greeter.respond(oldPassword);
        resetTimerState = 3;
        break;

    case 3:
        //old password
        ui->resetpasswordButton->setEnabled(false);
        pageTransition(ui->waitpage);
        if(lastPrompt.compare("(current) UNIX password: ") == 0 || lastPrompt.compare("Current Password: ") == 0|| lastPrompt.compare("Password: ") == 0)
            m_Greeter.respond(oldPassword);

        lastPrompt.clear();
        resetTimerState = 35;
        break;


    case 35:
        //old password
        ui->resetpasswordButton->setEnabled(false);
        pageTransition(ui->waitpage);
        if(lastPrompt.compare("(current) UNIX password: ") == 0 || lastPrompt.compare("Current Password: ") == 0|| lastPrompt.compare("Password: ") == 0)
            m_Greeter.respond(oldPassword);

        resetTimerState = 4;
        break;

    case 4:
        //"Enter new password: "
        ui->resetpasswordButton->setEnabled(false);
        pageTransition(ui->waitpage);
        m_Greeter.respond(ui->newpasswordinput->text());
        resetTimerState = 5;
        break;


    case 5:
        if(lastPrompt.compare("Enter new password: ") == 0 || lastPrompt.compare("New password: ") == 0){
            cancelLogin();
            resetStartFlag = false;
            loginStartFlag = false;
            errorflag = true;
            endFlag = true;


        }else{
            resetTimerState = 6;
        }

        break;

    case 6:
        //"Enter it again: "
        m_Greeter.respond(ui->newpasswordinput->text());
        //resetTimerState = 0;
        resetTimerState = 7;
        opcheckcounter = 0;

        // endFlag = 1;
        break;

    case 7:
        if(opcheckcounter < 2){

            opcheckcounter++;
            resetTimerState = 7;

        }else{


            resetTimerState = 8;

            //pageTransition(ui->loginpage);
        }
        break;

    case 8:
        if(lastPrompt.compare("Enter new password: ") == 0 || lastPrompt.compare("New password: ") == 0){
            cancelLogin();
            resetStartFlag = false;
            loginStartFlag = false;
            errorflag = true;

        }
        endFlag = true;

        break;

    }

    if(endFlag == true){

        if(errorflag){//login timeout
            cancelLogin();
            //pageTransition(ui->resetpage);

            pageTransition(ui->warningpage);
            ui->warninglabel->setText(translateResetPwdMessage(lastMessage));
            resetStartFlag = true;
            userResetRequest = false;
            needPasswordChange = false;
            needReenterOldPassword = true;
            opcheckcounter = 0;
        }
        ui->resetpasswordButton->setEnabled(true);
        resetTimerState = 0;
        resetTimer->stop();
    }else{
        resetTimer->stop();
        resetTimer->setInterval(500);
        resetTimer->start();
        resetStartFlag = true;
    }

}

QString LoginForm::translateResetPwdMessage(QString message){


    QString res;

    if(message.contains("The password fails the dictionary check - it is based on a dictionary word")){

        res = tr("Error: The password fails the dictionary check - it is based on a dictionary word");

    }else if(message.contains("The password is shorter than 8 characters")){

        res = tr("Error: The password is shorter than 8 characters");

    }else if(message.contains("Please make sure the password meets the complexity constraints")){

        res = tr("Error: Please make sure the password meets the complexity constraints and don't use old passwords");

    }else if(message.contains("The password is the same as the old one")){

        res = tr("Error: The password is the same as the old one");

    }else if(message.contains("The password fails the dictionary check - it is too simplistic/systematic")){

        res = tr("Error: The password fails the dictionary check - it is too simplistic/systematic");

    }else{
        res = message;
    }

    return res;
}


void LoginForm::on_acceptbutton_clicked()
{
    if(userResetRequest){

        if(passwordChangeError){
            pageTransition(ui->resetpage);
            userResetRequest = true;
            systemResetRequest = false;

        } else{
            pageTransition(ui->loginpage);
            userResetRequest = false;
            systemResetRequest = false;
        }

        passwordChangeError = false;

    }else{
        pageTransition(ui->resetpage);
        systemResetRequest = true;
        userResetRequest = false;

        ui->oldpasswordinput->setEnabled(true);
        ui->newpasswordinput->setEnabled(true);
        ui->newpasswordconfirminput->setEnabled(true);

        // ui->oldpasswordinput->clear();
        ui->newpasswordinput->clear();
        ui->newpasswordconfirminput->clear();
        ui->newpasswordinput->setFocus();


    }


    // ui->newpasswordinput->setFocus();

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


    int read_size = fread(data, 1, sizeof(data), fp);

    if(read_size < 1){
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


    int tm = Settings().network_ok_timeout();
    if(tm == 0)
        tm = 0;
    else
        tm = tm /5;

    networkOK = networkstatus;

    if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->waitpage) && !loginStartFlag && !resetStartFlag && !userResetRequest && !systemResetRequest){

        if(networkstatus == false && loginTimeot < Settings().waittimeout()){
            loginTimeot += 5;
        }else{

            nwcheckcount++;

            if(nwcheckcount > tm){

                nwcheckcount = 0;
                if(networkstatus == false)
                    timeoutFlag = true;
                else
                    timeoutFlag = false;


                pageTransition(ui->loginpage);
                cancelLogin();
                loginTimeot = 0;
            }
        }

    }else if(ui->stackedWidget->currentIndex() != ui->stackedWidget->indexOf(ui->waitpage) &&
             (networkstatus == false) && Settings().waittimeout() > 0 && !timeoutFlag &&
             !loginStartFlag && !resetStartFlag){

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
        ui->userInput->clearFocus();
        ui->passwordInput->clearFocus();
#ifdef SCREENKEYBOARD
        emit sendKeyboardCloseRequest();
#endif
    }
    else if(Page == ui->loginpage){

        ui->passwordInput->setEnabled(true);
        //ui->userInput->setEnabled(true);

        ui->passwordInput->clear();
        toolButtons[(lastuserindex + 1) % 3]->setFocus();


        if(!loginStartFlag)
            ui->messagelabel->clear();

        mv->stop();
    }
    else if(Page == ui->warningpage){

#ifdef SCREENKEYBOARD
        emit sendKeyboardCloseRequest();
#endif
        ui->userInput->clearFocus();
        ui->passwordInput->clearFocus();
        mv->stop();

#ifdef SCREENKEYBOARD
        emit sendKeyboardCloseRequest();
#endif
    }
    else if(Page == ui->resetpage){
        ui->userInput->clearFocus();
        ui->passwordInput->clearFocus();

        mv->stop();
    }


}


void LoginForm::on_backButton_clicked()
{
    needPasswordChange = 0;
    cancelLogin();
    ui->passwordInput->clear();
    ui->userInput->clear();
    if(total_user_count == 1)
        return;

    ui->messagelabel->clear();
    usersbuttonReposition();
    emit sendKeyboardCloseRequest();
}

void LoginForm::usersbuttonReposition(){
    QFont font;

    ui->userInput->clear();
    ui->userInput->hide();
    ui->toolButtonleft->setGeometry(left);
    ui->toolButtoncenter->setGeometry(center);
    ui->toolButtonright->setGeometry(right);


    font = ui->toolButtonleft->font();
    font.setPointSize(8);
    ui->toolButtonleft->setFont(font);

    font = ui->toolButtonright->font();
    font.setPointSize(8);
    ui->toolButtonright->setFont(font);

    font = ui->toolButtoncenter->font();
    font.setPointSize(11);
    ui->toolButtoncenter->setFont(font);


    toolButtons[0] = ui->toolButtonleft;
    toolButtons[1] = ui->toolButtoncenter;
    toolButtons[2] = ui->toolButtonright;
    lastuserindex = 0;

    ui->toolButtonleft->setIcon(iconPassive);
    ui->toolButtonright->setIcon(iconPassive);
    ui->toolButtoncenter->setIcon(iconActive);


    ui->domainnameLabel->setText(getUserRealm(userList[0]));
    emit sendCurrentUser(userList[0]);

    ui->toolButtonleft->hide();
    ui->toolButtoncenter->setText(userList[0]);
    ui->toolButtoncenter->show();

    if(total_user_count > 1){
        ui->toolButtonright->setText(userList[1]);
        ui->toolButtonright->show();

        if(!ui->toolButtonright->text().compare(tr("Other User"))){
            ui->toolButtonright->setIcon(iconEmptyPassive);
        }

    }else{
        ui->toolButtonright->hide();
    }

    ui->toolButtonleft->hide();
    toolButtons[1]->setFocus();
}


void LoginForm::on_cancelResetButton_clicked()
{

    userResetRequest = false;
    systemResetRequest = false;

    needPasswordChange = false;
    needReenterOldPassword = false;
    loginStartFlag = false;
    resetStartFlag = false;

    needPasswordChange = 0;
    cancelLogin();
    pageTransition(ui->loginpage);
    ui->newpasswordconfirminput->clear();
    ui->newpasswordinput->clear();
    ui->oldpasswordinput->clear();
    ui->userInput->setEnabled(true);
    ui->passwordInput->setEnabled(true);
    ui->passwordInput->clear();

#if 0
    if(ui->userInput->isHidden())
        ui->passwordInput->setFocus();
    else if(!ui->userInput->text().isEmpty())
        ui->passwordInput->setFocus();
    else
        ui->userInput->setFocus();
#endif
    capsLockCheck();
    ui->rstpwdmessagelabel->clear();

#ifdef SCREENKEYBOARD
    emit sendKeyboardCloseRequest();
#endif


}

#ifdef SCREENKEYBOARD
void LoginForm::focusChanged(QWidget *old, QWidget *now){



    if(now == NULL){
        //  screenKeyboard->close();
#if 0
        if(screenKeyboard->isHidden()){
            ui->loginframe->setFocus();
        }
#endif
        return;
    }



    if (now->objectName().compare(ui->userInput->objectName()) == 0){


        emit sendKeyboardRequest( ui->userInput->mapToGlobal(QPoint(0,0)), ui->userInput->width());

    }else if(now->objectName().compare(ui->passwordInput ->objectName()) == 0){

        emit sendKeyboardRequest(ui->passwordInput->mapToGlobal(QPoint(0,0)), ui->passwordInput->width());

    }else if(now->objectName().compare(ui->oldpasswordinput ->objectName()) == 0){

        emit sendKeyboardRequest( ui->oldpasswordinput->mapToGlobal(QPoint(0,0)), ui->oldpasswordinput->width());

    }else if(now->objectName().compare(ui->newpasswordinput ->objectName()) == 0){

        emit sendKeyboardRequest( ui->newpasswordinput->mapToGlobal(QPoint(0,0)), ui->newpasswordinput->width());

    }else if(now->objectName().compare(ui->newpasswordconfirminput ->objectName()) == 0){

        emit sendKeyboardRequest( ui->newpasswordconfirminput->mapToGlobal(QPoint(0,0)), ui->newpasswordconfirminput->width());

    }
}

void LoginForm::keyboardEvent(QString key){


    QWidget * fw = qApp->focusWidget();
    QString txt;




    if(key.compare(QString("enter")) == 0){
        QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
        QCoreApplication::postEvent ((QObject*)ui->passwordInput, event);
        return;
    }


    if(fw == NULL){
        return;
    }


    QLineEdit *target = NULL;


    if(fw->objectName().compare(ui->userInput->objectName()) == 0){

        target = ui->userInput;
    }else if(fw->objectName().compare(ui->passwordInput->objectName()) == 0){

        target = ui->passwordInput;
    }else if(fw->objectName().compare(ui->oldpasswordinput->objectName()) == 0){

        target = ui->oldpasswordinput;

    }else if(fw->objectName().compare(ui->newpasswordinput->objectName()) == 0){

        target = ui->newpasswordinput;

    }else if(fw->objectName().compare(ui->newpasswordconfirminput->objectName()) == 0){

        target = ui->newpasswordconfirminput;

    }


    if(target != NULL){

        QString tmptextleft;
        QString tmptextright;

        int textlen;

        if(key.compare(QString("clear")) == 0){
            txt = "";
            target->setText(txt);
        }else if(key.compare(QString("backspace")) == 0){

            int cursorpos = target->cursorPosition();
            tmptextleft = target->text().mid(0, cursorpos);
            tmptextright = target->text().mid(cursorpos, target->text().length());
            textlen = tmptextleft.length();
            tmptextleft = tmptextleft.mid(0, textlen - 1);
            txt = tmptextleft + tmptextright;
            target->setText(txt);
            target->setCursorPosition(cursorpos - 1);

        }else{

            txt = target->text() + key;
            target->setText(txt);
        }
    }

}


void LoginForm::keyboardCloseEvent(void){

    QWidget * fw = qApp->focusWidget();

    if(fw == NULL){
        return;
    }

    if(fw->objectName().compare(ui->userInput->objectName()) == 0){
        ui->userInput->clearFocus();
    }else if(fw->objectName().compare(ui->passwordInput->objectName()) == 0){
        ui->passwordInput->clearFocus();

    }else if(fw->objectName().compare(ui->oldpasswordinput->objectName()) == 0){
        ui->oldpasswordinput->clearFocus();

    }else if(fw->objectName().compare(ui->newpasswordinput->objectName()) == 0){
        ui->newpasswordinput->clearFocus();

    }else if(fw->objectName().compare(ui->newpasswordconfirminput->objectName()) == 0){
        ui->newpasswordconfirminput->clearFocus();
    }
}

#endif



void LoginForm::resetRequest(){
    qWarning() << "reset requested";
}

QString LoginForm::getHostname(){

    QString realm = realM;

    if(!realm.isNull() && !realm.isEmpty())
        return realm + " / "+ m_Greeter.hostname();

    return m_Greeter.hostname();
}

QString LoginForm::readRealm(){

    FILE *fp;
    char data[512] = {0};
    bool readerror = false;
    QString  tmpstring;
    QString outstr;
    int read_size;

    // fp = popen("net ads info", "r");
    fp = popen("/usr/sbin/realm list", "r");
    if (fp == NULL) {
        qWarning() << "Realm can not be read 1" ;
        readerror = true;
    }





    if(readerror == false){

        read_size = fread(data, 1, sizeof(data), fp);

        if( read_size < 1){
            qDebug() << tr("Realm can not be read") ;
        }else{
            outstr = getValueOfString(QString::fromLocal8Bit(data, read_size), QString("realm-name"));
        }


    }

    /* close */
    pclose(fp);


    return outstr;

}


QString LoginForm::getUserRealm(QString username){


    if(username.isNull() || username.isEmpty() || !username.compare(tr("Other User")))
        return QString("");

    if(ifLocalUser(username))
        return QString("");
    else
        return realM;
}


bool LoginForm::ifLocalUser(QString username){


    FILE *fp;
    char data[512];
    bool readerror = false;
    QString  tmpstring;
    QString outstr = NULL;
    int read_size;
    QString command;
    QString compare;


    command = QString(" cat /etc/passwd | grep ") + username;

    fp = popen(command.toLocal8Bit().data(), "r");
    if (fp == NULL) {
        qWarning() << "username can not be read from passwd" ;
        readerror = true;
    }


    if(readerror == false){

        read_size = fread(data, 1, sizeof(data), fp);

        if( read_size < 5){
            //  qDebug() << tr("Realm can not be read 2") ;
            readerror = true;
        }else{
            outstr = QString(data);
        }


    }
    pclose(fp);

    if(readerror){
        return false;
    }else{

        compare = username +":";

        if(!outstr.left(compare.length()).compare(compare)){

            return true;
        }

        return false;

    }

    return false;
}


void LoginForm::on_pwShowbutton_pressed()
{
    ui->passwordInput->setEchoMode(QLineEdit::EchoMode::Normal);
    ui->passwordInput->setDisabled(true);
}

void LoginForm::on_pwShowbutton_released()
{
    ui->passwordInput->setEchoMode(QLineEdit::EchoMode::Password);
    ui->passwordInput->setDisabled(false);
    //ui->pwShowbutton->clearFocus();
    //  ui->passwordInput->setFocus();

}

void LoginForm::on_showoldPwdButton_pressed()
{
    ui->oldpasswordinput->setEchoMode(QLineEdit::EchoMode::Normal);
    ui->oldpasswordinput->setDisabled(true);
}

void LoginForm::on_showoldPwdButton_released()
{
    ui->oldpasswordinput->setEchoMode(QLineEdit::EchoMode::Password);
    ui->oldpasswordinput->setDisabled(false);

}

void LoginForm::on_shownewPwdButton_pressed()
{
    ui->newpasswordinput->setEchoMode(QLineEdit::EchoMode::Normal);
    ui->newpasswordinput->setDisabled(true);
}

void LoginForm::on_shownewPwdButton_released()
{
    ui->newpasswordinput->setEchoMode(QLineEdit::EchoMode::Password);
    ui->newpasswordinput->setDisabled(false);
}

void LoginForm::on_showconfirmPwdButton_pressed()
{
    ui->newpasswordconfirminput->setEchoMode(QLineEdit::EchoMode::Normal);
    ui->newpasswordconfirminput->setDisabled(true);
}

void LoginForm::on_showconfirmPwdButton_released()
{
    ui->newpasswordconfirminput->setEchoMode(QLineEdit::EchoMode::Password);
    ui->newpasswordconfirminput->setDisabled(false);
}


void LoginForm::debugBox(QString mes){
    QMessageBox msgBox;
    msgBox.setWindowTitle("title");
    msgBox.setText(mes);
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    if(msgBox.exec() == QMessageBox::Yes){
        // do something
    }else {
        // do something else
    }
}
