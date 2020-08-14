#ifndef GREETER_H
#define GREETER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QLightDM/Power>
#include <QLightDM/Greeter>
#include <QLightDM/SessionsModel>
#include <QLightDM/UsersModel>

class Greeter : public QObject
{
    Q_OBJECT

    friend class DecoratedUsersModel;

    Q_PROPERTY(QString username READ readUserName WRITE writeUserName NOTIFY notifyUsername)
    Q_PROPERTY(QString password READ readPassword WRITE writePassword NOTIFY notifyPassword)
    Q_PROPERTY(QString newpassword READ readNewPassword WRITE writeNewPassword NOTIFY notifyNewPassword)
    Q_PROPERTY(QString session READ readSession WRITE writeSession NOTIFY notifySession)
    Q_PROPERTY(QStringList sessionlist READ readSessionList WRITE writeSessionList NOTIFY notifySessionList)
    Q_PROPERTY(QString userlist READ readUserList WRITE writeUserList NOTIFY notifyUserList)
    Q_PROPERTY(bool logincompleted READ readLoginCompleted WRITE writeLoginCompleted NOTIFY notifyLoginCompleted)
    Q_PROPERTY(bool authenticated READ readAuthenticated WRITE writeAuthenticated NOTIFY notifyAuthenticated)
    Q_PROPERTY(QString message READ readMessage WRITE writeMessage NOTIFY notifyMessage)
    //todo: capslock status
    //todo:reset login

public:
    explicit Greeter(QObject *parent = nullptr);

    QString readUserName(void);
    void writeUserName(QString &username);
    QString readPassword(void);
    void writePassword(QString &password);
    QString readNewPassword(void);
    void writeNewPassword(QString &password);
    QString readSession(void);
    void writeSession(QString &session);
    QStringList readSessionList(void);
    void writeSessionList(QStringList &sessionList);
    QString readUserList(void);
    void writeUserList(QString &userList);
    bool readLoginCompleted(void);
    void writeLoginCompleted(bool logincompleted);
    QString readMessage(void);
    void writeMessage(QString &message);
    bool readAuthenticated(void);
    void writeAuthenticated(bool logincompleted);


private:
    QLightDM::Greeter m_Greeter;
    QLightDM::PowerInterface power;
    QLightDM::SessionsModel sessionsModel;
    QLightDM::UsersModel userModel;

    QMap<int, void (QLightDM::PowerInterface::*)()> powerSlots;

    void cancelLogin(void);
    void Initialize(void);
    void addUsertoCache(QString &user_name);
    void initializeUserList(void);
    void InformFrontEnd(bool status);

    QTimer *loginTimer =NULL;

    QString m_userName;
    QString m_password;
    QString m_newPassword;
    QString m_message;
    QString m_prompt;
    QString m_session;
    QStringList m_userList;

    bool m_logincompleted = false;
    bool m_authenticated = false;

    int loginTimerState = 0;
    int total_user_count;
    bool promptFlag = false;
    bool needReenterOldPassword = false;
    bool needPasswordChange = false;
    bool resetStartFlag = false;
    bool userResetRequest = false;
    bool loginStartFlag = false;
    bool loginprompt = false;
    bool messageReceived = true;

signals:
    void notifyUsername(void);
    void notifyPassword(void);
    void notifyNewPassword(void);
    void notifySession(void);
    void notifySessionList(void);
    void notifyUserList(void);
    void notifyLoginCompleted(void);
    void notifyMessage(void);
    void notifyAuthenticated(void);


public slots:

    void onPrompt(QString prompt, QLightDM::Greeter::PromptType promptType);
    void onMessage(QString prompt, QLightDM::Greeter::MessageType messageType);
    void authenticationComplete();

private slots:
    void LoginTimerFinished();

};

#endif // GREETER_H
