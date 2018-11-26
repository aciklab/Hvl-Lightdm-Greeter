#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H

#include <QWidget>

#include <QLightDM/Power>
#include <QLightDM/Greeter>
#include <QLightDM/SessionsModel>


#include <QProcess>
#include <QDialog>
#include <QKeyEvent>
#include <QGraphicsOpacityEffect>
#include <QMap>
#include <QMainWindow>
#include "qquickview.h"
#include <QWebEngineView>
#include <QTimer>
#include <QLabel>
#include <QShortcut>


namespace Ui
{
class SettingsForm;
}

class SettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsForm(QWidget *parent = 0);
    ~SettingsForm();

protected slots:



private slots:

    void on_NwpushButton_pressed();

    void on_NwpushButton_released();

    void leaveDropDownActivated(int index);

    void timer_finished();

    void setKeyboardLayout(int index);

protected:
    virtual void keyPressEvent(QKeyEvent *event);

private:

    Ui::SettingsForm *ui;

    void initialize();
    void addLeaveEntry(bool canDo, QString iconName, QString text, QString actionName);
    void checkNetwork();
    pid_t proc_find(const char* name);
    void getKeyboardLayouts();



    QString networkInfoString;
    QLightDM::Greeter m_Greeter;
    QLightDM::PowerInterface power;
    QLightDM::SessionsModel sessionsModel;

    QTimer *timer;
    QLabel *qlabel;
    QString *keyboardList;
    int m_Screen;
    QStringList serviceList;

#define MAX_NETWORK_CHECK_COUNT 3
    int network_check_counter;


};

#endif // SETTINGSFORM_H
