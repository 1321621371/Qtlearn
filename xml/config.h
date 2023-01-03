#ifndef CONFIG_H
#define CONFIG_H
#include <QXmlStreamReader>
#include <QFile>
#include <QApplication>
#include <QDir>


class config
{
public:
    config();
    QString readConfig();
};

#endif // CONFIG_H
