#ifndef DIALOG_WEBVIEW_H
#define DIALOG_WEBVIEW_H

#include <QDialog>
#include <QtWebEngine/QtWebEngine>
#include <QtWebEngineCore/QtWebEngineCore>
#include <QtWebEngineWidgets/QtWebEngineWidgets>
#include <QQmlApplicationEngine>

namespace Ui {
class Dialog_webview;
}

class Dialog_webview : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_webview(QWidget *parent = 0);
    ~Dialog_webview();


private slots:
    void on_closebutton_clicked();

private:

    QString checkPasswordResetWebPage();
    void Initialize();

    Ui::Dialog_webview *ui;
    QWebEngineView *viewx;
};

#endif // DIALOG_WEBVIEW_H
