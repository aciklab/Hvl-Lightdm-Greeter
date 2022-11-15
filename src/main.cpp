#include <QtWidgets/QApplication>
#include <QDesktopWidget>
#include <QtGlobal>
#include <QtDebug>
#include <QSettings>
#include <QIcon>
#include <QTranslator>
#include <QString>
#include <QLocale>
#include <QRegularExpression>
#include <QObject>
#include <QGuiApplication>
#include <initializer_list>
#include <signal.h>
#include <unistd.h>

#include <iostream>

#include <sys/mman.h>


#include "settings.h"

#include "main.h"
#include "mainwindow.h"
#include "x11_utils.h"

QFile logfile;
QTextStream ts;

QScreen *screen;
extern bool lockhint;


void catchUnixSignals(std::initializer_list<int> quitSignals);

void messageHandler(QtMsgType type, const QMessageLogContext&, const QString& msg)
{
    std::cerr << type << ": " << QTime::currentTime().toString().toLatin1().data() << " " << msg.toLatin1().data() << "\n";
}

int main(int argc, char *argv[])
{

    QLightDM::Greeter m_Greeter;

    /* Prevent memory from being swapped out, as we are dealing with passwords */
    mlockall (MCL_CURRENT | MCL_FUTURE);

#if 0
    if (m_Greeter.connectSync()) {

        /* Make the greeter behave a bit more like a screensaver if used as un/lock-screen by blanking the screen */
        lockhint = m_Greeter.lockHint();

        if (lockhint)
        {

            setScreensaver();
        }


    }
#endif
    openNumlock();

    qInstallMessageHandler(messageHandler);
    Cache::prepare();

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});
    screen = QGuiApplication::primaryScreen();


    qDebug() << "hvl-lightdm is starting now";

    system("/usr/lib/at-spi2-core/at-spi-bus-launcher --launch-immediately &");

    QString locale = QLocale::system().name();

    QTranslator translator;

    if(!translator.load(locale, "/usr/share/lightdm/lightdm-hvl-greeter.conf.d/lang")){

        if(!translator.load(locale.section('_', 0, 0) , "/usr/share/lightdm/lightdm-hvl-greeter.conf.d/lang")){
            qInfo() << "Failed to load translator file for locale  " + locale;
        }
    }
    a.installTranslator(&translator);

    if (! Settings().iconThemeName_loginform().isEmpty()) {
        QIcon::setThemeName(Settings().iconThemeName_loginform());
    }

    MainWindow *focusWindow = 0;
    for (int i = 0; i < QGuiApplication::screens().length(); ++i) {
        MainWindow *w = new MainWindow(i);
        w->show();
        if (w->showLoginForm())
            focusWindow = w;
    }

    // Ensure we set the primary screen's widget as active when there
    // are more screens
    if (focusWindow) {
        focusWindow->setFocus(Qt::OtherFocusReason);
        focusWindow->activateWindow();
    }



    int ret = a.exec();
    if (lockhint){
        restoreScreensaver();
    }

    syncX();

    qInfo() << "greeeter return value:  " << ret;

    //return ret;
    return 0;
}

void catchUnixSignals(std::initializer_list<int> quitSignals) {
    auto handler = [](int sig) -> void {
        // blocking and not aysnc-signal-safe func are valid
        qInfo() << "quit the application by signal " << sig;
        QCoreApplication::quit();
    };

    sigset_t blocking_mask;
    sigemptyset(&blocking_mask);
    for (auto sig : quitSignals)
        sigaddset(&blocking_mask, sig);

    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_mask    = blocking_mask;
    sa.sa_flags   = 0;

    for (auto sig : quitSignals)
        sigaction(sig, &sa, nullptr);
}
