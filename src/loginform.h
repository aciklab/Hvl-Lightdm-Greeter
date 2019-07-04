#ifndef LOGINFORM_H
#define LOGINFORM_H

#include "main.h"

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
#include <QToolButton>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QShortcut>
#include <QLabel>


#include <QLightDM/Power>
#include <QLightDM/Greeter>
#include <QLightDM/SessionsModel>
#include <QLightDM/UsersModel>



#ifdef SCREENKEYBOARD
#include "keyboard.h"
#endif


#define LOGO_LABEL_SIZE     TOOL_BUTTON_ICON_SIZE
#define TOOL_BUTTON_FONT_SIZE  12

#define ANIMATION_TIME 200

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
    QString getHostname();


public slots:
    void cancelLogin();

    void startLogin();
    void onPrompt(QString prompt, QLightDM::Greeter::PromptType promptType);
    void onMessage(QString prompt, QLightDM::Greeter::MessageType messageType);
    void resetRequest();

    void authenticationComplete();
    void stopWaitOperation(const bool& networkstatus);
#ifdef SCREENKEYBOARD

    void keyboardCloseEvent();
    void keyboardEvent(QString key);
#endif


protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

Q_SIGNALS:
    void sendKeyboardRequest(QPoint from, int width);
    void sendKeyboardCloseRequest();




private slots:
    void on_pushButton_resetpwd_clicked();

    void on_resetpasswordButton_clicked();

    void passwordResetTimerFinished();

    void LoginTimerFinished();

    void on_acceptbutton_clicked();

    void on_userInput_editingFinished();

    void on_loginbutton_clicked();

    void on_pwShowbutton_pressed();

    void on_pwShowbutton_released();

    void userButtonClicked();

    void on_backButton_clicked();

    void animationTimerFinished();

    void on_cancelResetButton_clicked();

    void userPasswordResetRequest();


#ifdef SCREENKEYBOARD
    void focusChanged(QWidget *old, QWidget *now);

#endif



    void on_showoldPwdButton_pressed();

    void on_showoldPwdButton_released();

    void on_shownewPwdButton_pressed();

    void on_shownewPwdButton_released();

    void on_showconfirmPwdButton_pressed();

    void on_showconfirmPwdButton_released();

private:
    void initialize();
    QString currentSession();
    void setCurrentSession(QString session);

    void checkPasswordResetButton();
    void initializeUserList();
    void addUsertoCache(QString user);
    void userSelectStateMachine(int key, int button);
    bool capsOn();
    void capsLockCheck();
    QString getValueOfString(QString data, QString value);
    void pageTransition(QWidget *Page);
    void usersbuttonReposition();
    void loginPageTransition();
    void preparetoLogin();
    QString readRealm();
    bool ifLocalUser(QString username);
    QString getUserRealm(QString username);

    Ui::LoginForm *ui;

    QLightDM::Greeter m_Greeter;
    QLightDM::PowerInterface power;
    QLightDM::SessionsModel sessionsModel;
    QLightDM::UsersModel userModel;

    QMap<int, void (QLightDM::PowerInterface::*)()> powerSlots;

    QFrame *user_frame;
    QTimer *resetTimer;
    QTimer *loginTimer;
    QTimer *userRequestResetTimer;
    QTimer *animationTimer;
    QMovie *mv;
    QShortcut *shortcut;




    QString userList[6];
    QPropertyAnimation *anim1[3];
    QPropertyAnimation *anim2[3];
    QToolButton *toolButtons[3];

    QParallelAnimationGroup *animGroup;

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
    int userRequestTimerState;
    QString lastPrompt;
    QString lastMessage;
    bool loginStartFlag;
    bool resetStartFlag;
    QStringList knownUsers;
    int currentUserIndex;
    int loginTimeot;
    bool timeoutFlag;
    bool networkOK;
    int animationTimerState;
    int useroffset;
    int olduseroffset;
    bool animationprogress;
    int lastuserindex;
    int lastkey;

    bool loginprompt;
    bool userResetRequest;
    bool passwordChangeError;
    int nwcheckcount;


    QRect left = QRect(52,25,100,110);
    QRect center = QRect(157,10,120,140);
    QRect right = QRect(282,25,100,110);
    QPixmap iconPassive = QPixmap(":/resources/login_1_1.png");
    QPixmap iconActive = QPixmap(":/resources/login_1.png");
    QPixmap iconEmptyPassive = QPixmap(":/resources/login_1_1_bos.png");
    QPixmap iconEmptyActive = QPixmap(":/resources/login_1_bos.png");

#if 0

    QRect left = QRect(52,25,100,110);
    QRect center = QRect(157,10,120,140);
    QRect right = QRect(282,25,100,110);
#endif


};

#endif // LOGINFORM_H
