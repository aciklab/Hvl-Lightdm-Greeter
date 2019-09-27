#ifndef CLOCKFORM_H
#define CLOCKFORM_H

#include <QWidget>
#include <QTimer>
#include "main.h"

namespace Ui {
class clockForm;
}

class clockForm : public QWidget
{
    Q_OBJECT

private slots:

    void updateClock();

public:
    explicit clockForm(QWidget *parent = 0);
    ~clockForm();
    bool resized;

private:
    Ui::clockForm *ui;
    QTimer *timer;

};

#endif // CLOCKFORM_H
