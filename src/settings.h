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
    void setUserKeyboard(QString keyboard) { setValue("keyboard", keyboard); }
    QString getUserKeyboard() { return value("keyboard").toString(); }
};

#define CONFIG_FILE "/usr/share/lightdm/lightdm-hvl-greeter.conf.d/hvl-lightdm-greeter.conf"

#define BACKGROUND_IMAGE_DIR_KEY "greeter-background-image-dir"
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
    Settings() : QSettings(QString("/usr/share/lightdm/lightdm-hvl-greeter.conf.d/hvl-lightdm-greeter.conf"), QSettings::NativeFormat) {}
    QString iconThemeName_loginform() { return value("greeter-icon-theme").toString(); }
    QStringList backgroundImagePath() { return value("greeter-background-image").toStringList(); }
    QString offsetX_loginform() { return value("loginform-offset-x").toString(); }
    QString offsetY_loginform() { return value("loginform-offset-y").toString(); }
    QString offsetX_settingsform() { return value("settingsform-offset-x").toString(); }
    QString offsetY_settingsform() { return value("settingsform-offset-y").toString(); }
    QString offsetX_clockform() { return value("clockform-offset-x").toString(); }
    QString offsetY_clockform() { return value("clockform-offset-y").toString(); }
    QString sizeX_clockform() { return value("clockform-size-x").toString(); }
    QString sizeY_clockform() { return value("clockform-size-y").toString(); }
    QString offsetX_powerform() { return value("powerform-offset-x").toString(); }
    QString offsetY_powerform() { return value("powerform-offset-y").toString(); }
    QStringList getservices() { return value("services-to-check").toStringList(); }
    QString logopath() { return value("greeter-loginform-logo-image").toString(); }
    int waittimeout() { return value("wait-timeout").toInt(); }
    int  network_ok_timeout() { return value("network-ok-timeout").toInt(); }
    int cachedusercount(){return value("cached-user-count").toInt();}
    QString screenkeyboardenabled(){return value("screen_keyboard").toString(); }
    QString passwordresetenabled(){return value("password-reset-button").toString();}
    QString passwordresetwebpageurl(){return value("password-web-reset-webpage").toString();}
    int screenSaver_timeout() { return value("screen-saver-timeout").toInt(); }
    int slideShow_timeout() { return value("slide-show-timeout").toInt(); }
};




#endif // SETTINGS_H
