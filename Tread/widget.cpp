#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#define cout qDebug()

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    thread = new QThread(this);
    me = new mythread; // 不可以指定this,否则将于主线程共同进行
    me->moveToThread(thread); // 将自定义线程加入线程
    me->moveToThread(thread);
    connect(me,&mythread::Mysignal,this,&Widget::deelsignal);
    connect(this,&Widget::startThread,me,&mythread::Mytimer);
    // 通过信号starThread->触发MyTimer(槽)->触发Mysignal(信号)->触发deelsignal(使LCD启动计数)
    connect(this,&Widget::destroyed,[=](){// 关闭时结束线程

    me->isAction = false;
    thread->quit();
     //cout<<"关闭"<<endl;
    });

}

void Widget::deelsignal(){ // 信号传输的最终处理

    static int i = 0;
    ++i;
    ui->lcdNumber->display(i); // 设置LCD显示
}


Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    if(me->isAction) return;
    me->isAction = true; // 使自定义线程运行函数MyTimer 进入循环
    thread->start(); // 启动线程，但是并不运行有效函数
    emit startThread();

}

void Widget::on_pushButton_2_clicked()
{
    if(!me->isAction) return;
        me->isAction = false; // 将自定义程序运行函数MyTimer 可以退出循环
        thread->quit(); // 关闭线程
}
