#ifndef RIGHTCLICKENABLER_H
#define RIGHTCLICKENABLER_H

#include <QPushButton>
#include <QMouseEvent>


class rightClickEnabler : public QObject
{

    Q_OBJECT

public:
    rightClickEnabler();
    rightClickEnabler(QAbstractButton * button): QObject(button), _button(button){
        button->installEventFilter(this);
    }


protected:
    inline bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonPress)
        {
            auto mouseEvent = (QMouseEvent*)event;
            if (mouseEvent->button() == Qt::RightButton)
                sender();
            //_button->click();

        }
        return false;
    }


Q_SIGNALS:
    void rightclicksignal();

private:
    QAbstractButton* _button;
    void sender();



};

#endif // RIGHTCLICKENABLER_H
