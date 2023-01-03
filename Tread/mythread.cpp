#include "mythread.h"
#include <QThread>
#include <QDebug>

mythread::mythread(QObject *parent) : QObject(parent)
{
    isAction = true;
}


void mythread::Mytimer(){

    while(isAction)
    {
        QThread :: sleep(1);
        qdebug()<<"Mytimer被触发了";
        emit Mysignal();  // 触发信号，穿给主程序的处理函数deelsignal
    }



}
