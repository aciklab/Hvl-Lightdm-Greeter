#include "clockform.h"
#include "ui_clockform.h"
#include <QLightDM/UsersModel>
#include <QNetworkInterface>
#include <QTimer>
#include <QToolTip>
#include <QLabel>
#include <QTextBrowser>
#include <QDateTime>

clockForm::clockForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::clockForm)
{
    ui->setupUi(this);

    timer = new QTimer();
    timer->setTimerType(Qt::TimerType::CoarseTimer);

    timer->setSingleShot(false);
    updateClock();
    timer->setInterval(10);
    timer->start();

    connect(timer, SIGNAL(timeout()), this, SLOT(updateClock()));
    resized = false;
}

clockForm::~clockForm()
{
    delete ui;
    delete timer;
}

void clockForm::updateClock(){

    QString tempstring;
    timer->stop();
    QDateTime *qdt = new QDateTime();
    tempstring = qdt->currentDateTime().toString("hh:mm");
    ui->clockLabel->setText(tempstring);
    tempstring = qdt->currentDateTime().toString("dddd");
    ui->dayLabel->setText(tempstring.toUpper());
    tempstring = qdt->currentDateTime().toString(Qt::SystemLocaleShortDate);
    delete(qdt);
    ui->dateLabel->setText(tempstring.section(' ', 0,0));

    timer->setInterval(1000);
    timer->start();

}

