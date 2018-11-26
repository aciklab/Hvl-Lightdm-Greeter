#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>




class Cache : public QSettings
{
public:
    static const QString GREETER_DATA_DIR_PATH;
    static void prepare();

    Cache() : QSettings(GREETER_DATA_DIR_PATH + "/state", QSettings::NativeFormat) {}
    QString getLastUser() { return value("last-user").toString(); }
    QString getLastUserfromIndex(uint i) { return value(QStringLiteral("last-user_index%1").arg(i) ).toString(); }
    void setLastUsertoIndex(QString userId, uint i) { setValue(QStringLiteral("last-user_index%1").arg(i), userId); }
    void setLastUser(QString userId) { setValue("last-user", userId); }
    QString getLastSession(QString userId) { return value(userId + "/last-session").toString(); }
    void setLastSession(QString userId, QString session) { setValue(userId + "/last-session", session); }
};

#define CONFIG_FILE "/etc/lightdm/qt-lightdm-greeter.conf"

#define BACKGROUND_IMAGE_KEY "greeter-background-image"
#define LOGINFORM_OFFSETX_KEY "loginform-offset-x"
#define LOGINFORM_OFFSETY_KEY "loginform-offset-y"
#define PASSWORD_WEB_RESET_KEY "password-web-reset"
#define PASSWORD_RESET_WEBPAGE_KEY "password-web-reset-webpage"
#define IMAGEBOX_IMAGE "greeter-loginform-logo-image"


#define LOGFILE_PATH_KEY "logfile-path"

class Settings : public QSettings
{
public:
    Settings() : QSettings(QString("/etc/lightdm/qt-lightdm-greeter.conf"), QSettings::NativeFormat) {}
    QString iconThemeName_loginform() { return value("greeter-icon-theme").toString(); }
    QString backgrundImagePath() { return value("greeter-background-image").toString(); }
    QString offsetX_loginform() { return value("loginform-offset-x").toString(); }
    QString offsetY_loginform() { return value("loginform-offset-y").toString(); }
    QString offsetX_settingsform() { return value("settingsform-offset-x").toString(); }
    QString offsetY_settingsform() { return value("settingsform-offset-y").toString(); }
    QString offsetX_clockform() { return value("clockform-offset-x").toString(); }
    QString offsetY_clockform() { return value("clockform-offset-y").toString(); }
    QString sizeX_clockform() { return value("clockform-size-x").toString(); }
    QString sizeY_clockform() { return value("clockform-size-y").toString(); }
    QStringList getservices() { return value("services-to-check").toStringList(); }
    QString logopath() { return value("greeter-loginform-logo-image").toString(); }
};




#endif // SETTINGS_H
