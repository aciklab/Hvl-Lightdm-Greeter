
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
#include "networkdialog.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <QShortcut>

QString SettingsForm::current_layout = NULL;

SettingsForm::SettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsForm)
{

    ui->setupUi(this);

    initialize();

    qDebug() << tr("SettingsForm is initializing");

    timer = new QTimer();

    nwDialog = new NetworkDialog();
    nwDialog->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);

    for (int i = 0; i< serviceList.count();i++){
        nwDialog->addService(serviceList[i]);
    }

    connect(nwDialog, &NetworkDialog::servicereset, this, &SettingsForm::networkCheckSlot);


    networkOK = true;
    network_check_counter = 0;

    //ui->formFrame->clearFocus();
    //ui->kybrdcomboBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    //ui->leaveComboBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    //ui->NwpushButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);

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

}


void SettingsForm::initialize(){


    serviceList = Settings().getservices();

    getKeyboardLayouts();
    ui->kybrdcomboBox->setCurrentText("tr");
    connect(ui->kybrdcomboBox, SIGNAL(activated(int)), this, SLOT(setKeyboardLayout(int)));
    nwButtonPressed = false;
}


int SettingsForm::CheckService(QString Service){

    FILE *fp;
    char data[220];

    QString  tmpstring;
    QString outstr;
    int read_size;

    tmpstring = "";

    QString command = "systemctl status " + Service;
    QByteArray commandba = command.toLocal8Bit();

    fp = popen(commandba.data(), "r");
    if (fp == NULL) {
        qWarning() << Service + " Service check failed";
        return SERVICE_NOT_EXIST;//no need to wait
    }


    read_size = fread(data, 1, sizeof(data), fp);
    pclose(fp);

    if(read_size < 1){
        qWarning() << Service + " Service check failed";
        return SERVICE_NOT_EXIST;
    }

    /* close */


    /*get current layout*/

    outstr = getValueOfString(QString::fromLocal8Bit(data, read_size), QString("Active"));

    if(outstr == NULL){

        if(workingServices.contains(Service) || network_check_counter == 0){
            workingServices.removeAll(Service);
            qWarning() << Service + " Service is not exixst";

        }


        return SERVICE_NOT_EXIST;
    }

    if(outstr.compare("active") == 0){

        if(!workingServices.contains(Service) || network_check_counter == 0){
            workingServices.append(Service);
            qInfo() << Service << tr(" Service is working");

        }
        return SERVICE_WORKING;
    }else{

        if(workingServices.contains(Service) || network_check_counter == 0){
            workingServices.removeAll(Service);
            qWarning() << Service << tr(" Service is not working");

        }

        return SERVICE_NOT_WORKING;

    }
}
void SettingsForm::checkNetwork(){


    QString ip_string;
    QNetworkInterface netInterface;
    QNetworkInterface::InterfaceFlags flags;
    QNetworkAddressEntry address;

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

        if(!networkOK)
            qWarning() << "IP received";


        networkOK = true;

        networkInfoString = ip_string + '\n';
        QPixmap iconx(":/resources/ethernet-on.png");
        ui->NwpushButton->icon().addPixmap(iconx, QIcon::Mode::Normal, QIcon::State::On);
        ui->NwpushButton->setIcon(iconx);
        ui->NwpushButton->setCheckable(true);

    }else{


        if(networkOK)
            qWarning() << " No IP information from system";


        networkOK = false;

        networkInfoString = tr("  No IP information\n");
        QPixmap iconx(":/resources/ethernet-off.png");
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


    nwDialog->SetText(networkInfoString);

    network_check_counter++;

    ui->textEdit->setText(ip_string);

}


void SettingsForm::timer_finished(){


    checkNetwork();

    timer->stop();
    timer->setInterval(5000);
    timer->start();

}



void SettingsForm::on_NwpushButton_clicked()
{

    uint line_count = 0;

    for(int i=0; i<networkInfoString.length() ; i++)
    {
        if(networkInfoString[i] == '\n')
            line_count++;

    }

    nwDialog->setFixedHeight((line_count + 1) * 20);
    nwDialog->setFixedWidth(this->width());
    QPoint pt_g = QWidget::mapToGlobal(this->pos());
    QPoint pt = this->pos();

    uint labelx = ((pt_g.x() - pt.x())+ this->width()/2) - (nwDialog->width() / 2);

    nwDialog->logButtonClicked = false;
    nwDialog->SetText(networkInfoString);

    nwDialog->setGeometry(labelx, (pt_g.y() - pt.y()) - ((line_count + 1) * 20), 0, 0);
    nwDialog->setFixedHeight((line_count + 1) * 20);
    nwDialog->setFixedWidth(this->width());

    nwDialog->exec();
    nwDialog->clearFocus();

}


void SettingsForm::getKeyboardLayouts(){

    FILE *fp;
    char data[128];
    bool readerror = false;
    QString  tmpstring;
    QString outstr;
    int read_size;

    tmpstring = "";

    fp = popen("setxkbmap -query", "r");
    if (fp == NULL) {
        qWarning() << "Current Keyboard layout can not be read" ;
        readerror = true;
    }

    if(readerror == false){

        read_size = fread(data, 1, sizeof(data), fp);

        if( read_size < 1){
            qDebug() << tr("Current Keyboard layout can not be read\n") ;
        }

    }

    /* close */
    pclose(fp);

    /*get current layout*/

    outstr = getValueOfString(QString::fromLocal8Bit(data, read_size), QString("layout"));

    if(outstr != NULL)
        tmpstring = outstr;

    outstr = getValueOfString(QString::fromLocal8Bit(data, read_size), QString("variant"));

    if(outstr != NULL)
        tmpstring += ' ' + outstr;

    /*get current variant*/
    QPixmap iconx(":/resources/keyboard.png");

    ui->kybrdcomboBox->addItem(iconx, "Türkçe Q", "tr");
    ui->kybrdcomboBox->addItem(iconx, "Türkçe F", "tr f");
    ui->kybrdcomboBox->addItem(iconx, "English Q", "us");


    qInfo() << "Current Keyboard layout is: " +  tmpstring;
    emit sendKeyboardLayout(tmpstring);
    current_layout = tmpstring;


    if(ui->kybrdcomboBox->findData(tmpstring) == -1){

        ui->kybrdcomboBox->addItem(iconx, tmpstring, tmpstring);
    }

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

    qInfo() << "Current Keyboard layout is setting to " +  actionName + " now";

    QByteArray ba;
    ba = actionName.toLatin1();

    setcommand = ba.data();

    sprintf(cmd_array, "/usr/bin/setxkbmap %s &",setcommand);

    system(cmd_array);

    //set onscreen keyboard layout
    emit sendKeyboardLayout(actionName);
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

        if(ui->kybrdcomboBox->isActiveWindow()){

        }
        else
        {
            clearFocus();

            ui->kybrdcomboBox->clearFocus();
            ui->NwpushButton->clearFocus();
            QWidget::keyPressEvent(event);
        }
    }else if(event->key() == Qt::Key_Escape){

        clearFocus();

        ui->kybrdcomboBox->clearFocus();
        ui->NwpushButton->clearFocus();


    }else{


        QWidget::keyPressEvent(event);
    }

}


void SettingsForm::sendNWStatus(bool nwstatus){

    emit sendNWStatusSignal(nwstatus);
}

void SettingsForm::networkCheckSlot(){
    checkNetwork();
}
