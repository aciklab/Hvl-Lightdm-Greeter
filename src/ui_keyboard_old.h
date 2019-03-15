/********************************************************************************
** Form generated from reading UI file 'keyboard.ui'
**
** Created: Wed Feb 29 21:59:51 2012
**      by: Qt User Interface Compiler version 4.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_KEYBOARD_H
#define UI_KEYBOARD_H

#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

QT_BEGIN_NAMESPACE

class Ui_Keyboard
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLineEdit *lineEdit;
    QPushButton *backButton;
    QPushButton *clear;
    QGridLayout *gridLayout;
    QPushButton *Buttonq;
    QPushButton *Buttonw;
    QPushButton *Buttone;
    QPushButton *Buttonr;
    QPushButton *Buttont;
    QPushButton *Buttony;
    QPushButton *Buttonu;
    QPushButton *Buttoni;
    QPushButton *Buttono;
    QPushButton *Buttons;
    QPushButton *Buttond;
    QPushButton *Buttonf;
    QPushButton *Buttong;
    QPushButton *Buttonh;
    QPushButton *Buttonj;
    QPushButton *Buttonl;
    QPushButton *Buttonz;
    QPushButton *Buttonx;
    QPushButton *Buttonv;
    QPushButton *Buttonb;
    QPushButton *Buttonn;
    QPushButton *Buttonm;
    QPushButton *char_2;
    QPushButton *space;
    QPushButton *Button0;
    QPushButton *Button1;
    QPushButton *Button2;
    QPushButton *Button3;
    QPushButton *Button4;
    QPushButton *Button5;
    QPushButton *Button6;
    QPushButton *Button7;
    QPushButton *Button8;
    QPushButton *Button9;
    QPushButton *Buttonp;
    QPushButton *enterButton;
    QPushButton *Buttona;
    QPushButton *shift;
    QPushButton *Buttonc;
    QPushButton *Buttonk;

    void setupUi(QWidget *Keyboard)
    {
        if (Keyboard->objectName().isEmpty())
            Keyboard->setObjectName(QString::fromUtf8("Keyboard"));
        Keyboard->setWindowModality(Qt::NonModal);
        Keyboard->resize(600, 315);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Keyboard->sizePolicy().hasHeightForWidth());
        Keyboard->setSizePolicy(sizePolicy);
        Keyboard->setWindowTitle(QString::fromUtf8("Keyboard"));
        //Keyboard->setStyleSheet(QString::fromUtf8(""));
        verticalLayout = new QVBoxLayout(Keyboard);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        lineEdit = new QLineEdit(Keyboard);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        //lineEdit->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n""background-color: rgb(255, 255, 255);"));

        horizontalLayout->addWidget(lineEdit);

        backButton = new QPushButton(Keyboard);
        backButton->setObjectName(QString::fromUtf8("backButton"));

        horizontalLayout->addWidget(backButton);

        clear = new QPushButton(Keyboard);
        clear->setObjectName(QString::fromUtf8("clear"));

        horizontalLayout->addWidget(clear);


        verticalLayout->addLayout(horizontalLayout);

        gridLayout = new QGridLayout();
        gridLayout->setSpacing(2);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        Buttonq = new QPushButton(Keyboard);
        Buttonq->setObjectName(QString::fromUtf8("Buttonq"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(Buttonq->sizePolicy().hasHeightForWidth());
        Buttonq->setSizePolicy(sizePolicy1);
        Buttonq->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonq, 1, 0, 1, 1);

        Buttonw = new QPushButton(Keyboard);
        Buttonw->setObjectName(QString::fromUtf8("Buttonw"));
        sizePolicy1.setHeightForWidth(Buttonw->sizePolicy().hasHeightForWidth());
        Buttonw->setSizePolicy(sizePolicy1);
        Buttonw->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonw, 1, 1, 1, 1);

        Buttone = new QPushButton(Keyboard);
        Buttone->setObjectName(QString::fromUtf8("Buttone"));
        sizePolicy1.setHeightForWidth(Buttone->sizePolicy().hasHeightForWidth());
        Buttone->setSizePolicy(sizePolicy1);
        Buttone->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttone, 1, 2, 1, 1);

        Buttonr = new QPushButton(Keyboard);
        Buttonr->setObjectName(QString::fromUtf8("Buttonr"));
        sizePolicy1.setHeightForWidth(Buttonr->sizePolicy().hasHeightForWidth());
        Buttonr->setSizePolicy(sizePolicy1);
        Buttonr->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonr, 1, 3, 1, 1);

        Buttont = new QPushButton(Keyboard);
        Buttont->setObjectName(QString::fromUtf8("Buttont"));
        sizePolicy1.setHeightForWidth(Buttont->sizePolicy().hasHeightForWidth());
        Buttont->setSizePolicy(sizePolicy1);
        Buttont->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttont, 1, 4, 1, 1);

        Buttony = new QPushButton(Keyboard);
        Buttony->setObjectName(QString::fromUtf8("Buttony"));
        sizePolicy1.setHeightForWidth(Buttony->sizePolicy().hasHeightForWidth());
        Buttony->setSizePolicy(sizePolicy1);
        Buttony->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttony, 1, 5, 1, 1);

        Buttonu = new QPushButton(Keyboard);
        Buttonu->setObjectName(QString::fromUtf8("Buttonu"));
        sizePolicy1.setHeightForWidth(Buttonu->sizePolicy().hasHeightForWidth());
        Buttonu->setSizePolicy(sizePolicy1);
        Buttonu->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonu, 1, 6, 1, 1);

        Buttoni = new QPushButton(Keyboard);
        Buttoni->setObjectName(QString::fromUtf8("Buttoni"));
        sizePolicy1.setHeightForWidth(Buttoni->sizePolicy().hasHeightForWidth());
        Buttoni->setSizePolicy(sizePolicy1);
        Buttoni->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttoni, 1, 7, 1, 1);

        Buttono = new QPushButton(Keyboard);
        Buttono->setObjectName(QString::fromUtf8("Buttono"));
        sizePolicy1.setHeightForWidth(Buttono->sizePolicy().hasHeightForWidth());
        Buttono->setSizePolicy(sizePolicy1);
        Buttono->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttono, 1, 8, 1, 1);

        Buttons = new QPushButton(Keyboard);
        Buttons->setObjectName(QString::fromUtf8("Buttons"));
        sizePolicy1.setHeightForWidth(Buttons->sizePolicy().hasHeightForWidth());
        Buttons->setSizePolicy(sizePolicy1);
        Buttons->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttons, 3, 1, 1, 1);

        Buttond = new QPushButton(Keyboard);
        Buttond->setObjectName(QString::fromUtf8("Buttond"));
        sizePolicy1.setHeightForWidth(Buttond->sizePolicy().hasHeightForWidth());
        Buttond->setSizePolicy(sizePolicy1);
        Buttond->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttond, 3, 2, 1, 1);

        Buttonf = new QPushButton(Keyboard);
        Buttonf->setObjectName(QString::fromUtf8("Buttonf"));
        sizePolicy1.setHeightForWidth(Buttonf->sizePolicy().hasHeightForWidth());
        Buttonf->setSizePolicy(sizePolicy1);
        Buttonf->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonf, 3, 3, 1, 1);

        Buttong = new QPushButton(Keyboard);
        Buttong->setObjectName(QString::fromUtf8("Buttong"));
        sizePolicy1.setHeightForWidth(Buttong->sizePolicy().hasHeightForWidth());
        Buttong->setSizePolicy(sizePolicy1);
        Buttong->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttong, 3, 4, 1, 1);

        Buttonh = new QPushButton(Keyboard);
        Buttonh->setObjectName(QString::fromUtf8("Buttonh"));
        sizePolicy1.setHeightForWidth(Buttonh->sizePolicy().hasHeightForWidth());
        Buttonh->setSizePolicy(sizePolicy1);
        Buttonh->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonh, 3, 5, 1, 1);

        Buttonj = new QPushButton(Keyboard);
        Buttonj->setObjectName(QString::fromUtf8("Buttonj"));
        sizePolicy1.setHeightForWidth(Buttonj->sizePolicy().hasHeightForWidth());
        Buttonj->setSizePolicy(sizePolicy1);
        Buttonj->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonj, 3, 6, 1, 1);

        Buttonl = new QPushButton(Keyboard);
        Buttonl->setObjectName(QString::fromUtf8("Buttonl"));
        sizePolicy1.setHeightForWidth(Buttonl->sizePolicy().hasHeightForWidth());
        Buttonl->setSizePolicy(sizePolicy1);
        Buttonl->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonl, 3, 8, 1, 1);

        Buttonz = new QPushButton(Keyboard);
        Buttonz->setObjectName(QString::fromUtf8("Buttonz"));
        sizePolicy1.setHeightForWidth(Buttonz->sizePolicy().hasHeightForWidth());
        Buttonz->setSizePolicy(sizePolicy1);
        Buttonz->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonz, 4, 1, 1, 1);

        Buttonx = new QPushButton(Keyboard);
        Buttonx->setObjectName(QString::fromUtf8("Buttonx"));
        sizePolicy1.setHeightForWidth(Buttonx->sizePolicy().hasHeightForWidth());
        Buttonx->setSizePolicy(sizePolicy1);
        Buttonx->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonx, 4, 2, 1, 1);

        Buttonv = new QPushButton(Keyboard);
        Buttonv->setObjectName(QString::fromUtf8("Buttonv"));
        sizePolicy1.setHeightForWidth(Buttonv->sizePolicy().hasHeightForWidth());
        Buttonv->setSizePolicy(sizePolicy1);
        Buttonv->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonv, 4, 4, 1, 1);

        Buttonb = new QPushButton(Keyboard);
        Buttonb->setObjectName(QString::fromUtf8("Buttonb"));
        sizePolicy1.setHeightForWidth(Buttonb->sizePolicy().hasHeightForWidth());
        Buttonb->setSizePolicy(sizePolicy1);
        Buttonb->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonb, 4, 5, 1, 1);

        Buttonn = new QPushButton(Keyboard);
        Buttonn->setObjectName(QString::fromUtf8("Buttonn"));
        sizePolicy1.setHeightForWidth(Buttonn->sizePolicy().hasHeightForWidth());
        Buttonn->setSizePolicy(sizePolicy1);
        Buttonn->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonn, 4, 6, 1, 1);

        Buttonm = new QPushButton(Keyboard);
        Buttonm->setObjectName(QString::fromUtf8("Buttonm"));
        sizePolicy1.setHeightForWidth(Buttonm->sizePolicy().hasHeightForWidth());
        Buttonm->setSizePolicy(sizePolicy1);
        Buttonm->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonm, 4, 7, 1, 1);

        char_2 = new QPushButton(Keyboard);
        char_2->setObjectName(QString::fromUtf8("char_2"));
        sizePolicy1.setHeightForWidth(char_2->sizePolicy().hasHeightForWidth());
        char_2->setSizePolicy(sizePolicy1);
        char_2->setMinimumSize(QSize(20, 0));
        char_2->setCheckable(true);
        char_2->setChecked(false);

        gridLayout->addWidget(char_2, 4, 8, 2, 1);

        space = new QPushButton(Keyboard);
        space->setObjectName(QString::fromUtf8("space"));
        space->setMinimumSize(QSize(20, 50));

        gridLayout->addWidget(space, 5, 1, 1, 7);

        Button0 = new QPushButton(Keyboard);
        Button0->setObjectName(QString::fromUtf8("Button0"));
        sizePolicy1.setHeightForWidth(Button0->sizePolicy().hasHeightForWidth());
        Button0->setSizePolicy(sizePolicy1);
        Button0->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button0, 0, 0, 1, 1);

        Button1 = new QPushButton(Keyboard);
        Button1->setObjectName(QString::fromUtf8("Button1"));
        sizePolicy1.setHeightForWidth(Button1->sizePolicy().hasHeightForWidth());
        Button1->setSizePolicy(sizePolicy1);
        Button1->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button1, 0, 1, 1, 1);

        Button2 = new QPushButton(Keyboard);
        Button2->setObjectName(QString::fromUtf8("Button2"));
        sizePolicy1.setHeightForWidth(Button2->sizePolicy().hasHeightForWidth());
        Button2->setSizePolicy(sizePolicy1);
        Button2->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button2, 0, 2, 1, 1);

        Button3 = new QPushButton(Keyboard);
        Button3->setObjectName(QString::fromUtf8("Button3"));
        sizePolicy1.setHeightForWidth(Button3->sizePolicy().hasHeightForWidth());
        Button3->setSizePolicy(sizePolicy1);
        Button3->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button3, 0, 3, 1, 1);

        Button4 = new QPushButton(Keyboard);
        Button4->setObjectName(QString::fromUtf8("Button4"));
        sizePolicy1.setHeightForWidth(Button4->sizePolicy().hasHeightForWidth());
        Button4->setSizePolicy(sizePolicy1);
        Button4->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button4, 0, 4, 1, 1);

        Button5 = new QPushButton(Keyboard);
        Button5->setObjectName(QString::fromUtf8("Button5"));
        sizePolicy1.setHeightForWidth(Button5->sizePolicy().hasHeightForWidth());
        Button5->setSizePolicy(sizePolicy1);
        Button5->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button5, 0, 5, 1, 1);

        Button6 = new QPushButton(Keyboard);
        Button6->setObjectName(QString::fromUtf8("Button6"));
        sizePolicy1.setHeightForWidth(Button6->sizePolicy().hasHeightForWidth());
        Button6->setSizePolicy(sizePolicy1);
        Button6->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button6, 0, 6, 1, 1);

        Button7 = new QPushButton(Keyboard);
        Button7->setObjectName(QString::fromUtf8("Button7"));
        sizePolicy1.setHeightForWidth(Button7->sizePolicy().hasHeightForWidth());
        Button7->setSizePolicy(sizePolicy1);
        Button7->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button7, 0, 7, 1, 1);

        Button8 = new QPushButton(Keyboard);
        Button8->setObjectName(QString::fromUtf8("Button8"));
        sizePolicy1.setHeightForWidth(Button8->sizePolicy().hasHeightForWidth());
        Button8->setSizePolicy(sizePolicy1);
        Button8->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button8, 0, 8, 1, 1);

        Button9 = new QPushButton(Keyboard);
        Button9->setObjectName(QString::fromUtf8("Button9"));
        sizePolicy1.setHeightForWidth(Button9->sizePolicy().hasHeightForWidth());
        Button9->setSizePolicy(sizePolicy1);
        Button9->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Button9, 0, 9, 1, 1);

        Buttonp = new QPushButton(Keyboard);
        Buttonp->setObjectName(QString::fromUtf8("Buttonp"));
        sizePolicy1.setHeightForWidth(Buttonp->sizePolicy().hasHeightForWidth());
        Buttonp->setSizePolicy(sizePolicy1);
        Buttonp->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonp, 1, 9, 1, 1);

        enterButton = new QPushButton(Keyboard);
        enterButton->setObjectName(QString::fromUtf8("enterButton"));
        sizePolicy1.setHeightForWidth(enterButton->sizePolicy().hasHeightForWidth());
        enterButton->setSizePolicy(sizePolicy1);
        enterButton->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(enterButton, 3, 9, 3, 1);

        Buttona = new QPushButton(Keyboard);
        Buttona->setObjectName(QString::fromUtf8("Buttona"));
        sizePolicy1.setHeightForWidth(Buttona->sizePolicy().hasHeightForWidth());
        Buttona->setSizePolicy(sizePolicy1);
        Buttona->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttona, 3, 0, 1, 1);

        shift = new QPushButton(Keyboard);
        shift->setObjectName(QString::fromUtf8("shift"));
        sizePolicy1.setHeightForWidth(shift->sizePolicy().hasHeightForWidth());
        shift->setSizePolicy(sizePolicy1);
        shift->setMinimumSize(QSize(20, 0));
       // shift->setStyleSheet(QString::fromUtf8(""));
        shift->setCheckable(false);

        gridLayout->addWidget(shift, 4, 0, 2, 1);

        Buttonc = new QPushButton(Keyboard);
        Buttonc->setObjectName(QString::fromUtf8("Buttonc"));
        sizePolicy1.setHeightForWidth(Buttonc->sizePolicy().hasHeightForWidth());
        Buttonc->setSizePolicy(sizePolicy1);
        Buttonc->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonc, 4, 3, 1, 1);

        Buttonk = new QPushButton(Keyboard);
        Buttonk->setObjectName(QString::fromUtf8("Buttonk"));
        sizePolicy1.setHeightForWidth(Buttonk->sizePolicy().hasHeightForWidth());
        Buttonk->setSizePolicy(sizePolicy1);
        Buttonk->setMinimumSize(QSize(20, 0));

        gridLayout->addWidget(Buttonk, 3, 7, 1, 1);


        verticalLayout->addLayout(gridLayout);


        retranslateUi(Keyboard);

        QMetaObject::connectSlotsByName(Keyboard);
    } // setupUi

    void retranslateUi(QWidget *Keyboard)
    {
        backButton->setText(QApplication::translate("Keyboard", "Back", 0));
        clear->setText(QApplication::translate("Keyboard", "Clear", 0));
        Buttonq->setText(QApplication::translate("Keyboard", "q", 0));
        Buttonw->setText(QApplication::translate("Keyboard", "w", 0));
        Buttone->setText(QApplication::translate("Keyboard", "e", 0));
        Buttonr->setText(QApplication::translate("Keyboard", "r", 0));
        Buttont->setText(QApplication::translate("Keyboard", "t", 0));
        Buttony->setText(QApplication::translate("Keyboard", "y", 0));
        Buttonu->setText(QApplication::translate("Keyboard", "u", 0));
        Buttoni->setText(QApplication::translate("Keyboard", "i", 0));
        Buttono->setText(QApplication::translate("Keyboard", "o", 0));
        Buttons->setText(QApplication::translate("Keyboard", "s", 0));
        Buttond->setText(QApplication::translate("Keyboard", "d", 0));
        Buttonf->setText(QApplication::translate("Keyboard", "f", 0));
        Buttong->setText(QApplication::translate("Keyboard", "g", 0));
        Buttonh->setText(QApplication::translate("Keyboard", "h", 0));
        Buttonj->setText(QApplication::translate("Keyboard", "j", 0));
        Buttonl->setText(QApplication::translate("Keyboard", "l", 0));
        Buttonz->setText(QApplication::translate("Keyboard", "z", 0));
        Buttonx->setText(QApplication::translate("Keyboard", "x", 0));
        Buttonv->setText(QApplication::translate("Keyboard", "v", 0));
        Buttonb->setText(QApplication::translate("Keyboard", "b", 0));
        Buttonn->setText(QApplication::translate("Keyboard", "n", 0));
        Buttonm->setText(QApplication::translate("Keyboard", "m", 0));
        char_2->setText(QApplication::translate("Keyboard", "Char", 0));
        space->setText(QApplication::translate("Keyboard", "Space", 0));
        Button0->setText(QApplication::translate("Keyboard", "0", 0));
        Button1->setText(QApplication::translate("Keyboard", "1", 0));
        Button2->setText(QApplication::translate("Keyboard", "2", 0));
        Button3->setText(QApplication::translate("Keyboard", "3", 0));
        Button4->setText(QApplication::translate("Keyboard", "4", 0));
        Button5->setText(QApplication::translate("Keyboard", "5", 0));
        Button6->setText(QApplication::translate("Keyboard", "6", 0));
        Button7->setText(QApplication::translate("Keyboard", "7", 0));
        Button8->setText(QApplication::translate("Keyboard", "8", 0));
        Button9->setText(QApplication::translate("Keyboard", "9", 0));
        Buttonp->setText(QApplication::translate("Keyboard", "p", 0));
        enterButton->setText(QApplication::translate("Keyboard", "Enter", 0));
        Buttona->setText(QApplication::translate("Keyboard", "a", 0));
        shift->setText(QApplication::translate("Keyboard", "Shift", 0));
        Buttonc->setText(QApplication::translate("Keyboard", "c", 0));
        Buttonk->setText(QApplication::translate("Keyboard", "k", 0));
        Q_UNUSED(Keyboard);
    } // retranslateUi

};

namespace Ui {
    class Keyboard: public Ui_Keyboard {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_KEYBOARD_H
