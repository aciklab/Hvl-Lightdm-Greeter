#ifndef POWERFORM_H
#define POWERFORM_H

#include <QWidget>

#include <QLightDM/Power>
#include <QLightDM/Greeter>
#include <QLightDM/SessionsModel>

namespace Ui {
class PowerForm;
}

class PowerForm : public QWidget
{
    Q_OBJECT

public:
    explicit PowerForm(QWidget *parent = 0);
    ~PowerForm();

private slots:
    void on_toolButtonShutdown_clicked();

    void on_toolButtonHibernate_clicked();

    void on_toolButtonSuspend_clicked();

    void on_toolButtonRestart_clicked();

private:
    Ui::PowerForm *ui;
     QLightDM::PowerInterface power;

};

#endif // POWERFORM_H
