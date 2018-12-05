
#include "settingsform.h"
#include "ui_settingsform.h"
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

#include "settings.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <QShortcut>



SettingsForm::SettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsForm)

{

    ui->setupUi(this);

    initialize();

    timer = new QTimer();

    dialog = new QDialog(0, Qt::Popup | Qt::FramelessWindowHint);
    layout = new QVBoxLayout;
    popupLabel = new QLabel();



    ui->formFrame->clearFocus();
    ui->kybrdcomboBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    ui->leaveComboBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    timer->setTimerType(Qt::TimerType::CoarseTimer);
    timer->setInterval(50);
    timer->setSingleShot(false);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(timer_finished()));


}

SettingsForm::~SettingsForm()
{
    delete ui;
    delete timer;
    delete keyboardList;
    delete dialog;

}


void SettingsForm::initialize(){

    addLeaveEntry(power.canShutdown(), "system-shutdown", tr("Shutdown"), "shutdown");
    addLeaveEntry(power.canRestart(), "system-reboot", tr("Restart"), "restart");
    addLeaveEntry(power.canHibernate(), "system-suspend-hibernate", tr("Hibernate"), "hibernate");
    addLeaveEntry(power.canSuspend(), "system-suspend", tr("Suspend"), "suspend");
    ui->leaveComboBox->setDisabled(ui->leaveComboBox->count() <= 1);

    connect(ui->leaveComboBox, SIGNAL(activated(int)), this, SLOT(leaveDropDownActivated(int)));

    serviceList = Settings().getservices();

    getKeyboardLayouts();
    ui->kybrdcomboBox->setCurrentText("tr");
    connect(ui->kybrdcomboBox, SIGNAL(activated(int)), this, SLOT(setKeyboardLayout(int)));
    nwButtonPressed = false;


}

void SettingsForm::addLeaveEntry(bool canDo, QString iconName, QString text, QString actionName)
{
    if (canDo) {
        ui->leaveComboBox->addItem(QIcon::fromTheme(iconName), text, actionName);
    }
}




void SettingsForm::leaveDropDownActivated(int index)
{

    QMessageBox::StandardButton reply;

    QString actionName = ui->leaveComboBox->itemData(index).toString();
    QString text;
    ui->leaveComboBox->setCurrentIndex(-1);

    if      (actionName == "shutdown") text = tr("Go to Shutdown?");
    else if (actionName == "restart") text = tr("Go to Restart?");
    else if (actionName == "hibernate") text = tr("Go to Hibernate?");
    else if (actionName == "suspend") text = tr("Go to Suspend?");


    reply = QMessageBox::question(this, tr("Info"), text, QMessageBox::Ok|QMessageBox::Cancel);

    if (reply == QMessageBox::Ok) {

        if      (actionName == "shutdown") power.shutdown();
        else if (actionName == "restart") power.restart();
        else if (actionName == "hibernate") power.hibernate();
        else if (actionName == "suspend") power.suspend();

    }
}

int SettingsForm::CheckService(QString Service){

    FILE *fp;
    char data[220];

    bool readerror = false;
    QString  tmpstring;
    QString outstr;

    tmpstring = "";

    QString command = "systemctl status " + Service;
    QByteArray commnadb = command.toLocal8Bit();

    fp = popen(commnadb.data(), "r");
    if (fp == NULL) {
        qDebug() << Service + tr(" Service check failed");
        return SERVICE_NOT_EXIST;//no need to wait
    }


    if(fread(data, sizeof(data), 1, fp) < 1){
        qDebug() << Service + tr(" Service check failed");
        return SERVICE_NOT_EXIST;
    }

    /* close */
    pclose(fp);

    /*get current layout*/

    outstr = getValueOfString(QString::fromLocal8Bit(data), QString("Active"));

    if(outstr == NULL)
        return SERVICE_NOT_EXIST;

    if(outstr.compare("active") == 0)
        return SERVICE_WORKING;
    else
        return SERVICE_NOT_WORKING;

}

void SettingsForm::checkNetwork(){


    QString ip_string;
    QNetworkInterface netInterface;
    QNetworkInterface::InterfaceFlags flags;
    QNetworkAddressEntry address;
    network_check_counter = 0;
    uint ip_count = 0;
    int res;



    int runningServices = 0;

    ip_string += "  IP:\n";

    networkInfoString.clear();

    foreach (netInterface, QNetworkInterface::allInterfaces()) {
        flags = netInterface.flags();
        if( (bool)(flags & QNetworkInterface::IsRunning) && !(bool)(flags & QNetworkInterface::IsLoopBack)){
            foreach (address, netInterface.addressEntries()) {
                if(address.ip().protocol() == QAbstractSocket::IPv4Protocol){
                    ip_string += "  " +netInterface.interfaceNameFromIndex(netInterface.index()) + ": " + address.ip().toString()+ '\n';
                    ip_count++;
                }
            }
        }
    }


    if(ip_count != 0){
        networkInfoString = ip_string + '\n';
        //todo:network icon on
        QPixmap iconx(":/resources/ethernet-on.jpg");
        ui->NwpushButton->icon().addPixmap(iconx, QIcon::Mode::Normal, QIcon::State::On);
        ui->NwpushButton->setIcon(iconx);
        ui->NwpushButton->setCheckable(true);

    }else{
        networkInfoString = tr("  No IP information\n");
        QPixmap iconx(":/resources/ethernet-off.jpg");
        ui->NwpushButton->icon().addPixmap(iconx, QIcon::Mode::Normal, QIcon::State::On);
        ui->NwpushButton->setIcon(iconx);
        ui->NwpushButton->setCheckable(true);
    }


    for(int i = 0; i< serviceList.count();i++){

        networkInfoString += "  ";

        res = CheckService(serviceList[i]);

        if(res == SERVICE_NOT_WORKING){
            networkInfoString += (serviceList[i] + tr(" service is not working\n"));
        }else if(res == SERVICE_WORKING){
            networkInfoString += (serviceList[i] + tr(" service is working\n"));
            runningServices++;
        }else{
            runningServices++;
        }

    }

    if(ip_count != 0 && serviceList.count() == runningServices){
        sendNWStatus(true);
    }
    else{
        sendNWStatus(false);
    }


}


void SettingsForm::timer_finished(){

    network_check_counter++;

    checkNetwork();

    timer->stop();
    timer->setInterval(5000);
    timer->start();

}



void SettingsForm::on_NwpushButton_pressed()
{


    uint line_count = 0;


    for(int i=0; i<networkInfoString.length() ; i++)
    {
        if(networkInfoString[i] == '\n')
            line_count++;

    }

    dialog->setFixedHeight((line_count + 1) * 20);
    dialog->setFixedWidth(300);
    QPoint pt_g = QWidget::mapToGlobal(this->pos());
    QPoint pt = this->pos();

    uint labelx = ((pt_g.x() - pt.x())+ this->width()/2) - (dialog->width() / 2);

    layout->addWidget(popupLabel);
    popupLabel->setText(networkInfoString);
    dialog->setLayout(layout);
    dialog->setGeometry(labelx, (pt_g.y() - pt.y()) - ((line_count + 1) * 20), 0, 0);
    dialog->setFixedHeight((line_count + 1) * 20);
    dialog->setFixedWidth(300);


    dialog->exec();



}


void SettingsForm::getKeyboardLayouts(){


    FILE *fp;
    char data[128];
    bool readerror = false;
    QString  tmpstring;
    QString outstr;

    tmpstring = "";

    fp = popen("setxkbmap -query", "r");
    if (fp == NULL) {
        qDebug() << tr("Current Keyboard layout can not be read\n") ;
        readerror = true;
    }

    if(readerror == false){

        if(fread(data, sizeof(data), 1, fp) < 1){
            //qDebug() << tr("Current Keyboard layout can not be read\n") ;
        }

    }

    /* close */
    pclose(fp);

    /*get current layout*/

    outstr = getValueOfString(QString::fromLocal8Bit(data), QString("layout"));

    if(outstr != NULL)
        tmpstring = outstr;

    outstr = getValueOfString(QString::fromLocal8Bit(data), QString("variant"));

    if(outstr != NULL)
        tmpstring += ' ' + outstr;

    /*get current variant*/
    QPixmap iconx(":/resources/keyboard.jpeg");

    ui->kybrdcomboBox->addItem(iconx, "Türkçe Q", "tr");
    ui->kybrdcomboBox->addItem(iconx, "Türkçe F", "tr f");
    ui->kybrdcomboBox->addItem(iconx, "English Q", "us");


    for(int i=0; i< ui->kybrdcomboBox->count(); i++){

        if(ui->kybrdcomboBox->itemData(i).toString().compare(tmpstring) == 0)
            ui->kybrdcomboBox->setCurrentIndex(i);
    }

}


void SettingsForm::setKeyboardLayout(int index){

    char *setcommand;
    char cmd_array[256];
    memset(cmd_array,0,sizeof(cmd_array));

    QString actionName = ui->kybrdcomboBox->itemData(index).toString();

    QByteArray ba;
    ba = actionName.toLatin1();

    setcommand = ba.data();

    sprintf(cmd_array, "/usr/bin/setxkbmap %s",setcommand);

    system(cmd_array);
}


QString SettingsForm::getValueOfString(QString data, QString value){

    //qPrintable();
    QString result;

    int indx = data.indexOf(value);
    int endx = 0;

    if(indx == -1){
        return NULL;
    }

    indx += value.size();
    result = data.mid(indx);

    if(result.at(0) != QChar(':') && result.at(0) != QChar('=') ){

        if(result.at(1) != QChar(':') && result.at(1) != QChar('=') )
            return NULL;
        else
            indx = 2;

    }else{
        indx = 1;
    }


    while(result.at(indx) == ' ' || result.at(indx) == '\n'){
        indx++;
    }

    endx = indx;

    while(result.at(endx) != ' ' && result.at(endx) != '\n'){
        endx++;
    }


    result = result.mid(indx, (endx - indx));

    return result;
}


void SettingsForm::keyPressEvent(QKeyEvent *event)
{

    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {

        if(ui->kybrdcomboBox->isActiveWindow() || ui->leaveComboBox->isActiveWindow()){

        }
        else
        {
            QWidget::keyPressEvent(event);
        }
    }
    else
    {

        QWidget::keyPressEvent(event);
    }

}


void SettingsForm::sendNWStatus(bool nwstatus){

    emit sendNWStatusSignal(nwstatus);
}
