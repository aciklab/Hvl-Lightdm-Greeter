#include <QRect>
#include <QApplication>
#include <QGuiApplication>
#include <QDesktopWidget>
#include <QPalette>
#include <QString>
#include <QDebug>
#include <QtNetwork/QtNetwork>
#include <QMessageBox>
#include <QTimer>
#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

#include "mainwindow.h"
#include "loginform.h"
#include "settings.h"
#include "settingsform.h"
#include "clockform.h"
#include "powerform.h"
#include "x11_utils.h"
#include "stdlib.h"





#ifdef SCREENKEYBOARD
#include "keyboard.h"
#endif

#define LOGINFORM_DEFAULT_WIDTH 800
#define LOGINFORM_DEFAULT_HEIGHT 336


bool MainWindow::selectflag = false;
int MainWindow::image_index = 0;
MainWindow **MainWindow::mainWindowsList = NULL;

QImage *MainWindow::screenImage = NULL;

int MainWindow::widgetScreen = 0;

MainWindow::MainWindow(int screen, QWidget *parent) :
    QWidget(parent),
    m_Screen(screen)
{
    setObjectName(QString("MainWindow_%1").arg(screen));

    //QRect screenRect = QApplication::desktop()->screenGeometry(screen);
    // setGeometry(screenRect);
    qScreen = QGuiApplication::screens()[screen];
    setGeometry(qScreen->geometry());
    QRect screenRect = qScreen->geometry();

    if(mainWindowsList != NULL){
        mainWindowsList[screen] = this;

    }

    previousScreen = 1;


    // display login dialog only in the main screen
    
    if (qScreen == QGuiApplication::primaryScreen()) {

        if(QGuiApplication::screens().length() > 1){
            mainWindowsList = (MainWindow**)malloc(sizeof(MainWindow*) *QGuiApplication::screens().length());
            mainWindowsList[m_Screen] = this;
        }

        qApp->installEventFilter(this);
        mirrored = 0;
        currentScreen = m_Screen;

        widgetScreen = m_Screen;

        m_LoginForm = new LoginForm(this);

        int maxX = screenRect.width() - m_LoginForm->width();
        int maxY = screenRect.height() - m_LoginForm->height();
        int defaultX = 50*maxX/100;
        int defaultY = 30*maxY/100;
        int offsetX = getOffset(Settings().offsetX_loginform(), maxX, defaultX);
        int offsetY = getOffset(Settings().offsetY_loginform(), maxY, defaultY);
        
        m_LoginForm->move(offsetX, offsetY);
        m_LoginForm->show();


        m_SettingsForm = new SettingsForm(this);

        maxX = screenRect.width() - m_SettingsForm->width();
        maxY = screenRect.height() - m_SettingsForm->height();
        defaultX = 100*maxX/100;
        defaultY = 80*maxY/100;
        offsetX = getOffset(Settings().offsetX_settingsform(), maxX, defaultX);
        offsetY = getOffset(Settings().offsetY_settingsform(), maxY, defaultY);


        m_SettingsForm->move(offsetX, offsetY);
        m_SettingsForm->show();

        if(Settings().show_domaininfo())
            m_SettingsForm->updateHostName(m_LoginForm->getHostname());

        m_PowerForm = new PowerForm(this);

        maxX = screenRect.width() - m_PowerForm->width();
        maxY = screenRect.height() - m_PowerForm->height();

        defaultX = 50*maxX/100;
        defaultY = 80*maxY/100;
        offsetX = getOffset(Settings().offsetX_powerform(), maxX, defaultX);
        offsetY = getOffset(Settings().offsetY_powerform(), maxY, defaultY);

        m_PowerForm->move(offsetX, offsetY);
        m_PowerForm->show();


        m_ClockForm = new clockForm(this);

        maxX = screenRect.width() - m_ClockForm->width();
        maxY = screenRect.height() - m_ClockForm->height();
        defaultX = 5*maxX/100;
        defaultY = 5*maxY/100;
        offsetX = getOffset(Settings().offsetX_clockform(), maxX, defaultX);
        offsetY = getOffset(Settings().offsetY_clockform(), maxY, defaultY);

        if(screenRect.width() <= 800){
            m_ClockForm->move(0, 0);

        }else{
            m_ClockForm->move(offsetX, offsetY);
        }
        m_ClockForm->show();


        setMainBackground(true);

        // This hack ensures that the primary screen will have focus
        // if there are more screens (move the mouse cursor in the center
        // of primary screen - not in the center of all X area). It
        // won't affect single-screen environments.
        int centerX = screenRect.width()/2 + screenRect.x();
        int centerY = screenRect.height()/2 + screenRect.y();
        QCursor::setPos(centerX, centerY);
        this->cursor().setShape(Qt::ArrowCursor);

        QObject::connect(m_SettingsForm, &SettingsForm::sendNWStatusSignal, this, &MainWindow::receiveNetworkStatus);
        QObject::connect(this, &MainWindow::sendNetworkStatustoChilds, m_LoginForm, &LoginForm::stopWaitOperation);
        QObject::connect(m_LoginForm, &LoginForm::selectKeyboard, m_SettingsForm, &SettingsForm::keyboardSelectSlot);
        QObject::connect(m_SettingsForm, &SettingsForm::sendSessionInfo, m_LoginForm, &LoginForm::setCurrentSession);
        QObject::connect(m_LoginForm, &LoginForm::sendCurrentUser, m_SettingsForm, &SettingsForm::receiveCurrentUser);
        QObject::connect(m_LoginForm, &LoginForm::resetHideTimer, this, &MainWindow::resetHideFormsTimer);
        keyboardInit();


        int slide_timeout = Settings().slideShow_timeout();

        if(slide_timeout < 1)
            slide_timeout = 60;



        backgroundTimer = new QTimer();
        backgroundTimer->setTimerType(Qt::TimerType::CoarseTimer);
        backgroundTimer->setInterval(slide_timeout * 1000);
        backgroundTimer->setSingleShot(false);
        backgroundTimer->start();
        connect(backgroundTimer, SIGNAL(timeout()), this, SLOT(backgroundTimerCallback()));

        int screen_saver_timeout = Settings().screenSaver_timeout();

        if(screen_saver_timeout < 1)
            screen_saver_timeout = 30;


        formHideTimer = new QTimer();
        formHideTimer->setTimerType(Qt::TimerType::CoarseTimer);
        formHideTimer->setInterval(screen_saver_timeout * 1000);
        formHideTimer->setSingleShot(false);
        formHideTimer->start();
        connect(formHideTimer, SIGNAL(timeout()), this, SLOT(hideForms()));


    }else{


        setOtherBackgrounds(screenImage, true, false);
        if(QGuiApplication::screens().length() > 1 && mainWindowsList[1] != NULL && mainWindowsList[0] != NULL){
            if(mainWindowsList[1]->pos().x() == mainWindowsList[0]->pos().x()){
                mainWindowsList[1]->hide();
                mirrored = 1;
            }
        }

    }




}

MainWindow::~MainWindow()
{
}

bool MainWindow::showLoginForm()
{
    return qScreen == QGuiApplication::primaryScreen();
}

void MainWindow::setFocus(Qt::FocusReason reason)
{
    qDebug() <<reason;
}

int MainWindow::getOffset(QString settingsOffset, int maxVal, int defaultVal)
{

    int offset = defaultVal > maxVal ? maxVal : defaultVal;

    if (! settingsOffset.isEmpty())  {
        if (QRegExp("^\\d+px$", Qt::CaseInsensitive).exactMatch(settingsOffset))  {
            offset = settingsOffset.left(settingsOffset.size() - 2).toInt();
            if (offset > maxVal) offset = maxVal;
        }
        else if (QRegExp("^\\d+%$", Qt::CaseInsensitive).exactMatch(settingsOffset)) {
            int offsetPct = settingsOffset.left(settingsOffset.size() -1).toInt();
            if (offsetPct > 100) offsetPct = 100;
            offset = (maxVal * offsetPct)/100;
        }
        else {
            qWarning() << tr("Could not understand ") << settingsOffset
                       << tr("- must be of form <positivenumber>px or <positivenumber>%, e.g. 35px or 25%") ;
        }
    }

    return offset;
}

void MainWindow::setMainBackground(bool start)
{
    QImage backgroundImage;
    QSettings greeterSettings(CONFIG_FILE, QSettings::IniFormat);


    QRect rect;
    QPalette palette;


    this->setStyleSheet("background-position: center;");

    rect = qScreen->geometry();
    QString pathToBackgroundImageDir = greeterSettings.value(BACKGROUND_IMAGE_DIR_KEY).toString();
    QString pathToBackgroundImage = greeterSettings.value(BACKGROUND_IMAGE_KEY).toString();

    if(!pathToBackgroundImage.isNull() && !pathToBackgroundImage.isEmpty()){

        backgroundImage = QImage(pathToBackgroundImage);

    }else if(!pathToBackgroundImageDir.isNull() && !pathToBackgroundImageDir.isEmpty()){

        if(pathToBackgroundImageDir[pathToBackgroundImageDir.length() - 1 ] != '/')
            pathToBackgroundImageDir += '/';


        QDir directory(pathToBackgroundImageDir);
        QStringList backgroundImageList = directory.entryList(QStringList() << "*.jpg" << "*.JPG" << "*.jpeg" << "*.JPEG"<<"*.png"<<"*.PNG",QDir::Files);

        if(!selectflag){



            if(backgroundImageList.count() > 1)
                image_index = QRandomGenerator::global()->generate() % (backgroundImageList.count());
            else
                image_index = 0;

            selectflag = true;
        }else{
            image_index++;
            if(image_index >= backgroundImageList.count())
                image_index = 0;
        }

        if (!backgroundImageList.isEmpty()) {
            QString imagepath = pathToBackgroundImageDir + backgroundImageList[image_index];

            backgroundImage = QImage(imagepath);
            //  qDebug() << imagepath << tr(" is set as an background image");

        }else{
            qWarning() << tr("Not able to read image at index: ") << image_index << tr(" as image");
            pathToBackgroundImage = ":/resources/bgs/bg1.jpg";
            backgroundImage = QImage(pathToBackgroundImage);
        }


    }
    else{
        pathToBackgroundImage = ":/resources/bgs/bg1.jpg";
        backgroundImage = QImage(pathToBackgroundImage);


#if 0
        if (backgroundImage.isNull()) {
            qWarning() << tr("Not able to read") << pathToBackgroundImage << tr("as default image");
        }else{
            qDebug()  << pathToBackgroundImage << tr(" is set as an image");
        }
#endif
    }
    

    if (backgroundImage.isNull()) {
        palette.setColor(QPalette::Background, qRgb(255, 203, 80));
    }
    else {

        //resizeImage(rect, backgroundImage);//todo:test

        if(screenImage)
            screenImage->~QImage();

        screenImage = new QImage(backgroundImage);

        finalImage = resizeImage(rect, backgroundImage);
        // screenImage = &backgroundImage;

        if((!formshidden) && m_LoginForm != NULL ){
            QGraphicsOpacityEffect *opacitr = new QGraphicsOpacityEffect;
            opacitr->setOpacity(0.5);

            QImage result = applyEffectToImage(finalImage, opacitr, 0);

            QBrush brush(result);
            palette.setBrush(this->backgroundRole(), brush);

        }else{
            QBrush brush(finalImage);
            palette.setBrush(this->backgroundRole(), brush);
        }




        //QBrush brush(backgroundImage.scaled(rect.width(), rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    }
    this->setPalette(palette);

    /* We are painting x root background with current greeter background */
    if(qScreen == QGuiApplication::primaryScreen()){
        if(!finalImage.isNull()){
            if(start)
                setRootBackground(finalImage, qScreen);
        }else{
            QImage tmpimage(rect.width(), rect.height(), QImage::Format_ARGB32_Premultiplied) ;
            tmpimage.fill(qRgb(255,203,80));
            if(start)
                setRootBackground(tmpimage, qScreen);

        }
    }
}


void MainWindow::setOtherBackgrounds(QImage *backgroundImage, bool start, bool forcemain){



    QRect rect;
    QPalette palette;
    QImage tmp_image;

    if(m_LoginForm != NULL && !forcemain)
        return;



    this->setStyleSheet("background-position: center;");

    rect = qScreen->geometry();

    if(!backgroundImage){
        palette.setColor(QPalette::Background, qRgb(255, 203, 80));
        return;

    }

    if (backgroundImage->isNull()) {
        palette.setColor(QPalette::Background, qRgb(255, 203, 80));
        return;
    }
    else {


        tmp_image = resizeImage(rect, *backgroundImage);

        if(!formshidden && m_LoginForm != NULL ){
            QGraphicsOpacityEffect *opacity = new QGraphicsOpacityEffect;
            opacity->setOpacity(0.5);

            QImage result = applyEffectToImage(tmp_image, opacity, 0);

            QBrush brush(result);
            palette.setBrush(this->backgroundRole(), brush);

        }else{
            QBrush brush(tmp_image);
            palette.setBrush(this->backgroundRole(), brush);
        }




        //QBrush brush(backgroundImage.scaled(rect.width(), rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    }
    this->setPalette(palette);

    /* We are painting x root background with current greeter background */
    if(qScreen == QGuiApplication::primaryScreen()){
        if(!tmp_image.isNull()){
            if(start)
                setRootBackground(tmp_image, qScreen);
        }else{
            QImage tmpimage(rect.width(), rect.height(), QImage::Format_ARGB32_Premultiplied) ;
            tmpimage.fill(qRgb(255,203,80));
            if(start)
                setRootBackground(tmpimage, qScreen);

        }
    }

}


QImage MainWindow::applyEffectToImage(QImage src, QGraphicsEffect *effect, int extent = 0){
    if(src.isNull()) return QImage();   //No need to do anything else!
    if(!effect) return src;             //No need to do anything else!
    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(QPixmap::fromImage(src));
    item.setGraphicsEffect(effect);
    scene.addItem(&item);
    QImage res(src.size()+QSize(extent*2, extent*2), QImage::Format_ARGB32);
    res.fill(Qt::transparent);
    QPainter ptr(&res);
    scene.render(&ptr, QRectF(), QRectF( -extent, -extent, src.width()+extent*2, src.height()+extent*2 ) );
    return res;
}



QImage MainWindow::resizeImage(QRect screen_rect, QImage input_image){

    int image_width;
    int image_height;
    QImage new_image;
    //QImage final_image(screen_rect.width(), screen_rect.height(), input_image.format());
    QImage final_image;

    qreal screen_aspect_ratio = (qreal)screen_rect.width() / (qreal)screen_rect.height();

    qreal image_aspect_ratio = (qreal)input_image.width() / (qreal)input_image.height();

    if(image_aspect_ratio <= 1){

        image_width = screen_rect.width();
       // image_height = screen_rect.width() / image_aspect_ratio;

        new_image = input_image.scaled(image_width, screen_rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        final_image = input_image.scaled(screen_rect.width(), screen_rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);


    }else{//> 1


        if(screen_aspect_ratio > image_aspect_ratio){

            image_height = screen_rect.width() / image_aspect_ratio;
            image_width =screen_rect.width();
            new_image = input_image.scaled(image_width, image_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            int pixeloffset = (image_height - screen_rect.height()) / 2;
            final_image = new_image.copy(0, pixeloffset, image_width, image_height - (  pixeloffset));

        }else if(screen_aspect_ratio <= image_aspect_ratio){
            //norrower
            image_width = screen_rect.height() * image_aspect_ratio;
            image_height = screen_rect.height();
            new_image = input_image.scaled(image_width, image_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

            int pixeloffset = (image_width - screen_rect.width()) / 2;

            final_image = new_image.copy(pixeloffset, 0, image_width -  ( 2 * pixeloffset), image_height);

        }

    }

    return final_image;

}



void MainWindow::moveForms(int screen_number){


    QRect screenRect = qScreen->geometry();
    setGeometry(screenRect);

    screenRect.x();

    int maxX = screenRect.width() - m_LoginForm->width();
    int maxY = screenRect.height() - m_LoginForm->height();
    int defaultX = 50*maxX/100;
    int defaultY = 30*maxY/100;
    int offsetX = getOffset(Settings().offsetX_loginform(), maxX, defaultX);
    int offsetY = getOffset(Settings().offsetY_loginform(), maxY, defaultY);

    m_LoginForm->move(offsetX , offsetY);
    // m_LoginForm->show();


    maxX = screenRect.width() - m_SettingsForm->width();
    maxY = screenRect.height() - m_SettingsForm->height();
    defaultX = 100*maxX/100;
    defaultY = 80*maxY/100;
    offsetX = getOffset(Settings().offsetX_settingsform(), maxX, defaultX);
    offsetY = getOffset(Settings().offsetY_settingsform(), maxY, defaultY);


    m_SettingsForm->move(offsetX, offsetY);
    // m_SettingsForm->show();

    maxX = screenRect.width() - m_PowerForm->width();
    maxY = screenRect.height() - m_PowerForm->height();
    defaultX = 50*maxX/100;
    defaultY = 80*maxY/100;
    offsetX = getOffset(Settings().offsetX_powerform(), maxX, defaultX);
    offsetY = getOffset(Settings().offsetY_powerform(), maxY, defaultY);


    m_PowerForm->move(offsetX, offsetY);
    //m_PowerForm->show();


}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{

    if (event->type() == QEvent::MouseMove || event->type() == QEvent::KeyPress || event->type() == QEvent::MouseButtonPress){
        showForms();
    }

    if (event->type() == QEvent::MouseMove && QGuiApplication::screens().length() > 1)
    {

        QPoint globalCursorPos = QCursor::pos();

        if(mainWindowsList[1] != NULL && mainWindowsList[0] != NULL){
            if(mainWindowsList[1]->pos().x() == mainWindowsList[0]->pos().x()){
                mainWindowsList[1]->hide();
                mirrored = 1;
            }
        }


        if(mirrored)
            mainWindowsList[1]->hide();

        QScreen *mousescreen =QGuiApplication::screenAt(globalCursorPos);

        if(mousescreen != qScreen){

            // mainWindowsList[previousScreen]->hide();
            prevQScreen = qScreen;
            qScreen = mousescreen;



            QRect prvScreenRect = prevQScreen->geometry();
            QRect curScreenRect = qScreen->geometry();

            for (int i = 0; i < QGuiApplication::screens().length(); i++){
                if(mainWindowsList[i]->pos().x() == prvScreenRect.x() && mainWindowsList[i]->pos().y() == prvScreenRect.y()){
                    mainWindowsList[i]->hide();
                    mainWindowsList[i]->setGeometry(curScreenRect);

                    mainWindowsList[i]->move(QPoint(curScreenRect.x(), curScreenRect.y()));
                    mainWindowsList[i]->qScreen = qScreen;

                    mainWindowsList[i]->setOtherBackgrounds(screenImage,false, true);
                    mainWindowsList[i]->show();
                    // mainWindowsList[previousScreen]->hide();



                }else if(mainWindowsList[i]->pos().x() == curScreenRect.x() && mainWindowsList[i]->pos().y() == curScreenRect.y()){

                    mainWindowsList[i]->hide();
                    mainWindowsList[i]->setGeometry(prvScreenRect);
                    mainWindowsList[i]->move(QPoint(prvScreenRect.x(), prvScreenRect.y()));
                    mainWindowsList[i]->qScreen = prevQScreen;

                    mainWindowsList[i]->setOtherBackgrounds(screenImage,false, true);
                    moveForms(currentScreen);


                    mainWindowsList[i]->show();


                }

            }

        }else{
        }

    }

    if(mirrored && QGuiApplication::screens().length() > 1)
        mainWindowsList[1]->hide();

    return false;
}



void MainWindow::receiveKeyboardRequest(QPoint from, int width){

    if(checkTouchScreen())
        return;


    int middlepoint = from.x() + (width / 2);
    int keyboard_width = screenKeyboard->width();
    int keyboard_height = screenKeyboard->height();

    QRect screenRect = QGuiApplication::primaryScreen()->geometry();


    if(screenRect.width() <= 640){

        keyboard_width = 400;
        keyboard_height = 170;

    }else if(screenRect.width() <= 800){

        keyboard_width = 500;
        keyboard_height = 190;

    }else if(screenRect.width() <= 1024){

        keyboard_width = 600;
        keyboard_height = 250;

    }

    screenKeyboard->setGeometry(middlepoint - (keyboard_width / 2), from.y() + 150,  keyboard_width, keyboard_height);

    from.setX(middlepoint -  (keyboard_width / 2));
    from.setY(from.y() + 150);


    if(from.y() + keyboard_height > screenRect.height() ){

        from.setY( screenRect.height() - keyboard_height);
    }

    screenKeyboard->move(from);

    screenKeyboard->show();

}


int MainWindow::checkTouchScreen(){

    FILE *fp;
    char data[128];
    bool readerror = false;
    QString  tmpstring;
    QString outstr;
    int read_size;
    QString cachedlayout;
    int ret = 1;

    tmpstring = "";

    fp = popen("udevadm info --export-db | grep ID_INPUT_TOUCHSCREEN=1", "r");
    if (fp == NULL) {
        qWarning() << "Current Keyboard layout can not be read" ;
        readerror = true;
    }

    if(readerror == false){

        read_size = fread(data, 1, sizeof(data), fp);

        if( read_size < 2){
            ret = 1;
        }else{
            ret = 0;
        }



    }

    /* close */
    pclose(fp);
    return ret;
}

void MainWindow::receiveKeyboardClose(){

    screenKeyboard->close();

    emit keyboardClosed();

}

void MainWindow::sendKeyPress(QString key){

    emit sendKeytoChilds(key);

}


void MainWindow::checkNetwork(){

}


void MainWindow::receiveNetworkStatus(bool connected){


    if(QGuiApplication::screens().length() > 1 && mainWindowsList[1] != NULL && mainWindowsList[0] != NULL){
        if(mainWindowsList[1]->pos().x() == mainWindowsList[0]->pos().x()){
            mainWindowsList[1]->hide();
            mirrored = 1;
        }
    }

    emit sendNetworkStatustoChilds(connected);

    if(formshidden){
        hideForms();
    }


}


void MainWindow::keyboardInit(){


#ifdef SCREENKEYBOARD

    if(Settings().screenkeyboardenabled().compare("y") != 0){
        return;
    }

    //screen keyboard

    screenKeyboard = new Keyboard(this);
    screenKeyboard->setKeyboardLayout(m_SettingsForm->current_layout);


    screenKeyboard->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowSystemMenuHint | Qt::WindowDoesNotAcceptFocus);
    screenKeyboard->close();

    connect(screenKeyboard, &Keyboard::sendKey, this, &MainWindow::sendKeyPress);
    connect(this, &MainWindow::sendKeytoChilds, m_LoginForm,&LoginForm::keyboardEvent);

    connect(screenKeyboard, &Keyboard::sendCloseEvent, this, &MainWindow::receiveKeyboardClose);
    connect(this, &MainWindow::keyboardClosed, m_LoginForm,&LoginForm::keyboardCloseEvent);

    connect(m_LoginForm, &LoginForm::sendKeyboardRequest, this, &MainWindow::receiveKeyboardRequest);

    connect(m_LoginForm, &LoginForm::sendKeyboardCloseRequest, this, &MainWindow::receiveKeyboardClose);
    connect(m_SettingsForm, &SettingsForm::sendKeyboardLayout, screenKeyboard, &Keyboard::setKeyboardLayout);

#endif

}

void MainWindow::showForms(void){

    QPalette palette;
    QRect   rect;
    QImage tmpimage;


    if(!m_LoginForm){
        return;
    }


    if(!formshidden)
        return;

    if(m_LoginForm){
        m_LoginForm->showAll();
        // m_LoginForm->setFocus(Qt::OtherFocusReason);
    }

    if(m_ClockForm)
        m_ClockForm->show();

    if(m_PowerForm)
        m_PowerForm->show();

    if(m_SettingsForm)
        m_SettingsForm->show();

    if(!formHideTimer)
        return;

    formHideTimer->stop();
    formHideTimer->start();
    formshidden = 0;

    if( m_LoginForm != NULL ){


        rect = qScreen->geometry();
        QGraphicsOpacityEffect *blur = new QGraphicsOpacityEffect;
        blur->setOpacity(0.5);

        tmpimage = resizeImage(rect, *screenImage);
        QImage result = applyEffectToImage(tmpimage, blur, 0);

        QBrush brush(result);
        palette.setBrush(this->backgroundRole(), brush);
        this->setPalette(palette);
    }
}


void MainWindow::hideForms(void){

    QPalette palette;
    QRect   rect;
    QImage tmpimage;

    if(!m_LoginForm){
        return;
    }

    rect = qScreen->geometry();
    tmpimage = resizeImage(rect, *screenImage);
    QBrush brush(tmpimage);
    palette.setBrush(this->backgroundRole(), brush);
    this->setPalette(palette);
    repaint();


    if(m_LoginForm){
        //  m_LoginForm->clearFocus();
        m_LoginForm->hideAll();
        //m_LoginForm->setVisible(false);

    }


    if(m_SettingsForm){
        m_SettingsForm->hide();
    }



    if(m_ClockForm){
        m_ClockForm->clearFocus();
        m_ClockForm->hide();
    }
    if(m_PowerForm){
        m_PowerForm->clearFocus();
        m_PowerForm->hide();
    }

    formshidden = 1;

}




void MainWindow::backgroundTimerCallback(void){
    setMainBackground(true);


    for (int i = 0; i < QGuiApplication::screens().length(); i++){
        mainWindowsList[i]->setOtherBackgrounds(screenImage, true, false);
    }

}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //showForms();
    QWidget::keyPressEvent(event);
}
void MainWindow::resetHideFormsTimer(void){

    formHideTimer->stop();
    formHideTimer->start();
}

