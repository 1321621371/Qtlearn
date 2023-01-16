#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    QString getMid(QString src, QString start, QString end);
    ~Widget();

private slots:

    void on_btn7_clicked();

    void on_btn8_clicked();

    void on_btn9_clicked();

    void on_btn4_clicked();


    void on_btn6_clicked();

    void on_btn1_clicked();

    void on_btn2_clicked();

    void on_btn3_clicked();

    void on_btn0_clicked();

    void on_btndot_clicked();

    void on_btndel_clicked();

    void on_pushButton_clicked();

    void on_btn5_clicked();



private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
