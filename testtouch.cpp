#include "testtouch.h"
#include "ui_testtouch.h"
#include "touchdialog.h"

testTouch::testTouch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::testTouch)
{
    ui->setupUi(this);
    this->setFixedSize(800,480);
    m_num = 0;
    m_num2 = 0;
}

testTouch::~testTouch()
{
    delete ui;
}

void testTouch::on_pushButton_clicked()
{
    m_num++;
    ui->textEdit->setPlainText("hello " + QString::number(m_num));

}


void testTouch::on_pushButton_2_clicked()
{
    m_num2++;
    ui->textEdit_2->setPlainText("hi " + QString::number(m_num2));
}




void testTouch::on_pushButton_3_clicked()
{
    TouchDialog *dlg = new  TouchDialog();
    dlg->exec();
    delete dlg;
    dlg = nullptr;

}


void testTouch::on_pushButton_4_clicked()
{
    this->hide();
}

