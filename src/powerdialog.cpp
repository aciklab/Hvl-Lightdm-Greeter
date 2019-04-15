#include "powerdialog.h"
#include "ui_powerdialog.h"

#include <QPushButton>

PowerDialog::PowerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PowerDialog)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
}

PowerDialog::~PowerDialog()
{
    delete ui;
}


void PowerDialog::setText(QString text){

    ui->textEdit->setText(text);
}
