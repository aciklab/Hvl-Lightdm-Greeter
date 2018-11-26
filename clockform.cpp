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
}

clockForm::~clockForm()
{
    delete ui;
}

void clockForm::updateClock(){

    timer->stop();

    QDateTime *qdt = new QDateTime();

    QString tempstring;
    QString fontString;

    QFont font("DejaVu Sans Mono");
    font.setPointSize(32);
    font.setWeight( QFont::Bold );

    tempstring = qdt->currentDateTime().toString("hh:mm");
    ui->clockLabel->setText(tempstring);
    font.setPointSize(ui->clockLabel->height()/2);
    ui->clockLabel->setFont(font);

    tempstring = qdt->currentDateTime().toString("dddd");
    ui->dayLabel->setText(tempstring);
    font.setPointSize(ui->dayLabel->height()/3);
    ui->dayLabel->setFont(font);

    tempstring = qdt->currentDateTime().toString("dd.MM.yyyy");
    ui->dateLabel->setText(tempstring);
    font.setPointSize(ui->dateLabel->height()/2);
    ui->dateLabel->setFont(font);

    timer->setInterval(1000);
    timer->start();

}

void clockForm::initialize(){



}

