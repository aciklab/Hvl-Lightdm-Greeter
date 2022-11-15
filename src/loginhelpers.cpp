#include "loginhelpers.h"
#include <QString>
#include <QDebug>
#include <QCoreApplication>

QString LoginHelpers::realM = QString("");

LoginHelpers::LoginHelpers()
{

}


QString LoginHelpers::fixUserName(QString &username){

    QString newname, tmpname;
    int offset;
    QString realm = getUserRealm(username);

    offset = username.indexOf(realm.toLower());

    if(offset < 0)
        offset = username.indexOf(realm.toUpper());

    if(offset > 1){
        tmpname = username.mid(0, offset - 1);
    }else{
        tmpname = username;
    }

    if(realm.length() > 2){
        newname = tmpname.trimmed() + "@" + realm;
    }else {
        newname = tmpname;
    }

    return newname;
}

QString LoginHelpers::getShortUsername(QString &username){

    QString newname, tmpname;
    int offset;
    QString realm = getUserRealm(username);

    offset = username.indexOf(realm.toLower());

    if(offset < 0)
        offset = username.indexOf(realm.toUpper());

    if(offset > 1){
        tmpname = username.mid(0, offset - 1);
    }else{
        tmpname = username;
    }


    return tmpname;

}
QString LoginHelpers::readRealm(){

    FILE *fp;
    char data[512] = {0};
    bool readerror = false;
    QString  tmpstring;
    QString outstr;
    int read_size;

    // fp = popen("net ads info", "r");
    fp = popen("/usr/sbin/realm list", "r");
    if (fp == NULL) {
        qWarning() << "Realm can not be read 1" ;
        readerror = true;
    }





    if(readerror == false){

        read_size = fread(data, 1, sizeof(data), fp);

        if( read_size < 1){
            qDebug() << "Realm can not be read" ;
        }else{
            outstr = getValueOfString(QString::fromLocal8Bit(data, read_size), QString("realm-name"));
        }


    }

    /* close */
    pclose(fp);


    return outstr;

}


QString LoginHelpers::getUserRealm(QString username){


    if(username.isNull() || username.isEmpty() || !username.compare(tr("Other User")))
        return QString("");

    if(ifLocalUser(username))
        return QString("");
    else
        return realM;
}


bool LoginHelpers::ifLocalUser(QString username){


    FILE *fp;
    char data[16096] = {0};
    bool readerror = false;
    QString  tmpstring;
    QString outstr = NULL;
    int read_size;
    QString command;
    QString compare;


    command = QString(" cat /etc/passwd | grep ") + username;

    fp = popen(command.toLocal8Bit().data(), "r");
    if (fp == NULL) {
        qWarning() << "username can not be read from passwd" ;
        readerror = true;
    }


    if(readerror == false){

        read_size = fread(data, 1, sizeof(data), fp);

        if( read_size < 5){
            //  qDebug() << tr("Realm can not be read 2") ;
            readerror = true;
        }else{
            outstr = QString(data);
        }


    }
    pclose(fp);

    if(readerror){
        return false;
    }else{

        compare = username +":";

        if(!outstr.left(compare.length()).compare(compare)){

            return true;
        }

        return false;

    }

    return false;
}

QString LoginHelpers::getValueOfString(QString data, QString value){

    QString result;

    int indx = data.indexOf(value);
    int endx = 0;

    if(indx == -1){
        return NULL;
    }

    indx += value.size();
    result = data.mid(indx);

    if(result.at(0) != QChar(':') && result.at(0) != QChar('=') ){

        if(result.at(1) != QChar(':') && result.at(1) != QChar('=') )
            return NULL;
        else
            indx = 2;

    }else{
        indx = 1;
    }


    while(result.at(indx) == ' ' || result.at(indx) == '\n'){
        indx++;
    }

    endx = indx;

    while(result.at(endx) != ' ' && result.at(endx) != '\n'){
        endx++;
    }

    result = result.mid(indx, (endx - indx));

    return result;
}


QString LoginHelpers::translateResetPwdMessage(QString message){


    QString res;

    if(message.contains("The password fails the dictionary check - it is based on a dictionary word")){

        res = tr("Error: The password fails the dictionary check - it is based on a dictionary word");

    }else if(message.contains("The password is shorter than 8 characters")){

        res = tr("Error: The password is shorter than 8 characters");

    }else if(message.contains("Please make sure the password meets the complexity constraints")){

        res = tr("Error: Please make sure the password meets the complexity constraints and don't use old passwords");

    }else if(message.contains("The password is the same as the old one")){

        res = tr("Error: The password is the same as the old one");

    }else if(message.contains("The password fails the dictionary check - it is too simplistic/systematic")){

        res = tr("Error: The password fails the dictionary check - it is too simplistic/systematic");

    }else{
        res = message;
    }

    return res;
}
