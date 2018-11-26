#ifndef CLOCKFORM_H
#define CLOCKFORM_H

#include <QWidget>
#include <QTimer>

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

private:
    Ui::clockForm *ui;
    QTimer *timer;
    void initialize();
};

#endif // CLOCKFORM_H
