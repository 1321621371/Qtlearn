#ifndef TOUCHDIALOG_H
#define TOUCHDIALOG_H

#include <QDialog>

namespace Ui {
class TouchDialog;
}

class TouchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TouchDialog(QWidget *parent = nullptr);
    ~TouchDialog();

private slots:
    void on_pushButton_clicked();

private:
    Ui::TouchDialog *ui;
};

#endif // TOUCHDIALOG_H
