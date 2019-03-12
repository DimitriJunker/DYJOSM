#ifndef COSMOPT_H
#define COSMOPT_H
#include <qstring.h>
#include <QRadioButton>

#define T_SRC "defsrcP.taho"
#define T_MY_SRC "mydefsrc.taho"

class COsmOpt
{
public:
//2doua    QString m_useragent[3];
    void initSrc();
    QString defPath();
    QString defPath(bool defPathIsPrg);
    bool OnUpdSrc();
    bool loadTahoSrc(QString path,QString *sTaho=nullptr,QString errOld="", int minVersion=0);
    bool loadTahoSrc(bool defPathisPrg, int minVersion=0);
    bool loadTahoSrc(int minVersion=0);
    static QString m_par;
    static QString m_prgPath;
    static QString m_persPath;
    static QString m_srcPath;
    static QString m_tmpDir;
    COsmOpt();
//    virtual ~COsmOpt();

};

#endif // COSMOPT_H
