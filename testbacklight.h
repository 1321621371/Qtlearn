#ifndef TESTBACKLIGHT_H
#define TESTBACKLIGHT_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    QString   str;
     QByteArray ba;
    ~Dialog();

private slots:
    void on_pushButton_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_spinBox_valueChanged(int arg1);

private:
    Ui::Dialog *ui;
};

#endif // TESTBACKLIGHT_H
