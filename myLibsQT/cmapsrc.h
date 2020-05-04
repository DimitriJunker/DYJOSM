#ifndef CMAPSRC_H
#define CMAPSRC_H

#include <qstring.h>
#include "cxmlfile.h"
#include <qmap.h>
#include <qcombobox.h>
#include <qlistwidget.h>
#include <QFile>

enum {LCC_DES,LCC_SEL,LCC_TOG,LCC_ASK};

enum MapType {MAP_UNDEF=-1,MAP_BAS,MAP_OVR,MAP_VECT,MAP_OSM,MAP_DEF=100};
class CMapSrc
{
public:
    int m_maxThreads;
    double m_kSize;
    int m_mapsPF;
    static QString m_src;
    static QString getSrc();
    static void writeTahoSelected(QTextStream &oFile, MapType type, int nr=0);
    static void select(QString name,int action=LCC_SEL,int nr=0);
    static void resetOvr();
    static CMapSrc * lookup(QString name);
    static void initBoxes(QComboBox * src,QListWidget *ovr=nullptr,QListWidget *vect=nullptr,int nr=0);
    static QListWidget	*m_ovrCtrlP;
    static QListWidget	*m_ovrCtrlV;
    static QComboBox * m_srcBox;
    static void destroy();
    static bool readTaho(CXmlFile *xTaho);
    static QMap<QString,CMapSrc *> m_mapSrc;
    static QString m_mySrc;
    CMapSrc();
    static void writeTaho(QString path, bool onlyPriv);
    static CMapSrc *initP(QString name, QString url,bool isPrivate, QString ext, int maxThreads, QString pref="", unsigned char maxZoom=255, int mapID=1, QString mapDefin="l");
    static CMapSrc *initV(QString name,QString url,bool isPrivate,unsigned char maxThreads,QString pref,int offset,double kSize,int mapsPF);
    bool hatUnbekannteID();
    bool m_useOvrs[2];
    int m_offset;
    unsigned char m_maxZoom;
    MapType m_type;
    QString m_pref;
    QString m_name;
    QString m_url;
    QString m_ext;
    int m_mapID;
    QString m_mapDefin;
    bool m_isPrivate; 

    virtual ~CMapSrc();

};


#endif // CMAPSRC_H
