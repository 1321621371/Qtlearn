#include "call.h"

call::call(QObject *parent) : QObject(parent)
{
    timer = new QTimer;
    connect(timer,&QTimer::timeout,this,&call::timerout);
    timer->start(1000);
}
void call::setCallbackFunc(void (*func)(QPoint))
{
    m_callbackFunc = func;
}

void call::timerout()
{
    //定时器每秒获取一次当前鼠标坐标
     //函数功能：该函数检取当前光标的句柄。
    POINT point;
    GetCursorPos(&point);
    QPoint poi;
    poi.setX(point.x);
    poi.setY(point.y);
    m_callbackFunc(poi);

}
