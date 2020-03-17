#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QWidget>
#include <QDialog>
#include <QtGui>
#include <QLineEdit>
#include <QPushButton>

namespace Ui {
class Keyboard;
}

class Keyboard : public QWidget
{
    Q_OBJECT
    
public:
    explicit Keyboard(QWidget *parent = 0);
    void setLineEdit(QLineEdit * );
    ~Keyboard();
    QPushButton *enterButton;

Q_SIGNALS:
    void sendKey(QString key);
    void sendCloseEvent(void);

private slots:
    void keyboardHandler();
    void on_clear_clicked();
    void on_enterButton_clicked();

    void on_backButton_clicked();


    void on_shift_toggled(bool checked);

    void on_rightshift_toggled(bool checked);

    void on_Buttonaltgr_toggled(bool checked);

    void on_Buttonbackspace_clicked();

public slots:
    void setKeyboardLayout(QString &layout);

protected:
    void closeEvent (QCloseEvent *event);

private:

    void keySender(QString key);
    void readKeyboardLayout();
    void setCurrentSymbols(int n, bool caps);

    Ui::Keyboard *ui;
    QString outputText;
    QLineEdit *outputLineEdit;
    bool shift;
    bool char_clicked;
    bool altgr_clicked;

    QString layout_tr0 = "\"1234567890*-qwertyuıopğüasdfghjklşi,zxcvbnmöç.";
    QString layout_tr1 = "é!'^+%&/()=?_QWERTYUIOPĞÜASDFGHJKLŞİ;ZXCVBNMÖÇ:";
    QString layout_tr2 = "<>£#$½¾{[]}\\|@\255€¶™←ûîô\255¨~â§\255ª\255\255\255\255\255´'`«»¢“”nµ×·˙";

    QString layout_trf0 = "+1234567890/-fgğıodrnhpqwuieaütkmlyşxjövcçzsb.,";
    QString layout_trf1 = "*!\"^$%&'()=?_FGĞIODRNHPQWUİEAÜTKMLYŞXJÖVCÇZSB:;";
    QString layout_trf2 = "¬¹²#¼½¾{[]}\\|@\255\255¶ô¥®\255°£\255\255ûî€âû™\255µ\255´#`«»“¢”\255§×÷·";

    QString layout_us0 = "`1234567890-=qwertyuiop[]asdfghjkl;'\\zxcvbnm,./";
    QString layout_us1 = "~!@#$%^&*()_+QWERTYUIOP{}ASDFGHJKL:\"|ZXCVBNM<>?";
    QString layout_us2 = NULL;

    QString *currentLayout0;
    QString *currentLayout1;
    QString *currentLayout2;




};

#endif // KEYBOARD_H
