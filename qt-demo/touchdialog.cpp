#include "touchdialog.h"
#include "ui_touchdialog.h"

TouchDialog::TouchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TouchDialog)
{
    ui->setupUi(this);
}

TouchDialog::~TouchDialog()
{
    delete ui;
}

void TouchDialog::on_pushButton_clicked()
{
    accept();
}

