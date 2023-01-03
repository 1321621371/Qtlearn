#include "config.h"
#include <QDebug>
config::config()
{

}
//在D:\qt\build-xml-Desktop_Qt_5_6_1_MinGW_32bit-Debug\debug\Config  创建Config 文件夹 放入testconfig.xml

QString config::readConfig()
{

    QFile file;         //需要打开的文件
    QString nodename;   //节点的名字
    QString OutPut;     //最终输出的内容

    file.setFileName(qApp->applicationDirPath()+"/Config/testconfig.xml");   //查找xml文件的路径
    if(!file.exists())
    {
        qDebug() << "testconfig.xml文件不存在";

    }else
    {
        qDebug() << "testconfig.xml文件存在";


    }

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))   //打开文件
        {
            qDebug()<<"打开文件失败!";
        }

    else
       {
           QXmlStreamReader xmlreader(&file);   //读取xml文件的迭代器
           qDebug() << "testconfig.xml文件存在,且打开文件成功!";
           while(!xmlreader.atEnd() || !xmlreader.hasError())   //当文件没有结束且没有出错执行下面的代码
           {
               xmlreader.readNextStartElement();          //找到非根节点下的第一个子节点
               nodename = xmlreader.name().toString();    //第一个子节点Data
               if(nodename == "Data" && xmlreader.isStartElement())    //如果是第一个子节点的开始执行下列代码
               {
                   xmlreader.attributes().value("id").toString();        //读取出附加信息
                   OutPut.append(QString("ID:%1").arg(xmlreader.attributes().value("id").toString()));  //添加到输出的字符串里
                   OutPut.push_back('\n');
                   while(!(nodename == "Data" && xmlreader.isEndElement()))    //如果子节点中的内容没有结束
                   {
                       xmlreader.readNextStartElement();
                       nodename = xmlreader.name().toString();
                       if(nodename == "Number" && xmlreader.isStartElement())
                       {
                           while(!(nodename == "Number" && xmlreader.isEndElement()))
                           {
                               OutPut.append(QString("Numeber:%1").arg(xmlreader.readElementText()));
                               OutPut.push_back('\n');
                           }
                       }
                       else if(nodename == "Name" && xmlreader.isStartElement())
                       {
                           while(!(nodename == "Name" && xmlreader.isEndElement()))
                           {
                               OutPut.append(QString("Name:%1").arg(xmlreader.readElementText()));
                               OutPut.push_back('\n');
                           }
                       }
                       else if(nodename == "email" && xmlreader.isStartElement())
                       {
                           while(!(nodename == "email" && xmlreader.isEndElement()))
                           {
                               OutPut.append(QString("email:%1").arg(xmlreader.readElementText()));
                               OutPut.push_back('\n');
                           }
                       }
                       else if(nodename == "website" && xmlreader.isStartElement())
                       {
                           while(!(nodename == "website" && xmlreader.isEndElement()))
                           {
                                OutPut.append(QString("website:%1").arg(xmlreader.readElementText()));
                                OutPut.push_back('\n');
                                OutPut.append("--------------------------------\n");
                           }
                       }
                   }
               }
           }
           file.close();
           return OutPut;
       }




}
