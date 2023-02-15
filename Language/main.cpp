#include "widget.h"

#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //加入中文库
   //int id = QFontDatabase::addApplicationFont("/usr/share/fonts/ttf/SourceHanSansCN-Light.otf");

    //int id = QFontDatabase::addApplicationFont("/usr/share/fonts/opentype/hansans/SourceHanSansCN-ExtraLight.otf");
   // QString msyh = QFontDatabase::applicationFontFamilies (id).at(0);
    //QFont font(msyh,10);
    //a.setFont(font);
    Widget w;
    w.show();
    return a.exec();
}
