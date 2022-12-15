#include "testbeep.h"
#include "ui_testbeep.h"
#include <QDebug>

testBeep::testBeep(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::testBeep)
{
    ui->setupUi(this);
}

testBeep::~testBeep()
{
    delete ui;
}

void testBeep::on_pushButtonOn_clicked()
{
    QString   str;
    QByteArray ba;
    str = QString("echo 1 > /sys/module/tsc2007/parameters/beep_on");
    ba = str.toLatin1();
    // qDebug() << ba ;
    ::system(ba);
}


void testBeep::on_pushButtonOff_clicked()
{
    QString   str;
    QByteArray ba;
    str = QString("echo 0 > /sys/module/tsc2007/parameters/beep_on");
    ba = str.toLatin1();
    // qDebug() << ba ;
    ::system(ba);
}


void testBeep::on_pushButton_clicked()
{
    this->hide();
}

