
#include "settingsform.h"
#include "ui_settingsform.h"
#include <QWidget>
#include <QLightDM/UsersModel>
#include <QLightDM/SessionsModel>
#include <QNetworkInterface>
#include <QTimer>
#include <QToolTip>
#include <QLabel>
#include <QTextBrowser>
#include <QApplication>
#include <QThread>
#include <QMessageBox>
#include <QDir>
#include <iostream>
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


const int KeyRole = QLightDM::SessionsModel::KeyRole;
SettingsForm::SettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsForm),
    sessionsModel(QLightDM::SessionsModel::LocalSessions,this)
{
    int i;
    QString lastsession;
    QString user;

    ui->setupUi(this);

    initialize();
    winClicked = false;


    qDebug() << tr("SettingsForm is initializing");

    timer = new QTimer();

    QPixmap iconsession(2,30);

    iconsession.fill(QColor(0x1B, 0x6C, 0xBD, 0));

    for(i = 0; i< sessionsModel.rowCount(QModelIndex()); i++){

        ui->sessioncomboBox->addItem(iconsession, sessionsModel.data(sessionsModel.index(i, 0), KeyRole).toString(), sessionsModel.data(sessionsModel.index(i, 0), KeyRole).toString());

    }
    user = Cache().getLastUser();
    lastsession = Cache().getLastSession(user);

    if(!lastsession.isEmpty() && !lastsession.isNull()){
        for(i = 0; i< ui->sessioncomboBox->count(); i++){

            if (lastsession == sessionsModel.data(sessionsModel.index(i, 0), KeyRole).toString()) {
                ui->sessioncomboBox->setCurrentIndex(i);
            }

        }
    }else{
        ui->sessioncomboBox->setCurrentIndex(0);
    }

    nwDialog = new NetworkDialog();
    nwDialog->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);

    connect(nwDialog, &NetworkDialog::servicereset, this, &SettingsForm::networkCheckSlot);


    networkOK = true;
    network_check_counter = 0;
    timer->setTimerType(Qt::TimerType::CoarseTimer);
    timer->setInterval(50);
    timer->setSingleShot(false);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(timer_finished()));


    connect(this, SIGNAL(selectKeyboard(int)), this, SLOT(setKeyboardLayout(int)));

    batteryInit();

    checkNetwork();
}

SettingsForm::~SettingsForm()
{
    delete ui;
    delete timer;
    delete keyboardList;

}


void SettingsForm::initialize(){

    serviceList = Settings().getservices();
    connect(ui->kybrdcomboBox, SIGNAL(activated(int)), this, SLOT(setKeyboardLayout(int)));
    getKeyboardLayouts();

    nwButtonPressed = false;
}


int SettingsForm::CheckService(QString Service){


    QString  tmpstring;
    QString outstr;
    int read_size;

    tmpstring = "";

    QString line;
    QStringList arguments = {"status", Service};
    QProcess proc;
    proc.start("systemctl", arguments);
    if (!proc.waitForStarted()){
        qWarning() << Service + " Service check failed";
        return SERVICE_NOT_EXIST;//no need to wait
    }

    while(proc.waitForReadyRead())
    {
        line.append(proc.readAll());
    }

    if(line.length() < 1){
        qWarning() << Service + " Service check failed";
        return SERVICE_NOT_EXIST;
    }

    /*get current layout*/

    outstr = getValueOfString(line, QString("Active"));

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


    checkBattery();
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

    nwDialog->setFixedHeight((line_count + 1) * 30 + 50);
    nwDialog->setFixedWidth(this->width() + 200);
    QPoint pt_g = QWidget::mapToGlobal(this->pos());
    QPoint pt = this->pos();

    uint labelx = ((pt_g.x() - pt.x())+ (this->width() - 200)/2) - ((nwDialog->width()) / 2);

    nwDialog->logButtonClicked = false;
    nwDialog->SetText(networkInfoString);

    nwDialog->setGeometry(labelx, (pt_g.y() - pt.y()) - ((line_count + 1) * 30 + 50), 0, 0);
    nwDialog->setFixedHeight((line_count + 1) * 30 + 50);
    nwDialog->setFixedWidth(this->width() + 200);

    nwDialog->exec();
    nwDialog->clearFocus();

}


void SettingsForm::getKeyboardLayouts(){


    QString  tmpstring;
    QString outstr;

    QString cachedlayout;
    QString data;
    QByteArray read_data;

    tmpstring = "";

    QString line;
    QStringList arguments = {"-query"};
    QProcess proc;
    proc.start("setxkbmap", arguments);
    if (!proc.waitForStarted()){
        qWarning() << "Current Keyboard layout can not be read" ;

    }


    while(proc.waitForReadyRead())
        read_data.append(proc.readAll());

    // read_data = proc.readAllStandardOutput();
    data = QString(read_data);

    /*get current layout*/

    outstr = getValueOfString(data, QString("layout"));

    if(outstr != NULL)
        tmpstring = outstr;

    outstr = getValueOfString(data, QString("variant"));

    if(outstr != NULL)
        tmpstring += ' ' + outstr;

    /*get current variant*/
    QPixmap iconx(":/resources/keyboard.png");

    ui->kybrdcomboBox->addItem(iconx, "Türkçe Q", "tr");
    ui->kybrdcomboBox->addItem(iconx, "Türkçe F", "tr f");
    ui->kybrdcomboBox->addItem(iconx, "English Q", "us");

    cachedlayout = Cache().getUserKeyboard();

    if(!cachedlayout.isEmpty() && !cachedlayout.isNull())
        tmpstring = cachedlayout;

    qInfo() << "Current Keyboard layout is: " +  tmpstring;
    emit sendKeyboardLayout(tmpstring);
    current_layout = tmpstring;

    QStringList arguments_s = tmpstring.split(" ");
    proc.start("setxkbmap", arguments_s);
    proc.waitForFinished();

    Cache().setUserKeyboard(tmpstring);


    if(ui->kybrdcomboBox->findData(tmpstring) == -1){

        ui->kybrdcomboBox->addItem(iconx, tmpstring, tmpstring);
    }

    for(int i=0; i< ui->kybrdcomboBox->count(); i++){

        if(ui->kybrdcomboBox->itemData(i).toString().compare(tmpstring) == 0)
            ui->kybrdcomboBox->setCurrentIndex(i);
    }

}


void SettingsForm::setKeyboardLayout(int index){

    QProcess proc;

    QString actionName = ui->kybrdcomboBox->itemData(index).toString();

    qInfo() << "Current Keyboard layout is setting to " +  actionName + " now";

    QStringList arguments_s = actionName.split(" ");

    proc.start("setxkbmap", arguments_s);
    proc.waitForFinished();
    Cache().setUserKeyboard(actionName);

    //set onscreen keyboard layout
    emit sendKeyboardLayout(actionName);
}


QString SettingsForm::getValueOfString(QString data, QString value){


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
        winClicked = false;
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

        winClicked = false;
        clearFocus();

        ui->kybrdcomboBox->clearFocus();
        ui->NwpushButton->clearFocus();


    } else if(event->key() == Qt::Key_Meta){

        winClicked = true;


    }else if(event->key() == Qt::Key_Space){

        if(winClicked)
            keyboardSelect();
        winClicked = false;

    }else{

        winClicked = false;
        QWidget::keyPressEvent(event);
    }

}


void SettingsForm::sendNWStatus(bool nwstatus){

    emit sendNWStatusSignal(nwstatus);
}

void SettingsForm::networkCheckSlot(){
    checkNetwork();

}


void SettingsForm::updateHostName(QString hostname){

    ui->hostnamelabel->setText(hostname);
}


void SettingsForm::keyboardSelectSlot(void){
    keyboardSelect();
}


void SettingsForm::keyboardSelect(void){

    if(ui->kybrdcomboBox->currentIndex() == 0){
        ui->kybrdcomboBox->setCurrentIndex(1);
        emit selectKeyboard(1);
    }else if(ui->kybrdcomboBox->currentIndex() == 1){
        emit selectKeyboard(0);
        ui->kybrdcomboBox->setCurrentIndex(0);
    }
}


void SettingsForm::receiveCurrentUser(QString User){

    int i;
    QString sessionstr;
    ui->sessioncomboBox->setCurrentIndex(0);

    sessionstr = Cache().getLastSession(User);

    for(i = 0; i< ui->sessioncomboBox->count(); i++){

        if (sessionstr == sessionsModel.data(sessionsModel.index(i, 0), KeyRole).toString()) {
            ui->sessioncomboBox->setCurrentIndex(i);
            break;
        }

    }

    if(!sessionstr.isNull() && !sessionstr.isEmpty()){
        emit sendSessionInfo(sessionstr);
    }else{
        sessionstr = sessionsModel.data(sessionsModel.index(0, 0), KeyRole).toString();
        emit sendSessionInfo(sessionstr);
    }

}

void SettingsForm::on_sessioncomboBox_activated(int index)
{
    QString sessionname = ui->sessioncomboBox->itemData(index).toString();
    emit sendSessionInfo(sessionname);
}



void SettingsForm::batteryInit(void){

    QDir pathDir("/sys/class/power_supply/BAT0");
    if (pathDir.exists()){

        ui->Batterybutton->setToolTipDuration(10000);
        batteryExist = true;
        ui->Batterybutton->show();
    }else{
        batteryExist = false;
        ui->Batterybutton->hide();
    }
}

void SettingsForm::checkBattery(void){

    QFile file("/sys/class/power_supply/BAT0/capacity");
    int level;

    if(batteryExist){

        if(!file.open(QIODevice::ReadOnly))
        {
            return;
        }

        QTextStream instream(&file);
        QString line = instream.readLine();

        level = line.toInt();

        if(level > 90){

            QPixmap iconx(":/resources/battery_full.png");
            ui->Batterybutton->setIcon(iconx);
        }else if(level <= 15){

            QPixmap iconx(":/resources/battery_critical.png");
            ui->Batterybutton->setIcon(iconx);

        }else if (level <= 35){

            QPixmap iconx(":/resources/battery_low.png");
            ui->Batterybutton->setIcon(iconx);

        }else if(level <= 65){
            QPixmap iconx(":/resources/battery_normal.png");
            ui->Batterybutton->setIcon(iconx);


        }else if(level <= 90){

            QPixmap iconx(":/resources/battery_good.png");
            ui->Batterybutton->setIcon(iconx);

        }

        ui->Batterybutton->setToolTip("%" + line);

        file.close();

    }

}
