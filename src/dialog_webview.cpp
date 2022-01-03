#include "dialog_webview.h"
#include "ui_dialog_webview.h"
#include <QtWebEngine/QtWebEngine>
#include <QtWebEngineCore/QtWebEngineCore>
#include <QtWebEngineWidgets/QtWebEngineWidgets>
#include <QQmlApplicationEngine>



#include "mainwindow.h"
#include "loginform.h"
#include "settings.h"

Dialog_webview::Dialog_webview(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_webview)
{

    ui->setupUi(this);

    QtWebEngine::initialize();

    setFixedHeight(700 + ui->closebutton->width() + 1);
    setFixedWidth(1024);


    ui->widget1->setFixedHeight(700);
    ui->widget1->setFixedWidth(1024);
    ui->widget1->move(0, ui->closebutton->width());
    ui->widget1->setGeometry(0,0,1024,700);//position on screen
    ui->closebutton->move(1024 - ui->closebutton->width() - 1, 0);
    QString tmpurl = checkPasswordResetWebPage();

    if(!tmpurl.isEmpty() && !tmpurl.isNull()){
        //ui->widget1->load(QUrl(tmpurl,QUrl::ParsingMode::TolerantMode));
       ui->widget1->setUrl(QUrl(tmpurl,QUrl::ParsingMode::TolerantMode));
        //ui->widget1->setFocus();
        qInfo() <<  "Opening webpage " +  tmpurl + " now";
    }else{
        qCritical() << "Failed to opening webpage " +  tmpurl;
    }
}

Dialog_webview::~Dialog_webview()
{
    delete ui;

}

QString Dialog_webview::checkPasswordResetWebPage(){

    QString ret_string;

    QSettings greeterSettings(CONFIG_FILE, QSettings::IniFormat);

    if (greeterSettings.contains(PASSWORD_RESET_WEBPAGE_KEY)) {
        ret_string = greeterSettings.value(PASSWORD_RESET_WEBPAGE_KEY).toString();

    }


    return ret_string;

}


void Dialog_webview::Initialize(){

}


void Dialog_webview::on_closebutton_clicked()
{
    close();

}
