/*
* Copyright (c) 2012-2015 Christian Surlykke, Petr Vanek
*
* This file is part of qt-lightdm-greeter 
* It is distributed under the LGPL 2.1 or later license.
* Please refer to the LICENSE file for a copy of the license.
*/
#include <QRect>
#include <QApplication>
#include <QDesktopWidget>
#include <QPalette>
#include <QString>
#include <QDebug>
#include <QtNetwork/QtNetwork>
#include <QMessageBox>
#include <QBitmap>
#include <QPainter>

#include "mainwindow.h"
#include "loginform.h"
#include "settings.h"
#include "settingsform.h"
#include "clockform.h"

#include "stdlib.h"

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <QtX11Extras/QX11Info>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>



bool MainWindow::selectflag = false;
int MainWindow::image_index = 0;

MainWindow::MainWindow(int screen, QWidget *parent) :
    QWidget(parent),
    m_Screen(screen)
{
    setObjectName(QString("MainWindow_%1").arg(screen));

    QRect screenRect = QApplication::desktop()->screenGeometry(screen);
    setGeometry(screenRect);

    setBackground();

    // display login dialog only in the main screen
    
    if (m_Screen == QApplication::desktop()->primaryScreen()) {


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
        defaultX = 50*maxX/100;
        defaultY = 80*maxY/100;
        offsetX = getOffset(Settings().offsetX_settingsform(), maxX, defaultX);
        offsetY = getOffset(Settings().offsetY_settingsform(), maxY, defaultY);


        m_SettingsForm->move(offsetX, offsetY);
        m_SettingsForm->show();


        m_ClockForm = new clockForm(this);

        maxX = screenRect.width();
        maxY = screenRect.height();

        int sizex = getOffset(Settings().sizeX_clockform(), maxX, (20 * maxX) / 100);
        int sizey = getOffset(Settings().sizeY_clockform(), maxY, (20 * maxY) / 100);


        if(sizex !=0 && sizey != 0)
        {
            m_ClockForm->setFixedHeight(sizey);
            m_ClockForm->setFixedWidth(sizex);
        }

        maxX = screenRect.width() - m_ClockForm->width();
        maxY = screenRect.height() - m_ClockForm->height();
        defaultX = 5*maxX/100;
        defaultY = 5*maxY/100;
        offsetX = getOffset(Settings().offsetX_clockform(), maxX, defaultX);
        offsetY = getOffset(Settings().offsetY_clockform(), maxY, defaultY);

        m_ClockForm->move(offsetX, offsetY);
        m_ClockForm->show();


        QObject::connect(m_SettingsForm, &SettingsForm::sendNWStatusSignal, m_LoginForm, &LoginForm::stopWaitOperation);


        // This hack ensures that the primary screen will have focus
        // if there are more screens (move the mouse cursor in the center
        // of primary screen - not in the center of all X area). It
        // won't affect single-screen environments.
        int centerX = screenRect.width()/2 + screenRect.x();
        int centerY = screenRect.height()/2 + screenRect.y();
        QCursor::setPos(centerX, centerY);
        this->cursor().setShape(Qt::ArrowCursor);



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
    if (m_LoginForm) {
        m_LoginForm->setFocus(reason);
    }
    else  {
        QWidget::setFocus(reason);
    }
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
    QRect rect = QApplication::desktop()->screenGeometry(m_Screen);

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

            backgroundImage = QImage(imagepath, "jpg");
            qDebug() << imagepath << tr(" is set as an background image");

        }else{
            qWarning() << tr("Not able to read image at index: ") << image_index << tr(" as image");
        }


    }
    else{
        QString pathToBackgroundImage = ":/resources/bgs/bg1.jpg";
        backgroundImage = QImage(pathToBackgroundImage);

        qDebug()  << pathToBackgroundImage << tr(" is set as an image");

        if (backgroundImage.isNull()) {
            qWarning() << tr("Not able to read") << pathToBackgroundImage << tr("as default image");
        }
    }
    

    if (backgroundImage.isNull()) {
        palette.setColor(QPalette::Background, qRgb(255,203,80));
    }
    else {
        QBrush brush(backgroundImage.scaled(rect.width(), rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        palette.setBrush(this->backgroundRole(), brush);
    }
    this->setPalette(palette);


    /* We are painting x root background with current greeter background */
    if(m_Screen == QApplication::desktop()->primaryScreen()){
        if(!backgroundImage.isNull()){
            QImage tmpimage = backgroundImage.scaled(rect.width(), rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
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

    QRect screenRect = QApplication::desktop()->screenGeometry(m_Screen);

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

    /* Prevent from x shaped cursor after greeter is Closed */
    Cursor c = XcursorLibraryLoadCursor(dis, "arrow");
    XDefineCursor (dis, win, c);

    XFreePixmap(dis, pix);
    XDestroyImage(image);

    XClearWindow(dis, win);
}




