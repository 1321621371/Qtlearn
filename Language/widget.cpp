#include "widget.h"
#include "ui_widget.h"
#include <QFont>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setFixedSize(800,480);
    //extraLight
    ui->label_4->setFont(QFont("思源黑体 CN",21,QFont::ExtraLight));
    ui->label_5->setFont(QFont("思源黑体 CN",16,QFont::ExtraLight));
    ui->label_6->setFont(QFont("思源黑体 CN",12,QFont::ExtraLight));
    //light
    ui->label_7->setFont(QFont("思源黑体 CN",21,QFont::Light));
    ui->label_8->setFont(QFont("思源黑体 CN",16,QFont::Light));
    ui->label_9->setFont(QFont("思源黑体 CN",12,QFont::Light));
    //normal
    ui->label_10->setFont(QFont("思源黑体 CN",21,QFont::Normal));
    ui->label_11->setFont(QFont("思源黑体 CN",16,QFont::Normal));
    ui->label_12->setFont(QFont("思源黑体 CN",12,QFont::Normal));
    //media
    ui->label_13->setFont(QFont("思源黑体 CN",21,QFont::Medium));
    ui->label_14->setFont(QFont("思源黑体 CN",16,QFont::Medium));
    ui->label_15->setFont(QFont("思源黑体 CN",12,QFont::Medium));
    //bold
    ui->label_16->setFont(QFont("思源黑体 CN",21,QFont::Bold));
    ui->label_17->setFont(QFont("思源黑体 CN",16,QFont::Bold));
    ui->label_18->setFont(QFont("思源黑体 CN",12,QFont::Bold));


}

Widget::~Widget()
{
    delete ui;
}

