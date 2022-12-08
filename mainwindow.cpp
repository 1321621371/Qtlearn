#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_num = 0;
    m_num2 = 0;
//    this->setCursor(QCursor(Qt::BlankCursor));

    ui->textEdit_2->append("");
    ui->textEdit_2->append("width:" + QString::number(this->width()));
    ui->textEdit_2->append("height:" + QString::number(this->height()));
    ui->textEdit_2->append("x:" + QString::number(this->x()) + " - " + QString::number(this->x() + this->width()));
    ui->textEdit_2->append("y:" + QString::number(this->y()) + " - " + QString::number(this->y() + this->height()));

    ui->textEdit_2->append("");
    ui->textEdit_2->append("width:" + QString::number(ui->checkBox->width()));
    ui->textEdit_2->append("height:" + QString::number(ui->checkBox->height()));
    ui->textEdit_2->append("x:" + QString::number(ui->checkBox->x()) + " - " + QString::number(ui->checkBox->x() + ui->checkBox->width()));
    ui->textEdit_2->append("y:" + QString::number(ui->checkBox->y()) + " - " + QString::number(ui->checkBox->y() + ui->checkBox->height()));

    ui->textEdit_2->append("");
    ui->textEdit_2->append("width:" + QString::number(ui->checkBox_2->width()));
    ui->textEdit_2->append("height:" + QString::number(ui->checkBox_2->height()));
    ui->textEdit_2->append("x:" + QString::number(ui->checkBox_2->x()) + " - " + QString::number(ui->checkBox_2->x() + ui->checkBox_2->width()));
    ui->textEdit_2->append("y:" + QString::number(ui->checkBox_2->y()) + " - " + QString::number(ui->checkBox_2->y() + ui->checkBox_2->height()));

    ui->textEdit_2->append("");
    ui->textEdit_2->append("width:" + QString::number(ui->checkBox_3->width()));
    ui->textEdit_2->append("height:" + QString::number(ui->checkBox_3->height()));
    ui->textEdit_2->append("x:" + QString::number(ui->checkBox_3->x()) + " - " + QString::number(ui->checkBox_3->x() + ui->checkBox_3->width()));
    ui->textEdit_2->append("y:" + QString::number(ui->checkBox_3->y()) + " - " + QString::number(ui->checkBox_3->y() + ui->checkBox_3->height()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    m_num++;
    ui->textEdit->setPlainText("hello " + QString::number(m_num));

}


void MainWindow::on_pushButton_2_clicked()
{
    Dialog *dlg = new Dialog();
    dlg->exec();
    delete dlg;
    dlg = nullptr;
}


void MainWindow::on_pushButton_clicked(bool checked)
{

}


void MainWindow::on_pushButton_3_clicked()
{
}



void MainWindow::on_pushButton_4_clicked()
{
    m_num++;
    ui->textEdit_3->setPlainText("hi " + QString::number(m_num));
}

