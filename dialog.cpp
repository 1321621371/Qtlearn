#include "dialog.h"
#include "ui_dialog.h"
#include <QDebug>

#define Max 255
#define  NSINGLESTEP   5
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    this->setFixedSize(600,380);
   // QTextCodec *codec = QTextCodec::codecForName("UTF-8");//添加编码格式
  //  ui->label->setText(codec->toUnicode("背光等级")); //使用方法
    ui->horizontalSlider->setRange(0,Max);
    ui->spinBox->setRange(0,Max);
    ui->horizontalSlider->setValue(Max);
    ui->spinBox->setValue(Max);
    ui->horizontalSlider->setPageStep(10);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_clicked()
{
    this->hide();
}


void Dialog::on_horizontalSlider_valueChanged(int value)
{
    qDebug() << "horizontalSlider valueChanged :" << value;
    //n = ui->horizontalSlider->value();
    ui->spinBox->setValue(value);
    str = QString("echo %1 >/sys/class/backlight/backlight/brightness").arg(value);
     ba = str.toLatin1();
    // qDebug() << ba ;
     ::system(ba);
}


void Dialog::on_spinBox_valueChanged(int arg1)
{
    qDebug() << "spinBox valueChanged :" << arg1;
        ui->horizontalSlider->setValue(arg1);
        str = QString("echo %1 >/sys/class/backlight/backlight/brightness").arg(arg1);
        ba = str.toLatin1();
          //qDebug() << ba1 ;
          ::system(ba);
}

