
#include <QDebug>
#include <QAbstractListModel>
#include <QString>
#include <QThread>
#include "greeter.h"
#include "settings.h"

Greeter::Greeter(QObject *parent) :
    QObject(parent),
    m_Greeter(),
    power(this),
    sessionsModel(QLightDM::SessionsModel::LocalSessions,this),
    userModel(this)
{
    m_Greeter.setResettable(false);

    Initialize();

    if (!m_Greeter.connectSync()) {
        qCritical() << "Failed to connect lightdm";
        return;
    }

}


void Greeter::Initialize(void){



    connect(&m_Greeter, SIGNAL(showPrompt(QString, QLightDM::Greeter::PromptType)), this, SLOT(onPrompt(QString, QLightDM::Greeter::PromptType)));
    connect(&m_Greeter, SIGNAL(showMessage(QString, QLightDM::Greeter::MessageType)), this, SLOT(onMessage(QString, QLightDM::Greeter::MessageType)));
    connect(&m_Greeter, SIGNAL(authenticationComplete()), this, SLOT(authenticationComplete()));


    loginTimer = new QTimer();
    connect(loginTimer, SIGNAL(timeout()), this, SLOT(LoginTimerFinished()));


    initializeUserList();

}


void Greeter::onPrompt(QString prompt, QLightDM::Greeter::PromptType promptType){

    promptFlag = true;
    m_prompt = prompt;

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
        //pageTransition(ui->warningpage);
        //    ui->warninglabel->setText(tr("Your Password is Expired. You have to change it"));


    }
    else if((prompt.compare("Enter new password: ") == 0 || prompt.compare("New password: ") == 0 ) &&
            resetStartFlag && !needReenterOldPassword){

        needPasswordChange = true;
        //  pageTransition(ui->resetpage);
        userResetRequest = false;

        resetStartFlag = false;
        loginStartFlag = false;
    }else if((prompt.compare("login:") == 0 || prompt.compare("login: ") == 0 )){

        loginprompt = true;

    }

    qInfo() << "Prompt: "<< m_prompt;

}
void Greeter::onMessage(QString prompt, QLightDM::Greeter::MessageType messageType){

    QString type;


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
    m_message = type + " : "+ prompt;

}


void Greeter::authenticationComplete(){

    QString lastuser;

    qInfo() << "tmp complete";
    lastuser = m_userName;

    if (m_Greeter.isAuthenticated()) {

        qInfo() << "Authentication is completed for  " + lastuser;

        /* Reset to default screensaver values */

        needPasswordChange = 0;

        if(m_session.isEmpty() || m_session.isNull())
            m_session = m_Greeter.defaultSessionHint();

        // addUsertoCache(lastuser);
        Cache().setLastUser(lastuser);
        Cache().setLastSession(lastuser, m_session);
        Cache().sync();

        qWarning() <<  "Start session : " << m_session;
        if(!m_Greeter.startSessionSync(m_session)){

            qWarning() <<  "Failed to start session  ";

            if(!needPasswordChange)
                cancelLogin();

            if(!messageReceived)
                m_message = tr("Error : Login Incorrect");
            else if(m_message.isNull() || m_message.isEmpty())
                m_message = tr("Error : Login Incorrect");

            InformFrontEnd(false);

        }
        needPasswordChange = false;

    }else if(loginStartFlag == true || resetStartFlag == true) {

        qWarning() <<  "Authentication error for  " + lastuser;

        qWarning() <<  "message: " + m_message;

        if(!needPasswordChange)
            cancelLogin();

        if(!messageReceived)
            m_message = (tr("Error : Login Incorrect"));
        else if(m_message.isNull() ||  m_message.isEmpty())
            m_message = (tr("Error : Login Incorrect"));

        if(resetStartFlag){

            resetStartFlag = true;
            userResetRequest = false;
            needPasswordChange = false;
            needReenterOldPassword = true;
        }else{
            // preparetoLogin();
        }
        InformFrontEnd(false);

    }

    messageReceived = false;
    loginStartFlag = false;
    resetStartFlag = false;

}

void Greeter::initializeUserList(void){

    total_user_count = 0;

    m_userList.clear();



    for(int i = 0; i < Settings().cachedusercount(); i++){
        m_userList.append( Cache().getLastUserfromIndex(static_cast<uint>(i)));

        if(!m_userList[i].isNull() && !m_userList[i].isEmpty() && m_userList[i].length() > 1)
            total_user_count++;
    }

    for(int i = 0; i < 5; i++){
        if((m_userList[i].isNull() || m_userList[i].isEmpty() || m_userList[i].length() < 2) && i < 4){
            m_userList[i].clear();
            m_userList[i] = m_userList[i + 1];
            m_userList[i + 1].clear();
        }
    }

    m_userList[total_user_count]  = tr("Other User");



    qInfo() <<  (QString::number(total_user_count) + " users found for last users cache");


    m_session = Cache().getLastSession( m_userList[0]);
    qInfo() << "currentSessionStr: " << m_session;

    total_user_count++;

    QString username;
    QString imagepath;
    QModelIndex modelIndex;

    for(int i = 0; i< total_user_count; i++){

        for(int j = 0; j < userModel.rowCount(QModelIndex()); j++){

            modelIndex = userModel.index(j, 0);
            username = userModel.data(modelIndex, QLightDM::UsersModel::NameRole).toString();

            if(i != total_user_count){

                if(username.compare(m_userList[i]) == 0){

                    imagepath = userModel.data(modelIndex, QLightDM::UsersModel::ImagePathRole).toString();
                    break;
                }
            }
        }
    }

}


void Greeter::addUsertoCache(QString &user_name){

    int i;
    int nullfound = 0;



    if(m_userList[0].compare(user_name) == 0)
        return;

    for(i = 0; i< 5; i++){

        if(m_userList[0].isNull() || m_userList[0].isEmpty() ||  m_userList[0].length() < 2){
            m_userList[i].clear();
        }

        if(m_userList[i].compare(user_name) == 0)
            m_userList[i].clear();

        if(i >= Settings().cachedusercount() || m_userList[i].compare(tr("Other User")) == 0)
            m_userList[i].clear();

        if(m_userList[i].isNull() || m_userList[i].isEmpty() || m_userList[i].length() < 2){
            nullfound++;
            m_userList[i].clear();
        }
    }

    if(m_userList[0].isNull() || m_userList[0].isEmpty() ||  m_userList[0].length() < 2){
        m_userList[0].clear();
        m_userList[0] = user_name;
    }else{

        if(nullfound > 0){

            for(i = 4; i >= 1; i--){
                if((m_userList[i].isNull() || m_userList[i].isEmpty() || m_userList[i].length() < 2) && i != 0){


                    if((!m_userList[i - 1].isNull() && !m_userList[i - 1].isEmpty() && m_userList[i - 1].length() > 1) && i != 0){
                        m_userList[i].clear();
                        m_userList[i] = m_userList[i-1];
                        m_userList[i-1].clear();
                    }else{
                        m_userList[i-1].clear();
                    }

                }
            }

            m_userList[0] = user_name;

            for(i = 0; i < 5; i++){

                if((m_userList[i].isNull() || m_userList[i].isEmpty() || m_userList[i].length() < 2) && i != 0){
                    m_userList[i] = m_userList[i+ 1];
                    m_userList[i+ 1].clear();
                }

            }


        }else{
            m_userList[4] = m_userList[3];
            m_userList[3] = m_userList[2];
            m_userList[2] = m_userList[1];
            m_userList[1] = m_userList[0];
            m_userList[0] = user_name;
        }

    }

    for(i = 0; i < Settings().cachedusercount(); i++){
        if(!m_userList[i].isNull() && !m_userList[i].isEmpty() && m_userList[i].length() > 2)
            Cache().setLastUsertoIndex(m_userList[i], static_cast<uint>(i));
        else
            Cache().setLastUsertoIndex("", static_cast<uint>(i));

    }

    m_userList[total_user_count - 1] = tr("Other User");

}



void Greeter::cancelLogin(){

    //if (m_Greeter.inAuthentication()) {
    m_Greeter.cancelAuthentication();
    qWarning() << "Authentication is canceled";

    //}

}



void Greeter::LoginTimerFinished(){

    bool endFlag = 0;
    static int promptCheckCounter;
    static int userCheckCounter;

    switch(loginTimerState){

    case 0:
        //cancelLogin();
        userCheckCounter = 0;
        loginTimerState = 1;
        m_prompt.clear();
        loginTimer->setInterval(100);
        promptFlag = false;
        break;

    case 1:
        //enter user

        if(!m_Greeter.isAuthenticated()){

            //cancelLogin();
        }

        qInfo() << "Login start for: " + m_userName;
        m_prompt.clear();
        loginTimerState = 2;
        messageReceived = false;
        promptCheckCounter = 0;

        m_Greeter.authenticate(m_userName);
        loginTimer->setInterval(100);
        break;

    case 2:

        if(loginprompt){
            loginTimer->setInterval(100);
            loginprompt = false;

        }else{

            if(m_Greeter.isAuthenticated()){
                qWarning() << "user already Authenticated";
                m_Greeter.startSessionSync(m_session);
                endFlag = 1;
                promptCheckCounter = 0;


            }else if(m_prompt.compare("Password: ") == 0 || m_prompt.compare("password: ") == 0 || m_prompt.compare("\"Password: \"") == 0){

                qInfo() << "password is sending: " << m_password;
                m_Greeter.respond(m_password);
                endFlag = 1;
                promptCheckCounter = 0;
            }else if(promptCheckCounter > 40){

                endFlag = 1;
                cancelLogin();
                messageReceived = 0;
                loginStartFlag = false;
                loginTimer->setInterval(100);
                qWarning() << "No password prompt received";
                qWarning() << "Authentication is canceled";
                m_message = tr("No password prompt received");
                InformFrontEnd(false);

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

    }

    if(endFlag == 1){

        loginTimerState = 0;
        loginTimer->stop();
        messageReceived = 0;



    }else{

        loginTimer->stop();
        loginTimer->start();
        loginStartFlag = true;
    }
}


void Greeter::InformFrontEnd(bool status){

    m_logincompleted = true;
    m_authenticated = status;
    emit notifyLoginCompleted();


}

/*Backend functions
 *
 *
 */

QString Greeter::readUserName(void){
    return m_userName;
}

void Greeter::writeUserName(QString &username){
    m_userName = username;
}


QString Greeter::readPassword(void){
    return m_password;
}

void Greeter::writePassword(QString &password){

    if(loginStartFlag)
        return;

    m_password = password;

    if(m_userName.length() >0){
        loginTimer->setTimerType(Qt::TimerType::PreciseTimer);
        loginTimer->setSingleShot(true);
        loginTimer->setInterval(100);
        loginTimerState = 0;
        loginStartFlag = true;
        loginTimer->start();

    }

}

QString Greeter::readNewPassword(void){

}

void Greeter::writeNewPassword(QString &password){

    if(m_userName.length() >0 && m_password.length() > 0
            &&  m_newPassword.length() > 0){

    }


}

QString Greeter::readSession(void){

}

void Greeter::writeSession(QString &session){

}


QStringList Greeter::readSessionList(void){

}

void Greeter::writeSessionList(QStringList &sessionList){

}


QString Greeter::readUserList(void){
    QString out = "";

    for(int i = 0; i< m_userList.length(); i++)
        out += m_userList[i] + ",";

    return out;
}

void Greeter::writeUserList(QString &userList){

}


bool Greeter::readLoginCompleted(void){
    return m_logincompleted;
}

void Greeter::writeLoginCompleted(bool logincompleted){
    m_logincompleted = logincompleted;
}


bool Greeter::readAuthenticated(void){
    return m_authenticated;
}

void Greeter::writeAuthenticated(bool authenticated){
    m_authenticated = authenticated;
}



QString Greeter::readMessage(void){
    return m_message;
}
void Greeter::writeMessage(QString &message){
    m_message = message;
}
