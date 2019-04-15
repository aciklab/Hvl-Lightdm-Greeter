#include "powerform.h"
#include "ui_powerform.h"

#include <QWidget>
#include <QLightDM/UsersModel>
#include <QNetworkInterface>
#include <QTimer>
#include <QToolTip>
#include <QLabel>
#include <QTextBrowser>
#include <QApplication>
#include <QThread>
#include <QMessageBox>
#include "powerdialog.h"

PowerForm::PowerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PowerForm)
{
    ui->setupUi(this);


    QT_TRANSLATE_NOOP("QPlatformTheme", "Cancel");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Apply");
    QT_TRANSLATE_NOOP("QPlatformTheme", "&Yes");
    QT_TRANSLATE_NOOP("QPlatformTheme", "&No");
    QT_TRANSLATE_NOOP("QPlatformTheme", "OK");

    if (!power.canShutdown())
        ui->toolButtonShutdown->hide();

    if (!power.canRestart())
        ui->toolButtonRestart->hide();

    if (!power.canHibernate())
        ui->toolButtonHibernate->hide();


    if (!power.canSuspend())
        ui->toolButtonSuspend->hide();

}

PowerForm::~PowerForm()
{
    delete ui;
}

void PowerForm::on_toolButtonShutdown_clicked()
{

    PowerDialog msgbox;

    QString text;

    text = tr("Go to Shutdown?");

    msgbox.setText(text);

    if (msgbox.exec()) {

        qInfo() << "System is going to  Shutdown now";
        power.shutdown();

    }



}

void PowerForm::on_toolButtonHibernate_clicked()
{

    PowerDialog msgbox;

    QString text;

    text = tr("Go to Hibernate?");

    msgbox.setText(text);

    if (msgbox.exec()) {

        qInfo() << "System is going to Hibernate now";
        power.hibernate();

    }

}

void PowerForm::on_toolButtonSuspend_clicked()
{

    PowerDialog msgbox;

    QString text;

    text = tr("Go to Suspend?");

    msgbox.setText(text);

    if (msgbox.exec()) {

        qInfo() << "System is going to suspend now";
        power.suspend();

    }

}

void PowerForm::on_toolButtonRestart_clicked()
{

    PowerDialog msgbox;

    QString text;

    text = tr("Go to Restart?");

    msgbox.setText(text);

    if (msgbox.exec()) {

        qInfo() << "System is going to restart now";
        power.restart();

    }

}
