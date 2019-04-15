#include <QRect>
#include <QApplication>
#include <QDesktopWidget>
#include <QPalette>
#include <QString>
#include <QDebug>
#include <QtNetwork/QtNetwork>
#include <QMessageBox>

#include "mainwindow.h"
#include "loginform.h"
#include "settings.h"
#include "settingsform.h"
#include "clockform.h"
#include "powerform.h"

#include "stdlib.h"


#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <QtX11Extras/QX11Info>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/extensions/Xrandr.h>


#ifdef SCREENKEYBOARD
#include "keyboard.h"
#endif

#define LOGINFORM_DEFAULT_WIDTH 800
#define LOGINFORM_DEFAULT_HEIGHT 336


bool MainWindow::selectflag = false;
int MainWindow::image_index = 0;
MainWindow **MainWindow::mainWindowsList = NULL;

int MainWindow::widgetScreen = 0;

MainWindow::MainWindow(int screen, QWidget *parent) :
    QWidget(parent),
    m_Screen(screen)
{
    setObjectName(QString("MainWindow_%1").arg(screen));

    QRect screenRect = QApplication::desktop()->screenGeometry(screen);
    setGeometry(screenRect);


    if(mainWindowsList != NULL)
        mainWindowsList[m_Screen] = this;


    previousScreen = 1;

    setBackground();

    // display login dialog only in the main screen
    
    if (m_Screen == QApplication::desktop()->primaryScreen()) {

        if(QApplication::desktop()->screenCount() > 1){
            mainWindowsList = (MainWindow**)malloc(sizeof(MainWindow*) * QApplication::desktop()->screenCount());
            mainWindowsList[m_Screen] = this;
        }

        qApp->installEventFilter(this);

        currentScreen = m_Screen;

        widgetScreen = m_Screen;

        m_LoginForm = new LoginForm(this);

        int maxX = screenRect.width() - m_LoginForm->width();
        int maxY = screenRect.height() - m_LoginForm->height();
        int defaultX = 50*maxX/100;
        int defaultY = 30*maxY/100;
        int offsetX = getOffset(Settings().offsetX_loginform(), maxX, defaultX);
        int offsetY = getOffset(Settings().offsetY_loginform(), maxY, defaultY);
        
        m_LoginForm->move(offsetX, offsetY);
        m_LoginForm->show();

        m_SettingsForm = new SettingsForm(this);


        maxX = screenRect.width() - m_SettingsForm->width();
        maxY = screenRect.height() - m_SettingsForm->height();
        defaultX = 100*maxX/100;
        defaultY = 80*maxY/100;
        offsetX = getOffset(Settings().offsetX_settingsform(), maxX, defaultX);
        offsetY = getOffset(Settings().offsetY_settingsform(), maxY, defaultY);


        m_SettingsForm->move(offsetX, offsetY);
        m_SettingsForm->show();

        m_PowerForm = new PowerForm(this);

        maxX = screenRect.width() - m_PowerForm->width();
        maxY = screenRect.height() - m_PowerForm->height();

        defaultX = 50*maxX/100;
        defaultY = 80*maxY/100;
        offsetX = getOffset(Settings().offsetX_powerform(), maxX, defaultX);
        offsetY = getOffset(Settings().offsetY_powerform(), maxY, defaultY);

        m_PowerForm->move(offsetX, offsetY);
        m_PowerForm->show();


        m_ClockForm = new clockForm(this);

        maxX = screenRect.width();
        maxY = screenRect.height();

        maxX = screenRect.width() - m_ClockForm->width();
        maxY = screenRect.height() - m_ClockForm->height();
        defaultX = 5*maxX/100;
        defaultY = 5*maxY/100;
        offsetX = getOffset(Settings().offsetX_clockform(), maxX, defaultX);
        offsetY = getOffset(Settings().offsetY_clockform(), maxY, defaultY);

        m_ClockForm->move(offsetX, offsetY);
        m_ClockForm->show();

        // This hack ensures that the primary screen will have focus
        // if there are more screens (move the mouse cursor in the center
        // of primary screen - not in the center of all X area). It
        // won't affect single-screen environments.
        int centerX = screenRect.width()/2 + screenRect.x();
        int centerY = screenRect.height()/2 + screenRect.y();
        QCursor::setPos(centerX, centerY);
        this->cursor().setShape(Qt::ArrowCursor);

        QObject::connect(m_SettingsForm, &SettingsForm::sendNWStatusSignal, this, &MainWindow::receiveNetworkStatus);
        QObject::connect(this, &MainWindow::sendNetworkStatustoChilds, m_LoginForm, &LoginForm::stopWaitOperation);

        keyboardInit();

    }

}

MainWindow::~MainWindow()
{
}

bool MainWindow::showLoginForm()
{
    return m_Screen == QApplication::desktop()->primaryScreen();
}

void MainWindow::setFocus(Qt::FocusReason reason)
{
}

int MainWindow::getOffset(QString settingsOffset, int maxVal, int defaultVal)
{

    int offset = defaultVal > maxVal ? maxVal : defaultVal;

    if (! settingsOffset.isEmpty())  {
        if (QRegExp("^\\d+px$", Qt::CaseInsensitive).exactMatch(settingsOffset))  {
            offset = settingsOffset.left(settingsOffset.size() - 2).toInt();
            if (offset > maxVal) offset = maxVal;
        }
        else if (QRegExp("^\\d+%$", Qt::CaseInsensitive).exactMatch(settingsOffset)) {
            int offsetPct = settingsOffset.left(settingsOffset.size() -1).toInt();
            if (offsetPct > 100) offsetPct = 100;
            offset = (maxVal * offsetPct)/100;
        }
        else {
            qWarning() << tr("Could not understand ") << settingsOffset
                       << tr("- must be of form <positivenumber>px or <positivenumber>%, e.g. 35px or 25%") ;
        }
    }

    return offset;
}

void MainWindow::setBackground()
{
    QImage backgroundImage;
    QSettings greeterSettings(CONFIG_FILE, QSettings::IniFormat);

    QPalette palette;
    QRect rect = QApplication::desktop()->screenGeometry(QApplication::desktop()->screenNumber());

    QString pathToBackgroundImageDir = greeterSettings.value(BACKGROUND_IMAGE_DIR_KEY).toString();


    if(!pathToBackgroundImageDir.isNull()){

        if(pathToBackgroundImageDir[pathToBackgroundImageDir.length() - 1 ] != '/')
            pathToBackgroundImageDir += '/';


        QDir directory(pathToBackgroundImageDir);
        QStringList backgroundImageList = directory.entryList(QStringList() << "*.jpg" << "*.JPG" << "*.jpeg" << "*.JPEG"<<"*.png"<<"*.PNG",QDir::Files);

        if(!selectflag){

            qsrand (time(NULL));

            if(backgroundImageList.count() > 1)
                image_index = qrand() % (backgroundImageList.count());
            else
                image_index = 0;

            selectflag = true;
        }

        if (!backgroundImageList.isEmpty()) {
            QString imagepath = pathToBackgroundImageDir + backgroundImageList[image_index];

            backgroundImage = QImage(imagepath);
            //  qDebug() << imagepath << tr(" is set as an background image");

        }else{
            qWarning() << tr("Not able to read image at index: ") << image_index << tr(" as image");
        }


    }
    else{
        QString pathToBackgroundImage = ":/resources/bgs/bg1.jpg";
        backgroundImage = QImage(pathToBackgroundImage);


#if 0
        if (backgroundImage.isNull()) {
            qWarning() << tr("Not able to read") << pathToBackgroundImage << tr("as default image");
        }else{
            qDebug()  << pathToBackgroundImage << tr(" is set as an image");
        }
#endif
    }
    

    if (backgroundImage.isNull()) {
        palette.setColor(QPalette::Background, qRgb(255, 203, 80));
    }
    else {
        QBrush brush(backgroundImage.scaled(rect.width(), rect.height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        palette.setBrush(this->backgroundRole(), brush);
    }
    this->setPalette(palette);


    /* We are painting x root background with current greeter background */
    if(m_Screen == QApplication::desktop()->primaryScreen()){
        if(!backgroundImage.isNull()){
            QImage tmpimage = backgroundImage.scaled(rect.width(), rect.height(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            setRootBackground(tmpimage);
        }else{
            QImage tmpimage(rect.width(), rect.height(), QImage::Format_ARGB32_Premultiplied) ;
            tmpimage.fill(qRgb(255,203,80));
            setRootBackground(tmpimage);

        }
    }
}


void MainWindow::setRootBackground(QImage img){

    Display *dis  = QX11Info::display();
    int screen = m_Screen;
    Window win = RootWindow(dis, screen);

    QRect screenRect = QApplication::desktop()->screenGeometry(screen);

    int width = screenRect.width();
    int height = screenRect.height();

    unsigned int depth = (unsigned int)DefaultDepth(dis, screen);

    XFlush(dis);

    Pixmap pix = XCreatePixmap(dis,win, width, height, depth);

    char *tempimage = (char *)malloc(width * height * 4);

    int k = 0;

    for(int i = 0; i< height; i++ ){
        for(int j = 0; j< width; j++){
            *((uint*)tempimage + k) = img.pixel(j,i);
            k++;
        }
    }

    Visual *visual = DefaultVisual(dis, screen);

    XImage *image = XCreateImage(dis, visual, 24, ZPixmap, 0, tempimage, width, height, 32, 0);

    GC gc = XCreateGC(dis, pix, 0, NULL);

    XPutImage(dis, pix, gc, image, 0, 0, 0, 0, width, height);

    XSetWindowBackgroundPixmap(dis, win, pix);

    /* Prevent from x shaped cursor after greeter is closed */
    Cursor c = XcursorLibraryLoadCursor(dis, "arrow");
    XDefineCursor (dis, win, c);

    /* Clear pixmap */
    XFreePixmap(dis, pix);
    XDestroyImage(image);

    XClearWindow(dis, win);
}





void MainWindow::moveForms(int screen_number){


    QRect screenRect = QApplication::desktop()->screenGeometry(screen_number);
    setGeometry(screenRect);

    screenRect.x();

    int maxX = screenRect.width() - m_LoginForm->width();
    int maxY = screenRect.height() - m_LoginForm->height();
    int defaultX = 50*maxX/100;
    int defaultY = 30*maxY/100;
    int offsetX = getOffset(Settings().offsetX_loginform(), maxX, defaultX);
    int offsetY = getOffset(Settings().offsetY_loginform(), maxY, defaultY);

    m_LoginForm->move(offsetX , offsetY);
    m_LoginForm->show();

    maxX = screenRect.width() - m_SettingsForm->width();
    maxY = screenRect.height() - m_SettingsForm->height();
    defaultX = 100*maxX/100;
    defaultY = 80*maxY/100;
    offsetX = getOffset(Settings().offsetX_settingsform(), maxX, defaultX);
    offsetY = getOffset(Settings().offsetY_settingsform(), maxY, defaultY);


    m_SettingsForm->move(offsetX, offsetY);
    m_SettingsForm->show();

    maxX = screenRect.width() - m_PowerForm->width();
    maxY = screenRect.height() - m_PowerForm->height();
    defaultX = 50*maxX/100;
    defaultY = 80*maxY/100;
    offsetX = getOffset(Settings().offsetX_powerform(), maxX, defaultX);
    offsetY = getOffset(Settings().offsetY_powerform(), maxY, defaultY);


    m_PowerForm->move(offsetX, offsetY);
    m_PowerForm->show();


}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{

    if (event->type() == QEvent::MouseMove && QApplication::desktop()->screenCount() > 1)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint globalCursorPos = QCursor::pos();
        int mousescreen = qApp->desktop()->screenNumber(globalCursorPos);

        if(mousescreen != currentScreen){

            previousScreen = currentScreen;
            currentScreen = mousescreen;

            QRect prvScreenRect = QApplication::desktop()->screenGeometry(previousScreen);
            QRect curScreenRect = QApplication::desktop()->screenGeometry(currentScreen);

            for (int i = 0; i < QApplication::desktop()->screenCount(); i++){
                if(mainWindowsList[i]->pos().x() == prvScreenRect.x() && mainWindowsList[i]->pos().y() == prvScreenRect.y()){

                    mainWindowsList[i]->hide();
                    mainWindowsList[i]->setGeometry(curScreenRect);
                    mainWindowsList[i]->move(QPoint(curScreenRect.x(), curScreenRect.y()));
                    mainWindowsList[i]->setBackground();
                    mainWindowsList[i]->show();


                }else if(mainWindowsList[i]->pos().x() == curScreenRect.x() && mainWindowsList[i]->pos().y() == curScreenRect.y()){

                    mainWindowsList[i]->hide();
                    mainWindowsList[i]->setGeometry(prvScreenRect);
                    mainWindowsList[i]->move(QPoint(prvScreenRect.x(), prvScreenRect.y()));
                    mainWindowsList[i]->setBackground();
                    moveForms(currentScreen);
                    mainWindowsList[i]->show();

                }

            }


        }

    }
    return false;
}



void MainWindow::receiveKeyboardRequest(QPoint from, int width){

    int middlepoint = from.x() + (width / 2);
    int keyboard_width = screenKeyboard->width();
    int keyboard_height = screenKeyboard->height();

    QRect screenRect = QApplication::desktop()->screenGeometry(QApplication::desktop()->primaryScreen());


    if(screenRect.width() <= 640){

        keyboard_width = 640;
        keyboard_height = 240;

    }else if(screenRect.width() <= 800){

        keyboard_width = 800;
        keyboard_height = 300;

    }

    screenKeyboard->setGeometry(middlepoint - (keyboard_width / 2), from.y() + 150,  keyboard_width, keyboard_height);

    from.setX(middlepoint -  (keyboard_width / 2));
    from.setY( from.y() + 150);


    if(from.y() + keyboard_height > screenRect.height() ){

        from.setY( screenRect.height() - keyboard_height);
    }

    screenKeyboard->move(from);

    screenKeyboard->show();

}


void MainWindow::receiveKeyboardClose(){
    screenKeyboard->close();

    emit keyboardClosed();

}

void MainWindow::sendKeyPress(QString key){

    emit sendKeytoChilds(key);

}



void MainWindow::checkNetwork(){

}


void MainWindow::receiveNetworkStatus(bool connected){

    emit sendNetworkStatustoChilds(connected);

}


void MainWindow::keyboardInit(){


#ifdef SCREENKEYBOARD

    if(Settings().screenkeyboardenabled().compare("y") != 0){
        return;
    }

    //screen keyboard

    screenKeyboard = new Keyboard(this);
    screenKeyboard->setKeyboardLayout(m_SettingsForm->current_layout);


    screenKeyboard->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowSystemMenuHint | Qt::WindowDoesNotAcceptFocus);
    screenKeyboard->close();

    connect(screenKeyboard, &Keyboard::sendKey, this, &MainWindow::sendKeyPress);
    connect(this, &MainWindow::sendKeytoChilds, m_LoginForm,&LoginForm::keyboardEvent);

    connect(screenKeyboard, &Keyboard::sendCloseEvent, this, &MainWindow::receiveKeyboardClose);
    connect(this, &MainWindow::keyboardClosed, m_LoginForm,&LoginForm::keyboardCloseEvent);

    connect(m_LoginForm, &LoginForm::sendKeyboardRequest, this, &MainWindow::receiveKeyboardRequest);

    connect(m_LoginForm, &LoginForm::sendKeyboardCloseRequest, this, &MainWindow::receiveKeyboardClose);
    connect(m_SettingsForm, &SettingsForm::sendKeyboardLayout, screenKeyboard, &Keyboard::setKeyboardLayout);

#endif

}

