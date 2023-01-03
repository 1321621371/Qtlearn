#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QProcess>
#include<QMessageBox>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    process= new QProcess(this);
    connect(process,&QProcess::readyReadStandardOutput,this,&Widget::readoutput);
    connect(process,&QProcess::readyReadStandardError,this,&Widget::readerror);
}

Widget::~Widget()
{
    delete ui;
    if(process)
    {
        process->close();
        process->waitForFinished();
    }
}

void Widget::on_testButton_clicked()
{
   system("gnome-terminal -- bash -c '/home/xx/myssh/my.sh'&");//chmod a+x /home/xx/my.sh;
}

void Widget::on_pushButton2_clicked()
{
   QProcess::startDetached("/home/xx/myssh/bash.sh");//以分离进程的方式启动 没有子父进程关系
}

void Widget::on_pushButton_2_clicked()
{
    process->start("bash");                      //启动终端(Windows下改为cmd)
    process->waitForStarted();                   //等待启动完成
    process->write("/home/xx/myssh/my.sh\n");    //向终端写入命令，注意尾部的“\n”不可省略
    process->write("ifconfig\n");

}

void Widget::readoutput()
{
     ui->textEdit->append(process->readAllStandardOutput().data());   //将输出信息读取到编辑框

}

void Widget::readerror()
{
     QMessageBox::information(0, "Error", process->readAllStandardError().data());   //弹出信息框提示错误信息
}
