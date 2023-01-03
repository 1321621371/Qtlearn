#include "widget.h"
#include "ui_widget.h"
#include "config.h"
#include <QDebug>

//在D:\qt\build-xml-Desktop_Qt_5_6_1_MinGW_32bit-Debug\debug\Config  创建Config 文件夹 放入testconfig.xml

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    config config;
       QString str;
       str = config.readConfig();
       qDebug()<<qUtf8Printable(str);  //输出不带双引号，需要用qUtf8Printable转换
       ui->plainTextEdit->appendPlainText("读取XML信息如下:\n");
       ui->plainTextEdit->appendPlainText(str);

}

Widget::~Widget()
{
    delete ui;
}
