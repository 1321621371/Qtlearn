#ifndef TESTTOUCH_H
#define TESTTOUCH_H

#include <QDialog>

namespace Ui {
class testTouch;
}

class testTouch : public QDialog
{
    Q_OBJECT

public:
    explicit testTouch(QWidget *parent = nullptr);
    ~testTouch();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();



    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::testTouch *ui;
    int m_num;
    int m_num2;
};

#endif // TESTTOUCH_H
