#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

#include <QLightDM/Greeter>

#include "loginform.h"
#include "settingsform.h"
#include "clockform.h"
#include "powerform.h"

#include "main.h"

#ifdef SCREENKEYBOARD
#include "keyboard.h"
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(int screen, QWidget *parent = 0);
    ~MainWindow();

    void setFocus(Qt::FocusReason reason);

    bool showLoginForm();
    int m_Screen;

    LoginForm* loginForm() { return m_LoginForm;}

    //SettingsForm* settingsForm() { return m_SettingsForm;}

Q_SIGNALS:
    void networkInfo(bool connected);
    void keyboardClosed();
    void sendKeytoChilds(QString key);
    void sendNetworkStatustoChilds(bool connected);

public slots:
    void receiveKeyboardRequest(QPoint from, int width);
    void receiveKeyboardClose();
    void sendKeyPress(QString key);
    void receiveNetworkStatus(bool connected);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    int getOffset(QString offset, int maxVal, int defaultVal);
    void setBackground(bool start);
    void setRootBackground(QImage img);
    void moveForms(int screen_number);

    void keyboardInit();
    void checkNetwork();
    QImage resizeImage(QRect screen_rect, QImage input_image);




    LoginForm* m_LoginForm;
    clockForm *m_ClockForm;
    SettingsForm *m_SettingsForm;
    PowerForm *m_PowerForm;
    static int image_index;
    static bool selectflag;

    int currentScreen;
    int previousScreen;
    static int widgetScreen;
    int screenCount;
    static MainWindow **mainWindowsList;
    QPoint mouseppos;
    QLabel *hostNameLabel;
    int initialScreenset;
    int mirrored;

#ifdef SCREENKEYBOARD
    Keyboard *screenKeyboard;
    bool keyboardisactive;
#endif



};




#endif // MAINWINDOW_H
