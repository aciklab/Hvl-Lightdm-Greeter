/*
* Copyright (c) 2012-2015 Christian Surlykke
*
* This file is part of qt-lightdm-greeter 
* It is distributed under the LGPL 2.1 or later license.
* Please refer to the LICENSE file for a copy of the license.
*/
#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QWidget>
#include <QProcess>
#include <QDialog>
#include <QKeyEvent>
#include <QGraphicsOpacityEffect>
#include <QMap>
#include <QMainWindow>
#include "qquickview.h"
#include <QWebEngineView>
#include <QTimer>
#include <QFrame>


#include <QLightDM/Power>
#include <QLightDM/Greeter>
#include <QLightDM/SessionsModel>


namespace Ui
{
class LoginForm;
}

class LoginForm : public QWidget
{
    Q_OBJECT

    friend class DecoratedUsersModel;

public:
    explicit LoginForm(QWidget *parent = 0);
    ~LoginForm();
    virtual void setFocus(Qt::FocusReason reason);


public slots:
    void cancelLogin();

    void startLogin();
    void onPrompt(QString prompt, QLightDM::Greeter::PromptType promptType);
    void onMessage(QString prompt, QLightDM::Greeter::MessageType messageType);

    void authenticationComplete();
    void stopWaitOperation(const bool& networkstatus);


protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);


private slots:
    void on_pushButton_resetpwd_clicked();

    void on_userbutton1_clicked();

    void on_userbutton2_clicked();

    void on_userbutton3_clicked();

    void on_resetpasswordButton_clicked();

    void passwordResetTimerFinished();

    void LoginTimerFinished();

    void on_acceptbutton_clicked();

    void on_userInput_editingFinished();



    void on_loginbutton_clicked();

private:
    void initialize();
    QString currentSession();
    void setCurrentSession(QString session);

    void checkPasswordResetButton();
    void initializeUserList();
    void addUsertoCache(QString user);
    void userSelectStateMachine(int key, int button);
    void Debug(QString message);
    bool capsOn();
    void capsLockCheck();
    QString getValueOfString(QString data, QString value);
    void pageTransition(QWidget *Page);

    Ui::LoginForm *ui;

    QLightDM::Greeter m_Greeter;
    QLightDM::PowerInterface power;
    QLightDM::SessionsModel sessionsModel;

    QMap<int, void (QLightDM::PowerInterface::*)()> powerSlots;

    QFrame *user_frame;
    QTimer *resetTimer;
    QTimer *loginTimer;
    QMovie *mv;

    QString userList[5];
    int current_user_button;
    int total_user_count;
    bool messageReceived;
    bool needPasswordChange;
    bool needReenterOldPassword;
    bool promptFlag;
    QString oldPassword;
    QString tmpPassword;
    int resetTimerState;
    int loginTimerState;
    QString lastPrompt;
    bool loginStartFlag;
    bool resetStartFlag;
    QStringList knownUsers;
    int currentUserIndex;
    int loginTimeot;
    bool timeoutFlag;





};

#endif // LOGINFORM_H
