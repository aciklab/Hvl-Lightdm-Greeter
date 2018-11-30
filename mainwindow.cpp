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

#include "mainwindow.h"
#include "loginform.h"
#include "settings.h"
#include "settingsform.h"
#include "clockform.h"

MainWindow::MainWindow(int screen, QWidget *parent) :
    QWidget(parent),
    m_Screen(screen)
{
    setObjectName(QString("MainWindow_%1").arg(screen));

    QRect screenRect = QApplication::desktop()->screenGeometry(screen);
    setGeometry(screenRect);

    setBackground();



    // display login dialog only in the main screen
    
    if (showLoginForm()) {
        m_LoginForm = new LoginForm(this);

        int maxX = screenRect.width() - m_LoginForm->width();
        int maxY = screenRect.height() - m_LoginForm->height();
        int defaultX = 50*maxX/100;
        int defaultY = 30*maxY/100;
        int offsetX = getOffset(Settings().offsetX_loginform(), maxX, defaultX);
        int offsetY = getOffset(Settings().offsetY_loginform(), maxY, defaultY);
        
        m_LoginForm->move(offsetX, offsetY);
        m_LoginForm->show();

        //todo show users form


        m_SettingsForm = new SettingsForm(this);


        maxX = screenRect.width() - m_SettingsForm->width();
        maxY = screenRect.height() - m_SettingsForm->height();
        defaultX = 50*maxX/100;
        defaultY = 80*maxY/100;
        offsetX = getOffset(Settings().offsetX_settingsform(), maxX, defaultX);
        offsetY = getOffset(Settings().offsetY_settingsform(), maxY, defaultY);

        if(offsetY < (m_LoginForm->y() + m_LoginForm->height())){
            offsetY = m_LoginForm->y() + m_LoginForm->height() + 1;
        }


        m_SettingsForm->move(offsetX, offsetY);
        m_SettingsForm->show();


        m_ClockForm = new clockForm(this);

        maxX = screenRect.width();
        maxY = screenRect.height();

        int sizex = getOffset(Settings().sizeX_clockform(), maxX, 20*maxX/100);
        int sizey = getOffset(Settings().sizeY_clockform(), maxY, 20*maxY/100);


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


        //QObject::connect(m_SettingsForm, SIGNAL(sendNWStatusSignal(bool)), m_LoginForm, SLOT(stopWaitOperation(bool)));
        QObject::connect(m_SettingsForm, &SettingsForm::sendNWStatusSignal, m_LoginForm, &LoginForm::stopWaitOperation);




        // This hack ensures that the primary screen will have focus
        // if there are more screens (move the mouse cursor in the center
        // of primary screen - not in the center of all X area). It
        // won't affect single-screen environments.
        int centerX = screenRect.width()/2 + screenRect.x();
        int centerY = screenRect.height()/2 + screenRect.y();
        QCursor::setPos(centerX, centerY);
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
            qWarning() << tr("Could not understand") << settingsOffset
                       << tr("- must be of form <positivenumber>px or <positivenumber>%, e.g. 35px or 25%") ;
        }
    }

    return offset;
}

void MainWindow::setBackground()
{
    QImage backgroundImage;
    QSettings greeterSettings(CONFIG_FILE, QSettings::IniFormat);
    
    if (greeterSettings.contains(BACKGROUND_IMAGE_KEY)) {
        QString pathToBackgroundImage = greeterSettings.value(BACKGROUND_IMAGE_KEY).toString();
        
        backgroundImage = QImage(pathToBackgroundImage);
        qDebug() << backgroundImage << tr(" is set as an image");

        if (backgroundImage.isNull()) {
            qWarning() << tr("Not able to read") << pathToBackgroundImage << tr("as image");
        }

    }
    else{
        QString pathToBackgroundImage = ":/resources/bgs/bg1.jpg";
        backgroundImage = QImage(pathToBackgroundImage);

        qDebug() << backgroundImage << tr(" is set as an image");

        if (backgroundImage.isNull()) {
            qWarning() << tr("Not able to read") << pathToBackgroundImage << tr("as image");
        }
    }
    
    QPalette palette;
    QRect rect = QApplication::desktop()->screenGeometry(m_Screen);
    if (backgroundImage.isNull()) {
        palette.setColor(QPalette::Background, Qt::black);
    }
    else {
        QBrush brush(backgroundImage.scaled(rect.width(), rect.height()));
        palette.setBrush(this->backgroundRole(), brush);
    }
    this->setPalette(palette);
}

