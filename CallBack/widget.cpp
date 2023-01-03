#include "widget.h"
#include "ui_widget.h"
//使用call模块，并监听回调函数
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    m_call = new call(this);
    m_call->setCallbackFunc(getPoint);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::getPoint(QPoint point)
{

    qDebug()<< point;
}
