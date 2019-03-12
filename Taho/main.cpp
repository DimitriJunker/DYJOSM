#include "tahomainw.h"
#include "tahostyle.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(new TahoStyle);

    TahoMainW w;
    QString lang=QLocale::system().name();
    if(lang.left(2)!="de")
    {
        if(lang.left(2)=="fr")
        {
            w.m_language.load("taho_fr");
        }
        else
        {
            w.m_language.load("taho_en");
        }
        a.installTranslator(&w.m_language);
    }
    w.show();

    return a.exec();
}
