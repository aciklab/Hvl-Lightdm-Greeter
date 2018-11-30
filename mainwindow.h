/*
* Copyright (c) 2012-2015 Christian Surlykke, Petr Vanek
*
* This file is part of qt-lightdm-greeter 
* It is distributed under the LGPL 2.1 or later license.
* Please refer to the LICENSE file for a copy of the license.
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

#include <QLightDM/Greeter>

#include "loginform.h"
#include "settingsform.h"
#include "clockform.h"

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

    LoginForm* loginForm() { return m_LoginForm;}

    //SettingsForm* settingsForm() { return m_SettingsForm;}


private:
    int getOffset(QString offset, int maxVal, int defaultVal);
    void setBackground();

    int m_Screen;
    LoginForm* m_LoginForm;
    clockForm *m_ClockForm;
    SettingsForm *m_SettingsForm;

};

#endif // MAINWINDOW_H
