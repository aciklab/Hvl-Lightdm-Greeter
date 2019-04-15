#ifndef POWERDIALOG_H
#define POWERDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class PowerDialog;
}

class PowerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PowerDialog(QWidget *parent = 0);
    ~PowerDialog();
    void setText(QString text);

private:
    Ui::PowerDialog *ui;
};

#endif // POWERDIALOG_H
