#ifndef TESTBEEP_H
#define TESTBEEP_H

#include <QDialog>

namespace Ui {
class testBeep;
}

class testBeep : public QDialog
{
    Q_OBJECT

public:
    explicit testBeep(QWidget *parent = nullptr);
    ~testBeep();

private slots:
    void on_pushButtonOn_clicked();

    void on_pushButtonOff_clicked();

    void on_pushButton_clicked();

private:
    Ui::testBeep *ui;
};

#endif // TESTBEEP_H
