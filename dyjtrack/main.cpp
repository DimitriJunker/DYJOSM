#include "dyjmainw.h"
#include <QApplication>
#include <QTranslator>
#include"qdebug.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    dytMainW w;
    QString lang=QLocale::system().name();
    if(lang.left(2)!="de")
    {
        if(lang.left(2)=="fr")
            qDebug()<< w.m_language.load("dyjtrack_fr");
        else
            w.m_language.load("dyjtrack_en");
        a.installTranslator(&w.m_language);
    }
    w.show();

    return a.exec();
}
