#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QProcess>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_testButton_clicked();

    void on_pushButton2_clicked();

    void on_pushButton_2_clicked();

    void readoutput();

    void readerror();

private:
    Ui::Widget *ui;
    QProcess *process;
};

#endif // WIDGET_H
