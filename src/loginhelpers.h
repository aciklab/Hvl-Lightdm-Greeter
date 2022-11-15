#ifndef LOGINHELPERS_H
#define LOGINHELPERS_H

#include <QString>
#include <QObject>
class LoginHelpers : public QObject
{
     Q_OBJECT
public:


    LoginHelpers();

    static QString fixUserName(QString &username);
    static QString getShortUsername(QString &username);
    static bool ifLocalUser(QString username);
    static QString getUserRealm(QString username);
    static QString readRealm();
    static QString getValueOfString(QString data, QString value);
    static QString translateResetPwdMessage(QString message);
    static QString realM;
private:
};


#endif // LOGINHELPERS_H
