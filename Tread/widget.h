#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QThread>
#include"mythread.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();



signals:
    void startThread(); // 线程启动信号


private slots:
    void on_pushButton_clicked();  //start
     void deelsignal();  // 最终处理函数
    void on_pushButton_2_clicked();  //stop

private:
    Ui::Widget *ui;
    QThread *thread;
    mythread* me;

};

#endif // WIDGET_H
