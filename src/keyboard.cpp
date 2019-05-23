#include "keyboard.h"
#include "ui_keyboard.h"
#include <QtGui>


// ui(new Ui::Keyboard)

Keyboard::Keyboard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Keyboard)

{
    ui->setupUi(this);

    setAttribute(Qt::WA_ShowWithoutActivating);
    readKeyboardLayout();
   // setKeyboardLayout(NULL);


    connect ( ui->Buttonacute, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Button1, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Button2, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Button3, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Button4, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Button5, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Button6, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Button7, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Button8, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Button9, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Button0, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonunderscore, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonequal, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );



    connect ( ui->Buttonq, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonw, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttone, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonr, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttont, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttony, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonu, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttoni, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttono, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonp, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonopenbracket, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonclosedbracket, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );



    connect ( ui->Buttona, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttons, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttond, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonf, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttong, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonh, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonj, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonk, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonl, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonsemicolon, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonsinglequote, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonbackslash, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );

    connect ( ui->Buttonz, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonx, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonc, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonv, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonb, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonn, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonm, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttoncomma, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttondot, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );
    connect ( ui->Buttonslash, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );





    connect ( ui->space, SIGNAL( clicked() ), this, SLOT( keyboardHandler() ) );


    connect ( ui->Buttonbackspace, SIGNAL( clicked() ), this, SLOT( on_backspace_clicked() ) );




    outputText = "";
    shift = false;
    char_clicked = false;
    altgr_clicked = false;
    setCurrentSymbols(0,false);


}

void Keyboard::keyboardHandler()
{
    QPushButton *button = (QPushButton *)sender();
    //qDebug() << "pressed " << button->text();

    QString inputText = button->text();

    if (inputText == "Space")
    {
        inputText = " ";


    }
    else if(inputText == "&&")
    {
        inputText = "&";

    }
    else if(inputText == "\\")
    {
        inputText = ui->Buttona->text() ;
    }
    else
    {

        inputText = inputText;

    }

    keySender(inputText);
    //ui->lineEdit->setText(outputText);
    clearFocus();
}


void Keyboard::setLineEdit(QLineEdit * line)
{


}

Keyboard::~Keyboard()
{
    delete ui;
}




void Keyboard::on_rightshift_toggled(bool checked)
{
    ui->shift->setChecked(checked);
}


void Keyboard::on_shift_toggled(bool checked)
{
    //checked = ui->rightshift->isChecked();

    if(checked)
    {
        shift = true;

        if(altgr_clicked)
            return;


        setCurrentSymbols(1, false);
    }
    else
    {
        shift = false;
        if(altgr_clicked)
            return;

        setCurrentSymbols(0, false);



    }

    clearFocus();

}

void Keyboard::on_Buttonaltgr_toggled(bool checked)
{
    if(checked){
        altgr_clicked = true;
        setCurrentSymbols(2, false);

    }else{

        if(shift){
            setCurrentSymbols(1, false);
        }else{
            setCurrentSymbols(0, false);
        }

        altgr_clicked = false;
    }

    clearFocus();
}


void Keyboard::on_clear_clicked()
{

    keySender(QString("clear"));

    clearFocus();
}

void Keyboard::on_backspace_clicked(){

    keySender(QString("backspace"));

    clearFocus();
}



void Keyboard::on_enterButton_clicked()
{
    //qDebug() << "enter";

    keySender(QString("enter"));

    close();
}


void Keyboard::on_backButton_clicked()
{
    close();

}


void Keyboard::keySender(QString key){

    emit sendKey(key);
}


void Keyboard::closeEvent (QCloseEvent *event)
{

    emit sendCloseEvent();
}

//read
//read keyboard layout
//set keyboard layouts



void Keyboard::readKeyboardLayout(){

    currentLayout0  = &layout_tr0;
    currentLayout1  = &layout_tr1;
    currentLayout2  = &layout_tr2;

}


void Keyboard::setKeyboardLayout(QString &layout){


    if(layout.compare("tr") == 0){

        currentLayout0  = &layout_tr0;
        currentLayout1  = &layout_tr1;
        currentLayout2  = &layout_tr2;

    }else if(layout.compare("tr f") == 0){
        currentLayout0  = &layout_trf0;
        currentLayout1  = &layout_trf1;
        currentLayout2  = &layout_trf2;

    }else{
        currentLayout0  = &layout_us0;
        currentLayout1  = &layout_us1;
        currentLayout2  = &layout_us2;

    }

    setCurrentSymbols(0,false);
}


void Keyboard::setCurrentSymbols(int n, bool caps){

    QString *layout;

    if (n == 0){

        layout = currentLayout0;

    }else if(n == 1){

        layout = currentLayout1;

    }else{

        layout = currentLayout2;

    }


    if(layout->isNull() || layout->isEmpty()){
        return;
    }


    ui->Buttonacute->setText(layout->at(0));
    ui->Button1->setText(layout->at(1));
    ui->Button2->setText(layout->at(2));
    ui->Button3->setText(layout->at(3));
    ui->Button4->setText(layout->at(4));
    ui->Button5->setText(layout->at(5));
    ui->Button6->setText(layout->at(6));
    ui->Button7->setText(layout->at(7));
    ui->Button8->setText(layout->at(8));
    ui->Button9->setText(layout->at(9));
    ui->Button0->setText(layout->at(10));
    ui->Buttonunderscore->setText(layout->at(11));
    ui->Buttonequal->setText(layout->at(12));



    ui->Buttonq->setText(layout->at(13));
    ui->Buttonw->setText(layout->at(14));
    ui->Buttone->setText(layout->at(15));
    ui->Buttonr->setText(layout->at(16));
    ui->Buttont->setText(layout->at(17));
    ui->Buttony->setText(layout->at(18));
    ui->Buttonu->setText(layout->at(19));
    ui->Buttoni->setText(layout->at(20));
    ui->Buttono->setText(layout->at(21));
    ui->Buttonp->setText(layout->at(22));
    ui->Buttonopenbracket->setText(layout->at(23));
    ui->Buttonclosedbracket->setText(layout->at(24));


    ui->Buttona->setText(layout->at(25));
    ui->Buttons->setText(layout->at(26));
    ui->Buttond->setText(layout->at(27));
    ui->Buttonf->setText(layout->at(28));
    ui->Buttong->setText(layout->at(29));
    ui->Buttonh->setText(layout->at(30));
    ui->Buttonj->setText(layout->at(31));
    ui->Buttonk->setText(layout->at(32));
    ui->Buttonl->setText(layout->at(33));
    ui->Buttonsemicolon->setText(layout->at(34));
    ui->Buttonsinglequote->setText(layout->at(35));
    ui->Buttonbackslash->setText(layout->at(36));

    ui->Buttonz->setText(layout->at(37));
    ui->Buttonx->setText(layout->at(38));
    ui->Buttonc->setText(layout->at(39));
    ui->Buttonv->setText(layout->at(40));
    ui->Buttonb->setText(layout->at(41));
    ui->Buttonn->setText(layout->at(42));
    ui->Buttonm->setText(layout->at(43));
    ui->Buttoncomma->setText(layout->at(44));
    ui->Buttondot->setText(layout->at(45));
    ui->Buttonslash->setText(layout->at(46));



    QHBoxLayout *lyt =ui->horizontalLayout1;
    QList<QPushButton *> list = lyt -> findChildren<QPushButton *> ();
    QPushButton *btn;

    for(int i = 0; i< lyt->count(); i++){


        if(QPushButton *pb = qobject_cast<QPushButton*>(lyt->itemAt(i)->widget())) {

            if(pb->text().compare("\255") == 0){
                pb->setText("");
            }
        }



    }

    lyt = ui->horizontalLayout_2;

    for(int i = 0; i< lyt->count(); i++){

        if(QPushButton *pb = qobject_cast<QPushButton*>(lyt->itemAt(i)->widget())) {

            if(pb->text().compare("\255") == 0){
                pb->setText("");
            }
        }
    }


    lyt = ui->horizontalLayout_3;

    for(int i = 0; i< lyt->count(); i++){

        if(QPushButton *pb = qobject_cast<QPushButton*>(lyt->itemAt(i)->widget())) {

            if(pb->text().compare("\255") == 0){
                pb->setText("");
            }
        }
    }

    lyt = ui->horizontalLayout_4;

    for(int i = 0; i< lyt->count(); i++){

        if(QPushButton *pb = qobject_cast<QPushButton*>(lyt->itemAt(i)->widget())) {

            if(pb->text().compare("\255") == 0){
                pb->setText("");
            }
        }
    }


    if(caps){

        ui->Buttonq->setText(layout->at(13).toUpper());
        ui->Buttonw->setText(layout->at(14).toUpper());
        ui->Buttone->setText(layout->at(15).toUpper());
        ui->Buttonr->setText(layout->at(16).toUpper());
        ui->Buttont->setText(layout->at(17).toUpper());
        ui->Buttony->setText(layout->at(18).toUpper());
        ui->Buttonu->setText(layout->at(19).toUpper());
        ui->Buttoni->setText(layout->at(20).toUpper());
        ui->Buttono->setText(layout->at(21).toUpper());
        ui->Buttonp->setText(layout->at(22).toUpper());
        ui->Buttonopenbracket->setText(layout->at(23).toUpper());
        ui->Buttonclosedbracket->setText(currentLayout0->at(24).toUpper());

        ui->Buttona->setText(layout->at(25).toUpper());
        ui->Buttons->setText(layout->at(26).toUpper());
        ui->Buttond->setText(layout->at(27).toUpper());
        ui->Buttonf->setText(layout->at(28).toUpper());
        ui->Buttong->setText(layout->at(29).toUpper());
        ui->Buttonh->setText(layout->at(30).toUpper());
        ui->Buttonj->setText(layout->at(31).toUpper());
        ui->Buttonk->setText(layout->at(32).toUpper());
        ui->Buttonl->setText(layout->at(33).toUpper());
        ui->Buttonsemicolon->setText(layout->at(34) .toUpper());
        ui->Buttonsinglequote->setText(layout->at(35).toUpper());

        ui->Buttonz->setText(layout->at(37).toUpper());
        ui->Buttonx->setText(layout->at(38).toUpper());
        ui->Buttonc->setText(layout->at(39).toUpper());
        ui->Buttonv->setText(layout->at(40).toUpper());
        ui->Buttonb->setText(layout->at(41).toUpper());
        ui->Buttonn->setText(layout->at(42).toUpper());
        ui->Buttonm->setText(layout->at(43).toUpper());
        ui->Buttoncomma->setText(layout->at(44).toUpper());
        ui->Buttondot->setText(layout->at(45).toUpper());

    }


}

//todo read xkb layout
//parse line
//parse symbol
//convert text to character


