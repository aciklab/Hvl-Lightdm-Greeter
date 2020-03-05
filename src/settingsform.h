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
#include <QBoxLayout>
#include "networkdialog.h"


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
    static QString current_layout;
    void updateHostName(QString hostname);



protected slots:



private slots:

    void timer_finished();

    void setKeyboardLayout(int index);

    void on_NwpushButton_clicked();



    void on_sessioncomboBox_activated(int index);

public slots:

    void networkCheckSlot(void);

    void keyboardSelectSlot(void);

    void receiveCurrentUser(QString User);

protected:
    virtual void keyPressEvent(QKeyEvent *event);
Q_SIGNALS:
     void sendNWStatusSignal(bool status);
     void sendKeyboardLayout(QString &layout);
     void  selectKeyboard(int);
     void sendSessionInfo(QString &sessionname);

private:

    Ui::SettingsForm *ui;

    void initialize();
    void checkNetwork();
    void getKeyboardLayouts();
    void sendNWStatus(bool nwstatus);
    QString getValueOfString(QString data, QString value);
    void keyboardSelect(void);

#define SERVICE_WORKING 0
#define SERVICE_NOT_WORKING -1
#define SERVICE_NOT_EXIST -2
    int CheckService(QString Service);
    void batteryInit(void);
    void checkBattery(void);



    QString networkInfoString;
    QLightDM::Greeter m_Greeter;
    QLightDM::PowerInterface power;
    QLightDM::SessionsModel sessionsModel;

    QMap<int, void (QLightDM::PowerInterface::*)()> powerSlots;

    QTimer *timer;

    QString *keyboardList;
    int m_Screen;
    QStringList serviceList;
    QStringList workingServices;
    bool nwButtonPressed;
    bool networkOK;
    NetworkDialog *nwDialog;
    bool winClicked;
    bool batteryExist;



#define MAX_NETWORK_CHECK_COUNT 3
    int network_check_counter;



};

#endif // SETTINGSFORM_H
