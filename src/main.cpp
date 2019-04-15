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
#include <initializer_list>
#include <signal.h>
#include <unistd.h>

#include <iostream>

#include <sys/mman.h>


#include "settings.h"

#include "main.h"
#include "mainwindow.h"

QFile logfile;
QTextStream ts;
int m_Screen;
int timeout, interval, prefer_blanking, allow_exposures;

void syncX();
void catchUnixSignals(std::initializer_list<int> quitSignals);
void changeResolution();
void setScreensaver();
void restoreScreensaver();
bool lockhint = false;


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

    m_Screen = 0;
   // changeResolution();//gm_edition

    // use std::cerr for logs
    qInstallMessageHandler(messageHandler);
    Cache::prepare();

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});
    m_Screen = QApplication::desktop()->primaryScreen();


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
    for (int i = 0; i < QApplication::desktop()->screenCount(); ++i) {
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



#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <QtX11Extras/QX11Info>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include<X11/extensions/Xrandr.h>

void syncX(){
#if 0
    Display *display = QX11Info::display();
    int screen = XDefaultScreen (display);
    Window w = RootWindow (display, screen);
    Atom id = XInternAtom (display, "AT_SPI_BUS", True);
    if (id != None)
    {
        XDeleteProperty (display, w, id);
        XSync (display, false);
    }

#endif
}



void ignoreUnixSignals(std::initializer_list<int> ignoreSignals) {
    // all these signals will be ignored.
    for (int sig : ignoreSignals)
        signal(sig, SIG_IGN);


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




void changeResolution(){



    Display                 *dpy;
    Window                  root;
    int                     num_sizes;
    XRRScreenSize           *xrrs;
    XRRScreenConfiguration  *conf;
    short                   possible_frequencies[64][64];
    short                   original_rate;
    Rotation                original_rotation;
    SizeID                  original_size_id;

    QRect screenRect = QApplication::desktop()->screenGeometry(m_Screen);

    if(screenRect.width() <= 1920 && screenRect.height() <= 1080){
        //return;
    }


    //     CONNECT TO X-SERVER, GET ROOT WINDOW ID
    //
    dpy    = XOpenDisplay(NULL);
    root   = RootWindow(dpy, 0);
    //
    //     GET POSSIBLE SCREEN RESOLUTIONS
    //
    xrrs   = XRRSizes(dpy, 0, &num_sizes);
    //
    //     LOOP THROUGH ALL POSSIBLE RESOLUTIONS,
    //     GETTING THE SELECTABLE DISPLAY FREQUENCIES
    //
    for(int i = 0; i < num_sizes; i ++) {
        short   *rates;
        int     num_rates;

        printf("\n\t%2i : %4i x %4i   (%4imm x%4imm ) ", i, xrrs[i].width, xrrs[i].height, xrrs[i].mwidth, xrrs[i].mheight);

        rates = XRRRates(dpy, 0, i, &num_rates);

        for(int j = 0; j < num_rates; j ++) {
            possible_frequencies[i][j] = rates[j];
            printf("%4i ", rates[j]); } }

    printf("\n");
    //
    //     GET CURRENT RESOLUTION AND FREQUENCY
    //
    conf                   = XRRGetScreenInfo(dpy, root);
    original_rate          = XRRConfigCurrentRate(conf);
    original_size_id       = XRRConfigCurrentConfiguration(conf, &original_rotation);

    printf("\n\tCURRENT SIZE ID  : %i\n", original_size_id);
    printf("\tCURRENT ROTATION : %i \n", original_rotation);
    printf("\tCURRENT RATE     : %i Hz\n\n", original_rate);
    //
    //     CHANGE RESOLUTION
    //

    for(int i = 0; i < num_sizes; i ++) {

        if(xrrs[i].width == 1360 && xrrs[i].height == 768){

            printf("\tCHANGED TO %i x %i PIXELS, %i Hz\n\n", xrrs[i].width, xrrs[i].height, possible_frequencies[i][0]);
            XRRSetScreenConfigAndRate(dpy, conf, root, i, RR_Rotate_0, possible_frequencies[i][0], CurrentTime);
        }
    }


    //
    //     SLEEP A WHILE
    //
    usleep(5000000);
    //
    //     RESTORE ORIGINAL CONFIGURATION
    //
    //  printf("\tRESTORING %i x %i PIXELS, %i Hz\n\n", xrrs[original_size_id].width, xrrs[original_size_id].height, original_rate);
    //XRRSetScreenConfigAndRate(dpy, conf, root, original_size_id, original_rotation, original_rate, CurrentTime);
    //
    //     EXIT
    //
    XCloseDisplay(dpy);
}


void setScreensaver(){


    Display *display = QX11Info::display();
    XGetScreenSaver (display, &timeout, &interval, &prefer_blanking, &allow_exposures);
    XForceScreenSaver (display, ScreenSaverActive);
    //XSetScreenSaver (display, 60, 0, ScreenSaverActive, DefaultExposures);
    XSetScreenSaver (display, 60, 0, ScreenSaverActive, DefaultExposures);

}

void restoreScreensaver(){

    Display *display = QX11Info::display();
    XSetScreenSaver (display, timeout, interval, prefer_blanking, allow_exposures);
}
