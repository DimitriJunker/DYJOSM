#ifndef COSM_H
#define COSM_H

#include <qpoint.h>
#include <qstring.h>
#include <qlist.h>
#include <QDir>
#define PI 3.1415926535897931


struct RGBCELL
{
    unsigned char m_rgb[3]; // A pixel
};

class COsmTag
{
public:
    QString m_value;
    QString m_key;
};
typedef QList<COsmTag>   COsmTagList;

class COsmObj
{
public:
    COsmTagList m_tags;
    bool m_visible;
    int m_uid;
    QString m_user;
    int m_changeset;
    int m_version;
    unsigned int m_id;
};
class COsmNode: public COsmObj
{
public:
    double m_lon;
    double m_lat;
};
class COsmWay: public COsmObj
{
public:
};
class COsm
{
public:
    static void rmdirRec(QString dirname);
    int getsaveVTileWithCache(int nr,QString &file,QString *pErrTxt);
    static QString URLDownloadToStr(QString &url);
    static void convGpxAscShrt(QString zielTxt,QString zielGps,QString quell,bool conv2asc,QString craetor);
    static QPoint getTileNumber(double lat, double lon, int zoom);
    int getsaveTileWithCache(int x, int y, QString &file, QString *pErrTxt=nullptr, QString addExt="");
    COsm(int zoom,QString tilesource,QString tileExt,QString tahCache,unsigned int maxCacheDays);
    virtual ~COsm();
    static QMap<QString,QString> m_osmIDs;

private:
    QString m_tahCache;
    QString m_tilesource;
    QString m_tilesourceStd;
    QString m_tileExt;
    int m_zoom;
    unsigned int m_maxCacheDays;
};


#endif // COSM_H
