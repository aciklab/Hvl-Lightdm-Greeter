#include "networkdialog.h"
#include "ui_networkdialog.h"

#include "QString"
#include <QDebug>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

NetworkDialog::NetworkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetworkDialog)
{
    ui->setupUi(this);

    logButtonClicked = false;


}

NetworkDialog::~NetworkDialog()
{
    delete ui;
}

void NetworkDialog::SetText(QString Text){

    if(!logButtonClicked)
        ui->NDtextEdit->setText(Text);

}

void NetworkDialog::addService(QString Text){


    ui->servicecomboBox->addItem(Text, Text);
}

static bool hidden = false;

void NetworkDialog::on_restartservicebutton_clicked()
{
    char *setcommand;
    char cmd_array[256];
    memset(cmd_array,0,sizeof(cmd_array));

    QString actionName = ui->servicecomboBox->itemData(ui->servicecomboBox->currentIndex()).toString();

    if(actionName == NULL)
        return;


    QByteArray ba;
    ba = actionName.toLatin1();
    setcommand = ba.data();

    close();
    hidden  =true;

    sprintf(cmd_array, "systemctl restart %s &",setcommand);
    system(cmd_array);

    emit servicereset();

    logButtonClicked = false;

}

void NetworkDialog::on_logbutton_clicked()
{
    FILE *fp;


    QString  tmpstring;
    QString outstr;

    tmpstring = "";


    fp = fopen("/var/log/lightdm/seat0-greeter.log", "r");
    if (fp == NULL) {
        qWarning() << tr("Greeter log read error");
        return;
    }


    fseek(fp, 0L, SEEK_END);
    int sz = ftell(fp);

    char data[sz - 1];

    rewind(fp);

    if(fread(data, sizeof(data), 1, fp) < 1){
        //qWarning() << tr("Greeter log read error");
        //pclose(fp);
        // return;
    }

    /* close */
    pclose(fp);


    for(int i = sz - 1; i> 0; i--){

        if(data[i] != '\"'){
            data[i] = 0;
        }else{
            break;
        }
    }

    tmpstring = QString::fromLocal8Bit(data);

    ui->NDtextEdit->setText(tmpstring);

    logButtonClicked = true;

}
