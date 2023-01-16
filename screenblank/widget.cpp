#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QProcess>
#include <QRegExp>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    QRegExp rx("\\d+");
         QString d;
          int data_d;
          QProcess process;
          process.start("xset -q");
          process.waitForFinished();
          QString result = process.readAllStandardOutput();
            qDebug()<<"result:"<< result;
          QString dis = getMid(result,"timeout:","cycle");
             qDebug()<<"dis:"<< dis;
             rx.indexIn(dis,0);
             d = rx.cap(0);
             data_d = d.toInt();
           qDebug()<<"data_d="<<data_d<<endl;
//           ui->lineEdit->setText(d+"s");
            ui->lineEdit->setText(d+"s");

}

QString Widget::getMid(QString src, QString start, QString end)
{
    int startIndex = src.indexOf(start, 0);
    int endIndex = src.indexOf(end, startIndex);
    QString subStr = src.mid(startIndex, endIndex - startIndex);
    return subStr;

}

Widget::~Widget()
{
    delete ui;
}



void Widget::on_btn7_clicked()
{
    ui->lineEdit->setText(ui->lineEdit->text()+"7");
}


void Widget::on_btn8_clicked()
{
     ui->lineEdit->setText(ui->lineEdit->text()+"8");
}


void Widget::on_btn9_clicked()
{
 ui->lineEdit->setText(ui->lineEdit->text()+"9");
}


void Widget::on_btn4_clicked()
{
ui->lineEdit->setText(ui->lineEdit->text()+"4");
}

void Widget::on_btn5_clicked()
{
    ui->lineEdit->setText(ui->lineEdit->text()+"5");
}

void Widget::on_btn6_clicked()
{
ui->lineEdit->setText(ui->lineEdit->text()+"6");
}


void Widget::on_btn1_clicked()
{
ui->lineEdit->setText(ui->lineEdit->text()+"1");
}


void Widget::on_btn2_clicked()
{
ui->lineEdit->setText(ui->lineEdit->text()+"2");
}


void Widget::on_btn3_clicked()
{
ui->lineEdit->setText(ui->lineEdit->text()+"3");
}


void Widget::on_btn0_clicked()
{
ui->lineEdit->setText(ui->lineEdit->text()+"0");
}


void Widget::on_btndot_clicked()
{
ui->lineEdit->setText(ui->lineEdit->text()+".");
}


void Widget::on_btndel_clicked()
{
    QString text = ui->lineEdit->text();
    ui->lineEdit->setText(text.remove(text.length()-1,1));

}


void Widget::on_pushButton_clicked()
{
    QString str1;
      QByteArray ba;
      QString str =ui->lineEdit->text();
       qDebug()<<"str:" <<str;
      str1 = QString("xset s "+str);
          ba = str1.toLatin1();
          ::system(ba);


}




