/*
* Copyright (c) 2012-2015 Christian Surlykke
*
* This file is part of qt-lightdm-greeter 
* It is distributed under the LGPL 2.1 or later license.
* Please refer to the LICENSE file for a copy of the license.
*/
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


#include "loginform.h"
#include "ui_loginform.h"
#include "settings.h"
#include "dialog_webview.h"
#include <pwd.h>
#include "settings.h"




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


    view = new QWebEngineView();
    ui->setupUi(this);
    initialize();
}

LoginForm::~LoginForm()
{
    delete ui;
    delete view;
    delete user_frame;

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


    QString imagepath = Settings().logopath();

    if(imagepath.isNull()){
        ui->logolabel->setPixmap(icon.scaled(ui->logolabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }else{

        QPixmap logoicon(imagepath);
         ui->logolabel->setPixmap(logoicon.scaled(ui->logolabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    setCurrentSession(m_Greeter.defaultSessionHint());

    //connect(ui->userInput, SIGNAL(editingFinished()), this, SLOT(EditFinished()));
    connect(&m_Greeter, SIGNAL(showPrompt(QString, QLightDM::Greeter::PromptType)), this, SLOT(onPrompt(QString, QLightDM::Greeter::PromptType)));
    connect(&m_Greeter, SIGNAL(showMessage(QString,QLightDM::Greeter::MessageType)), this, SLOT(onMessage(QString,QLightDM::Greeter::MessageType)));
    connect(&m_Greeter, SIGNAL(authenticationComplete()), this, SLOT(authenticationComplete()));

    //ui->passwordInput->setEnabled(false);
    ui->passwordInput->clear();

    initializeUserList();
    checkPasswordResetButton();

    QLightDM::UsersModel usersModel;
    for (int i = 0; i < usersModel.rowCount(QModelIndex()); i++) {
        knownUsers << usersModel.data(usersModel.index(i, 0), QLightDM::UsersModel::NameRole).toString();
    }
    if (! m_Greeter.hideUsersHint()) {
        ui->userInput->setCompleter(new QCompleter(knownUsers));
        ui->userInput->completer()->setCompletionMode(QCompleter::InlineCompletion);
    }

    QString user = Cache().getLastUser();

    if (user.isEmpty()) {
        user = m_Greeter.selectUserHint();
    }
    ui->userInput->setText(user);
    userChanged();

    message_received = 0;
    needPasswordChange = 0;
    needReenterOldPassword = 0;

}

void LoginForm::userChanged()
{

    ui->messagelabel->clear();

    if (m_Greeter.inAuthentication()) {
        m_Greeter.cancelAuthentication();


    }
    else {
        ui->userInput->setFocus();

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

    loginTimer = new QTimer();
    loginTimer->setTimerType(Qt::TimerType::CoarseTimer);

    loginTimer->setSingleShot(false);

    loginTimer->setInterval(10);
    loginTimerState = 0;
    connect(loginTimer, SIGNAL(timeout()), this, SLOT(LoginTimerFinished()));
    loginTimer->start();



}

void LoginForm::onPrompt(QString prompt, QLightDM::Greeter::PromptType promptType)
{
    //ui->passwordInput->setEnabled(true);
    //ui->passwordInput->setFocus();

    promptFlag = true;
    lastPrompt = prompt;


    //"password: "
    //"Enter new password: "
    //"Enter it again: "

    if((prompt.compare(tr("Enter new password: ")) == 0)&& needPasswordChange != 1 && !resetStartFlag){
        needPasswordChange = 1;
        ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->warningpage));
        oldPassword = tmpPassword;

        ui->newpasswordinput->setEnabled(true);
        ui->newpasswordconfirminput->setEnabled(true);
        ui->newpasswordinput->clear();
        ui->newpasswordconfirminput->clear();
        ui->oldpasswordinput->setText(oldPassword.trimmed());
        ui->oldpasswordinput->setEnabled(true);
        ui->acceptbutton->setFocus();
    }else if((prompt.compare(tr("(current) UNIX password: ")) == 0)&& needPasswordChange != 1 && !resetStartFlag){
        needReenterOldPassword = 1;
        needPasswordChange = 1;
        ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->warningpage));
        oldPassword = tmpPassword;

        ui->newpasswordinput->setEnabled(true);
        ui->newpasswordconfirminput->setEnabled(true);
        ui->newpasswordinput->clear();
        ui->newpasswordconfirminput->clear();
        ui->oldpasswordinput->setText(oldPassword.trimmed());
        ui->oldpasswordinput->setEnabled(true);
        ui->acceptbutton->setFocus();
    }

}

void LoginForm::onMessage(QString prompt, QLightDM::Greeter::MessageType messageType){
    QString type = NULL;

    if(messageType == QLightDM::Greeter::MessageTypeError){
        type = "Error";
    }
    else if(messageType == QLightDM::Greeter::MessageTypeInfo){
        type = "Info";
    }
    else{
        type = "";
    }

    if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage)){
        message_received = 1;
        ui->messagelabel->setText(type + " : " + prompt);
        ui->messagelabel_2->setText(type + " : " + prompt);
    }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->warningpage)){
        ui->messagelabel_2->setText(type + " : " + prompt);
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

    }else{
        ui->passwordInput->clear();

        if(!needPasswordChange)
            userChanged();

        if(!message_received)
            ui->messagelabel->setText("Error : Login Incorrect");

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

        message_received = 0;
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

    for(i = 0; i< 5; i++){
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



    cached_user_count = 1;

    for(int i = 0; i < 5; i++){
        userList[i] = Cache().getLastUserfromIndex(i);
    }

    if(userList[0] != NULL){

        ui->userbutton1->setText(" " + Cache().getLastUserfromIndex(0));
    }
    else if(Cache().getLastUser() != NULL)
    {
        ui->userbutton1->setText(" " + Cache().getLastUser());
    }
    else{
        ui->userbutton1->hide();
    }



    if(userList[1] != NULL){

        ui->userbutton2->setText(" " + Cache().getLastUserfromIndex(1));
        cached_user_count++;
    }
    else
    {
        ui->userbutton2->setEnabled(false);
        ui->userbutton2->hide();
    }

    if(userList[2] != NULL){

        ui->userbutton3->setText(" " + Cache().getLastUserfromIndex(2));
        cached_user_count++;
    }
    else
    {
        ui->userbutton3->setEnabled(false);
        ui->userbutton3->hide();
    }


    if(userList[3] != NULL){

        ui->userbutton4->setText(" " + Cache().getLastUserfromIndex(3));
        cached_user_count++;
    }
    else
    {
        ui->userbutton4->setEnabled(false);
        ui->userbutton4->hide();
    }


    if(userList[4] != NULL){

        ui->userbutton5->setText(" " + Cache().getLastUserfromIndex(4));
        cached_user_count++;
    }
    else
    {
        ui->userbutton5->setEnabled(false);
        ui->userbutton5->hide();
    }

    current_user_button = 0;
    ui->userbutton1->setStyleSheet( "background-color: rgba(200, 200, 200,0.8)");

    if(Cache().getLastUser() != NULL){
        ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->userspage));
        ui->userspage->setFocus();
        ui->userspage->setFocusPolicy(Qt::FocusPolicy::WheelFocus);
    }else{
        ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->loginpage));
    }


}


void LoginForm::userSelectStateMachine(int key, int button){


    if(key == Qt::Key_Up)
        current_user_button--;
    if(key == Qt::Key_Down)
        current_user_button++;

    if(current_user_button < 0)
        current_user_button = 0;

    if(current_user_button >= cached_user_count)
        current_user_button = cached_user_count - 1;

    if(button >= 0)
        current_user_button  = button - 1;

    ui->usersframe->setFocus();

    ui->userbutton1->setStyleSheet("background-color: rgba(200, 200, 200,0.1)");
    ui->userbutton2->setStyleSheet("background-color: rgba(200, 200, 200,0.1)");
    ui->userbutton3->setStyleSheet("background-color: rgba(200, 200, 200,0.1)");
    ui->userbutton4->setStyleSheet("background-color: rgba(200, 200, 200,0.1)");
    ui->userbutton5->setStyleSheet("background-color: rgba(200, 200, 200,0.1)");

    switch(current_user_button){
    case 0:
        ui->userbutton1->setStyleSheet("background-color: rgba(200, 200, 200,0.8)");
        ui->userInput->setText(ui->userbutton1->text().trimmed());
        ui->passwordInput->setFocus();
        break;

    case 1:
        ui->userbutton2->setStyleSheet("background-color: rgba(200, 200, 200,0.8)");
        ui->userInput->setText(ui->userbutton2->text().trimmed());
        ui->passwordInput->setFocus();
        break;

    case 2:
        ui->userbutton3->setStyleSheet("background-color: rgba(200, 200, 200,0.8)");
        ui->userInput->setText(ui->userbutton3->text().trimmed());
        ui->passwordInput->setFocus();
        break;

    case 3:
        ui->userbutton4->setStyleSheet("background-color: rgba(200, 200, 200,0.8)");
        ui->userInput->setText(ui->userbutton4->text().trimmed());
        ui->passwordInput->setFocus();
        break;

    case 4:
        ui->userbutton5->setStyleSheet("background-color: rgba(200, 200, 200,0.8)");
        ui->userInput->setText(ui->userbutton5->text().trimmed());
        ui->passwordInput->setFocus();
        break;
    }

}


void LoginForm::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->userspage)){
            ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->loginpage));
            capsLockCheck();
        }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->resetpage))
            on_resetpasswordButton_clicked();
        else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->warningpage)){
            on_acceptbutton_clicked();
        }
        else
            startLogin();
    }
    else if (event->key() == Qt::Key_Escape) {

        needPasswordChange = 0;
        userChanged();

        if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->userspage)){
            ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->loginpage));
            ui->userInput->clear();
            ui->passwordInput->clear();
            ui->userInput->setFocus();
            capsLockCheck();


        }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->loginpage) && !loginStartFlag){

            if(Cache().getLastUser() != NULL){
                ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->userspage));
                ui->userInput->clear();
                ui->passwordInput->clear();
                userSelectStateMachine(0,1);
            }

        }else if(ui->stackedWidget->currentIndex() == ui->stackedWidget->indexOf(ui->resetpage)){
            ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->loginpage));
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
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->loginpage));
    ui->userspage->setFocus();
    userSelectStateMachine(0,1);
    ui->passwordInput->setFocus();
    capsLockCheck();
}

void LoginForm::on_userbutton2_clicked()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->loginpage));
    ui->userspage->setFocus();
    userSelectStateMachine(0,2);
    ui->passwordInput->setFocus();
    capsLockCheck();

}

void LoginForm::on_userbutton3_clicked()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->loginpage));
    ui->userspage->setFocus();
    userSelectStateMachine(0,3);
    ui->passwordInput->setFocus();
    capsLockCheck();
}

void LoginForm::on_userbutton4_clicked()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->loginpage));
    ui->userspage->setFocus();
    userSelectStateMachine(0,4);
    ui->passwordInput->setFocus();
    capsLockCheck();
}

void LoginForm::on_userbutton5_clicked()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->loginpage));
    ui->userspage->setFocus();
    userSelectStateMachine(0,5);
    ui->passwordInput->setFocus();
    capsLockCheck();

}

void LoginForm::Debug(QString message){

    qDebug() << message;
}


void LoginForm::on_resetpasswordButton_clicked()
{
    if(ui->oldpasswordinput->text().isEmpty()){
        ui->rstpwdmessagelabel->setText(tr("Eski şifre yanlış girildi"));
        return;
    }

    if(ui->newpasswordinput->text().isEmpty() || ui->newpasswordconfirminput->text().isEmpty()){
        ui->rstpwdmessagelabel->setText(tr("Girilen yeni şifreler aynı değil"));
        return;
    }

    if(ui->oldpasswordinput->text().trimmed().compare(oldPassword)){

        ui->rstpwdmessagelabel->setText(tr("Eski şifre yanlış girildi"));

    }else if(ui->newpasswordconfirminput->text().trimmed().compare(ui->newpasswordinput->text().trimmed())){

        ui->rstpwdmessagelabel->setText(tr("Girilen yeni şifreler aynı değil"));

    }else{
        //everything is ok
        resetTimer = new QTimer();
        resetTimer->setTimerType(Qt::TimerType::CoarseTimer);

        resetTimer->setSingleShot(false);

        if(needPasswordChange == 0){
            m_Greeter.cancelAuthentication();
            needPasswordChange = true;
            resetTimerState = 0;
            resetTimer->setInterval(10);

        }else if(needReenterOldPassword){
            needReenterOldPassword = 0;
            resetTimerState = 3;
            resetTimer->setInterval(10);
        }else{

            resetTimerState = 4;
        }

        connect(resetTimer, SIGNAL(timeout()), this, SLOT(passwordResetTimerFinished()));
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
    bool userFoundFlag = false;
    QString tempStr;

    switch(loginTimerState){
    case 0:
        m_Greeter.cancelAuthentication();
        userCheckCounter = 0;
        loginTimerState = 1;
        lastPrompt.clear();
        break;

    case 1:
        //enter user

        for(int i = 0; i < knownUsers .count(); i++){

            tempStr = knownUsers[i];

            if(tempStr.trimmed().compare(ui->userInput->text().trimmed()) == 0)
                userFoundFlag = true;
        }

        if(!userFoundFlag){
            knownUsers.clear();
            QLightDM::UsersModel usersModel;
            for (int i = 0; i < usersModel.rowCount(QModelIndex()); i++) {
                knownUsers << usersModel.data(usersModel.index(i, 0), QLightDM::UsersModel::NameRole).toString();
            }
            if (! m_Greeter.hideUsersHint()) {
                ui->userInput->setCompleter(new QCompleter(knownUsers));
                ui->userInput->completer()->setCompletionMode(QCompleter::InlineCompletion);
            }

            for(int i = 0; i < knownUsers .count(); i++){

                tempStr = knownUsers[i];

                if(tempStr.trimmed().compare(ui->userInput->text().trimmed())== 0)
                    userFoundFlag = true;
            }

        }

        if(!userFoundFlag && userCheckCounter < 8){

            loginTimerState = 1;
            userCheckCounter++;
        }else{
            m_Greeter.authenticate(ui->userInput->text());
            loginTimerState = 2;
            promptCheckCounter = 0;
        }
        lastPrompt.clear();
        break;

    case 2:
        if(lastPrompt.compare("password: ") != 0 && promptCheckCounter < 100){
            promptCheckCounter++;
        }else{
            m_Greeter.respond(ui->passwordInput->text().trimmed());
            loginTimerState = 3;
            endFlag = 1;

        }
        break;
    }

    if(endFlag == 1){
        loginTimerState = 0;
        loginTimer->stop();
        disconnect(loginTimer, SIGNAL(timeout()), this, SLOT(LoginTimerFinished()));
        delete loginTimer;
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
        if(lastPrompt.compare("(current) UNIX password: ") == 0)
            m_Greeter.respond(oldPassword.trimmed());
        resetTimerState = 4;
        break;

    case 4:
        //"Enter new password: "
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
        disconnect(resetTimer, SIGNAL(timeout()), this, SLOT(passwordResetTimerFinished()));
        delete resetTimer;
    }else{
        resetTimer->stop();
        resetTimer->setInterval(500);
        resetTimer->start();
        resetStartFlag = true;
    }

}

void LoginForm::on_acceptbutton_clicked()
{
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->resetpage));
    ui->newpasswordinput->setFocus();

}


bool LoginForm::capsOn()
{

    FILE *fp;
    char path[2];
    char readbyte;
    bool ret = false;

    fp = fopen("/sys/class/input/event0/device/input0::capslock/brightness", "rw");
    if(fp == NULL){
        qDebug() << "Unable to read capslock status\n" ;
        return false;
    }

    if (fgets(path, sizeof(path),fp) == NULL) {
        qDebug() << "Unable to read capslock status\n" ;
        fclose(fp);
        return false;
    }

    fclose(fp);

    if(path[0] == '0')
        ret = false;
    else
        ret = true;

    return ret;


}

void LoginForm::on_userInput_editingFinished()
{

    capsLockCheck();

}


void LoginForm::capsLockCheck(){


    //check caps lock position
    if(capsOn()){
        ui->messagelabel->setText("Caps Lock Açık");
    }else{
        if(ui->messagelabel->text().compare("Caps Lock Açık") == 0)
            ui->messagelabel->clear();

    }

}

