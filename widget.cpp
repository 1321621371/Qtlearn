#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setFixedSize(800,480);
    ui->touchButton->setGeometry(30,10,160,100);
    ui->backlightButton->setGeometry(210,10,160,100);
    ui->beepButton->setGeometry(390,10,160,100);

}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_backlightButton_clicked()
{
    Dialog *dialog = new Dialog(this);
        dialog->setModal(true);
        dialog->show();
}

