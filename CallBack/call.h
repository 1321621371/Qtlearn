#ifndef CALL_H
#define CALL_H

#include <QObject>
#include <QTimer>
#include <QPoint>
#include <Windows.h>

class call : public QObject
{
    Q_OBJECT
public:
    explicit call(QObject *parent = 0);
    void setCallbackFunc(void(*func)(QPoint point));

private:
    QTimer* timer;
    void(*m_callbackFunc)(QPoint point);
signals:

public slots:
    void timerout();
};

#endif // CALL_H
