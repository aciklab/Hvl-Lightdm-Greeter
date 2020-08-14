#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtWidgets/QApplication>
#include <QDebug>
#include <QScreen>
#include <QQuickWindow>
#include <QQuickView>

#include "greeter.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine, engine2;


    const auto screens = qApp->screens();
    for (int i = 0; i <screens.count(); ++i) {
        qDebug() << i + 1 << screens.at(i)->geometry().width();

    }

    qmlRegisterType<Greeter>("org.hvl.greeter", 1, 0, "Greeter");

    engine.load(QUrl(QStringLiteral("qrc:/ui/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    QObject *rootObj = engine.rootObjects().first();
    rootObj->setProperty("width", screens.at(0)->geometry().width());
    rootObj->setProperty("height", screens.at(0)->geometry().height());
    rootObj->setProperty("x", screens.at(0)->geometry().x());

#if 0
    engine2.load(QUrl(QStringLiteral("qrc:/ui/other.qml")));
    QObject *rootObj2 = engine2.rootObjects().first();
    rootObj2->setProperty("width", screens.at(1)->geometry().width());
    rootObj2->setProperty("height", screens.at(1)->geometry().height());
    rootObj2->setProperty("x", screens.at(1)->geometry().x());
#endif


   // Greeter hvlgreeter;


    return app.exec();
}
