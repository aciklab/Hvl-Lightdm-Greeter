#ifndef NETWORKDIALOG_H
#define NETWORKDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class NetworkDialog;
}

class NetworkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkDialog(QWidget *parent = 0);
    ~NetworkDialog();

    void SetText(QString Text);

    void addService(QString Text);

    bool logButtonClicked;

Q_SIGNALS:
    void servicereset(void);

private slots:

    void on_logbutton_clicked();

private:
    Ui::NetworkDialog *ui;

};

#endif // NETWORKDIALOG_H
