#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>

class mythread : public QObject
{
    Q_OBJECT
public:
    explicit mythread(QObject *parent = 0);
    void Mytimer(); //有效函数
    bool isAction;   //用于控制有效函数MyTimer的进行

signals:
    void Mysignal(); // 连接有效函数（槽）与 主程序的处理函数
public slots:
};

#endif // MYTHREAD_H
