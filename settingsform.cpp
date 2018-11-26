
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

    qlabel = new QLabel();


    ui->formFrame->clearFocus();
    ui->kybrdcomboBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    ui->leaveComboBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    timer->setTimerType(Qt::TimerType::CoarseTimer);
    timer->setInterval(250);
    timer->setSingleShot(false);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(timer_finished()));



}

SettingsForm::~SettingsForm()
{
    delete ui;

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
    connect(ui->kybrdcomboBox, SIGNAL(activated(int)), this, SLOT(setKeyboardLayout(int)));

}

void SettingsForm::addLeaveEntry(bool canDo, QString iconName, QString text, QString actionName)
{
    if (canDo) {
        ui->leaveComboBox->addItem(QIcon::fromTheme(iconName), text, actionName);
    }
}




void SettingsForm::leaveDropDownActivated(int index)
{

    QString actionName = ui->leaveComboBox->itemData(index).toString();
    if      (actionName == "shutdown") power.shutdown();
    else if (actionName == "restart") power.restart();
    else if (actionName == "hibernate") power.hibernate();
    else if (actionName == "suspend") power.suspend();
}


pid_t SettingsForm::proc_find(const char* name){
    DIR *dir;
    struct dirent *ent;
    FILE *fd;
    char* endptr;
    char buf[512];
    long lpid;

    if (!(dir = opendir("/proc"))) {
        return -1;
    }

    while((ent = readdir(dir)) != NULL) {

        lpid = strtol(ent->d_name, &endptr, 10);
        if (*endptr != '\0') {
            continue;
        }

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "/proc/%ld/cmdline", lpid);

        fd = fopen(buf, "r");


        if (fd) {

            memset(buf, 0, sizeof(buf));

            if (fgets(buf, sizeof(buf),fd) != NULL) {

                char* first = strtok(buf, " ");
                if (!strcmp(first, name)) {
                    fclose(fd);
                    closedir(dir);
                    return (pid_t)lpid;
                }
            }
            fclose(fd);
        }

    }

    closedir(dir);
    return -1;
}


void SettingsForm::checkNetwork(){


    QString ip_string;
    QNetworkInterface netInterface;
    QNetworkInterface::InterfaceFlags flags;
    QNetworkAddressEntry address;
    network_check_counter = 0;
    uint ip_count = 0;
    pid_t pidx;
    QByteArray servicearray;
    char * serviceptr;

    ip_string += "  IP   ";



    foreach (netInterface, QNetworkInterface::allInterfaces()) {
        flags = netInterface.flags();
        if( (bool)(flags & QNetworkInterface::IsRunning) && !(bool)(flags & QNetworkInterface::IsLoopBack)){
            foreach (address, netInterface.addressEntries()) {
                if(address.ip().protocol() == QAbstractSocket::IPv4Protocol){
                    ip_string += "     " +netInterface.interfaceNameFromIndex(netInterface.index()) + ": " + address.ip().toString()+ '\n';
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
    }
    else
    {
        networkInfoString = "  IP bilgisi yok\n";
        QPixmap iconx(":/resources/ethernet-off.jpg");
        ui->NwpushButton->icon().addPixmap(iconx, QIcon::Mode::Normal, QIcon::State::On);
        ui->NwpushButton->setIcon(iconx);
        ui->NwpushButton->setCheckable(true);


    }


    for(int i= 0; i< serviceList.count();i++){

        servicearray = serviceList[i].toLatin1();
        serviceptr = servicearray.data();

        pidx = proc_find(serviceptr);

        if(pidx == -1)
            networkInfoString += serviceList[i] + " servisi çalışmıyor\n";
        else
            networkInfoString += serviceList[i] + " servisi çalışıyor\n";

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

    QVBoxLayout *layout = new QVBoxLayout;
    uint line_count = 0;

    wchar_t a[networkInfoString.length()];

    QString tmp;

    tmp = networkInfoString;
    tmp.toWCharArray(a);

    for(int i=0; i<networkInfoString.length() ; i++)
    {
        if(a[i] == '\n')
            line_count++;

    }






    qlabel->setFixedHeight((line_count + 1) * 20);
    qlabel->setFixedWidth(300);
    QPoint ptx = QWidget::mapToGlobal(this->pos());
    QPoint pty = this->pos();


    uint windowwidth = this->width();
    uint labelx = ((ptx.x() - pty.x())+ this->width()/2) - (qlabel->width() / 2);


    qlabel->setGeometry(labelx, (ptx.y() - pty.y()) - ((line_count + 1) * 20), 0, 0);
    qlabel->setFixedHeight((line_count + 1) * 20);
    qlabel->setFixedWidth(300);




    qlabel->setText(networkInfoString);
    qlabel->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    qlabel->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    qlabel->setFocus();



    qlabel->show();
}

void SettingsForm::on_NwpushButton_released(){

    int x =  qlabel->x();

    qlabel->hide();
}


void SettingsForm::getKeyboardLayouts(){


    FILE *fp;
    char path[64];
    uint readerror = 0;
    QString  tmpstring;

    tmpstring = "";

    QPixmap iconx(":/resources/keyboard.jpeg");

    fp = popen("/usr/bin/localectl list-x11-keymap-layouts", "r");
    if (fp == NULL) {
        qDebug() << "Klavye Layoutları alınamadı\n" ;
        readerror = 1;
    }

    if(readerror == 0){

        while (fgets(path, sizeof(path)-1, fp) != NULL) {

            path[strlen(path) - 1] = '\0';

            tmpstring = "";

            for(int i = 0; i< strlen(path); i++)
            {
                tmpstring += QChar( path[i]);
            }



            ui->kybrdcomboBox->addItem(iconx,tmpstring, tmpstring);
        }

    }else{

    }



    /* close */
    pclose(fp);


    //keyboardList;




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



void SettingsForm::keyPressEvent(QKeyEvent *event)
{


    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {

        if(ui->kybrdcomboBox->isActiveWindow() || ui->leaveComboBox->isActiveWindow() ){

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
