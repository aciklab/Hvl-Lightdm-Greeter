#include "networkdialog.h"
#include "ui_networkdialog.h"

#include "QString"
#include <QDebug>
#include <QFile>

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


void NetworkDialog::on_logbutton_clicked()
{
    QString  tmpstring;
    QString outstr;

    tmpstring = "";
    QFile file("/var/log/lightdm/seat0-greeter.log");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        tmpstring += line + "\n";
    }
    file.close();

    ui->NDtextEdit->setText(tmpstring);

    logButtonClicked = true;

}
