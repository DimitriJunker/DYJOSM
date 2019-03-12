// osm.cpp: Implementierung der Klasse osm.
//
//////////////////////////////////////////////////////////////////////

// 1.08.16 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.04a'
// Quellen kommen und gehen
/*
 * 25.12.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.07f'
 * Konnte ein Tile nicht geladen werden wurde bisher versucht es durch ein Tile
 * der Standardquelle zu ersetzen, dies brachte aber nicht viel und war nur aufwendig.
 *
 * 29.12.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.07g'
 * viele URLs die früher mit http begannen beginnen jetzt mit https
 *
 * 25.12.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.07h'
 * andere kleine Bugs behoben

*/

#include "cosm.h"
#include <fstream>
#include <direct.h>
#include "sys/stat.h"
#include "math.h"
#include "io.h"
#include <urlmon.h>
#include <new.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include "urldownload.h"
#include <qtextstream.h>
#include <qdir.h>
#include <qinputdialog.h>

//#pragma comment ( lib, "wininet.lib")


//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
COsm::COsm(int zoom,QString tilesource,QString tileExt,QString tahCache,UINT maxCacheDays)

{
    m_zoom=zoom;
    m_tilesource=tilesource;
    m_tileExt=tileExt;
    m_tahCache=tahCache;
    m_maxCacheDays=maxCacheDays;
}

COsm::~COsm()
{

}

#define SUCCESS 0
#define INVALID_PATH 1
#define ACCESS_DENIED 2
#define DISK_FULL 3
#define INVALID_URL 4
#define FILE_NOT_FOUND 5


// Rückgabe 0: OK
//			1: konnte nicht geladen werden
//			2: konnte nicht geladen werden aber altes vorhanden
//			3: y nicht im erlaubten Bereich -> schwarz

int COsm::getsaveTileWithCache(int x, int y,QString &file,QString *pErrTxt,QString addExt)
{
    QString path2,oldFile="";
    int ret=1;
    int anzTilesInZoom=1<<m_zoom;	//CHG: DYJTracker 1.01a DYJ
    while(x<0)
        x+=anzTilesInZoom;
    while(x>=anzTilesInZoom)
        x-=anzTilesInZoom;
    if(y<0||y>=anzTilesInZoom)
        return 3;
    if(m_tahCache.endsWith('-'))    //QT5 offlinemaps
    {
        QFileInfo fi(m_tahCache);
        path2=fi.path();
        file=QString("%1%2-%3-%4%5").arg(m_tahCache).arg(m_zoom).arg(x).arg(y).arg(m_tileExt);
    }
    else
    {
        path2=QString("%1%2/%3").arg(m_tahCache).arg(m_zoom).arg(x);
        file=QString("%1/%2%3%4").arg(path2).arg(y).arg(m_tileExt).arg(addExt);
        QDir dirH;
        dirH.mkpath(path2);
    }
    if(pErrTxt)
        *pErrTxt="";
    QFileInfo fii(file);

    if(fii.exists())	// prüfen ob das File bereits besteht
    {
        qint64 alter=(QDateTime::currentDateTime().toMSecsSinceEpoch()-fii.lastModified().toMSecsSinceEpoch())/1000;
        if(alter>(3600*24*m_maxCacheDays) || fii.size()< 100  )
        {
            oldFile=file+"_old";
            QFile fi(file);
            fi.rename(oldFile);
        }
        else
            return 0;

    }
    QString url;
    bool ok=FALSE;
    QString sUrl,sX,sY,sZ;
    sX=QString("%1").arg(x);
    sY=QString("%1").arg(y);
    sZ=QString("%1").arg(m_zoom);
    sUrl=m_tilesource;
    if(sUrl.contains("$X",Qt::CaseInsensitive) ||sUrl.contains("{X}",Qt::CaseInsensitive))
    {
        sUrl.replace("$X",sX,Qt::CaseInsensitive);
        sUrl.replace("${X}",sX,Qt::CaseInsensitive);
        sUrl.replace("{X}",sX,Qt::CaseInsensitive);
        if(sUrl.contains("$Y",Qt::CaseInsensitive) ||sUrl.contains("{Y}",Qt::CaseInsensitive))
        {
            sUrl.replace("$Y",sY,Qt::CaseInsensitive);
            sUrl.replace("${Y}",sY,Qt::CaseInsensitive);
            sUrl.replace("{Y}",sY,Qt::CaseInsensitive);
            if(sUrl.contains("$Z",Qt::CaseInsensitive) ||sUrl.contains("{Z}",Qt::CaseInsensitive))
            {
                sUrl.replace("$Z",sZ,Qt::CaseInsensitive);
                sUrl.replace("${Z}",sZ,Qt::CaseInsensitive);
                sUrl.replace("{Z}",sZ,Qt::CaseInsensitive);
                sUrl.replace("$$","$",Qt::CaseInsensitive);
                ok=TRUE;
            }
        }
    }
    if(ok)
    {
        if(sUrl.contains("{ID_",Qt::CaseInsensitive))
        {
            int indx=sUrl.indexOf("{ID_");
            QString idName=sUrl.mid(indx+1);
            indx=idName.indexOf("}");
            if(indx>-1)
            {
                idName.truncate(indx);
                QString idValue=m_osmIDs.value(idName,"");
                if(idValue.isEmpty())
                   return(1);
                QString idTag="{"+idName+"}";
                sUrl.replace(idTag,idValue,Qt::CaseInsensitive);

            }
        }
        url=sUrl;
    }
    else
        url=QString("%1/%2/%3/%4%5").arg(m_tilesource).arg(m_zoom).arg(x).arg(y).arg(m_tileExt);
    QString urlStart;
    static char startwerta='a',startwert1='1';
    if(url.startsWith("http://a."))
    {
        startwerta++;
        if(startwerta=='d')
            startwerta='a';
        url.replace(7,1,startwerta);
    }
    else if(url.startsWith("http://otile1"))
    {
        startwert1++;
        if(startwert1=='4')
            startwert1='1';
        url.replace(12,1,startwert1);
    }
    else if(url.startsWith("https://a."))
    {
        startwerta++;
        if(startwerta=='d')
            startwerta='a';
        url.replace(8,1,startwerta);
    }
    else if(url.startsWith("https://otile1"))
    {
        startwert1++;
        if(startwert1=='4')
            startwert1='1';
        url.replace(13,1,startwert1);
    }
    urlStart=url;
    while(ret)
    {
        if((ret=urlDownload::downloadFile(url, file))==0)
        {
            QFileInfo fii(file);
            if(fii.exists())	// prüfen ob das File OK sein könnte
            {
                if(fii.size()< 100  )
                {
                    ret=1;
                    QFile fi(file);
                    fi.remove();
                    if(pErrTxt)
                        *pErrTxt=QString("\tURLDownloadToFile(%1,%2) <100Byte<br>\r\n").arg(url).arg(file);
                }
                else
                {
                    QFile pngFile(file);
                    if(pngFile.open(QIODevice::ReadOnly))
                    {
                        bool isJpgOrPng=false;
                        char head[22]="";
                        if(!file.right(4).compare(".png",Qt::CaseInsensitive))
                        {
                            char pngHead[]={-119,80,78,71,13,10,26,10};
                            pngFile.read(head,20);
                            pngFile.close();
                            isJpgOrPng=(memcmp(head,pngHead,8)==0);
                       }
                        if(!file.right(4).compare(".jpg",Qt::CaseInsensitive))
                        {
                            char jpgHead[]={-1,-40};
                            pngFile.read(head,20);
                            pngFile.close();
                            isJpgOrPng=(memcmp(head,jpgHead,2)==0);

                        }
                        if(isJpgOrPng)
                        {
                            ret=0;
                            if(pErrTxt)
                                    *pErrTxt="";
                        }
                        else
                        {
                            bool isHtml=false;
                            QString sHead=head,fileHtml="";
                            isHtml=sHead.indexOf("<!DOCTYPE HTML",0,Qt::CaseInsensitive)!=-1 ||sHead.indexOf("<HTML",0,Qt::CaseInsensitive)!=-1;
                            QFile fi(file);
                            if(isHtml)
                            {
                                QFileInfo alt(file);
                                QString name=alt.baseName()+".html";
                                QFileInfo neu(alt.dir(),name);
                                fileHtml=neu.filePath();
                                isHtml=fi.rename(fileHtml);
                            }

                            ret=1;
                            if(isHtml)
                            {
                                if(pErrTxt)
                                {
                                    *pErrTxt=QString(QObject::tr("\tURLDownloadToFile(%1,%2) liefert ein <a href=\"file:///%3\">html-File</a> (Fehlermeldung?)<br>\r\n")).arg(url).arg(file).arg(fileHtml);
                                    *pErrTxt+=QString("<!--  <tmpfile>%1</tmpfile> -->\r\n").arg(fileHtml);
                                }
                            }
                                else
                            {
                                fi.remove();
                                if(pErrTxt)
                                    *pErrTxt=QString(QObject::tr("\tURLDownloadToFile(%1,%2) kein png oder jpg<br>\r\n")).arg(url).arg(file);
                            }
                        }
                    }
                }
            }
            else
            {
                ret=1;
                if(pErrTxt)
                    *pErrTxt=QString("\tURLDownloadToFile(%1,%2) -> FileNotFound<br>\r\n").arg(url,file);
            }
        }
        else if(pErrTxt)
            *pErrTxt=QString("\tURLDownloadToFile(%1,%2) -> 0x%3<br>\r\n").arg(url).arg(file).arg(QString::number(ret,16));
        if(ret)
        {
            ret=1;
            if(url.startsWith("http://a."))
                url.replace(7,1,'b');
            else if(url.startsWith("http://b."))
                url.replace(7,1,'c');
            else if(url.startsWith("http://c."))
                url.replace(7,1,'a');
            else if(url.startsWith("http://otile1"))
                url.replace(12,1,'2');
            else if(url.startsWith("http://otile2"))
                url.replace(12,1,'3');
            else if(url.startsWith("http://otile3"))
                url.replace(12,1,'4');
            else if(url.startsWith("http://otile4"))
                url.replace(12,1,'1');
            else if(url.startsWith("https://a."))
                url.replace(8,1,'b');
            else if(url.startsWith("https://b."))
                url.replace(8,1,'c');
            else if(url.startsWith("https://c."))
                url.replace(8,1,'a');
            else if(url.startsWith("https://otile1"))
                url.replace(13,1,'2');
            else if(url.startsWith("https://otile2"))
                url.replace(13,1,'3');
            else if(url.startsWith("https://otile3"))
                url.replace(13,1,'4');
            else if(url.startsWith("https://otile4"))
                url.replace(13,1,'1');
            else
                break;
            if(!url.compare(urlStart))
                break;
        }
    }
    if(!oldFile.isEmpty())
    {
        QFile fi(oldFile);
        if(ret)
        {
            fi.rename(file);
            ret=2;		// Altes File behalten
            if(pErrTxt)
                *pErrTxt+="\r\n\t\tOld file kept<br>\r\n";
        }
        else
            fi.remove();
    }
    return (ret);
}


//##############################################################################
//# transform the latitude and logitude to the OSM tile number -> needed for tile-download
//##############################################################################
QPoint COsm::getTileNumber(double lat, double lon,int zoom)
{
    QPoint ret;
    ret.setX(int( (lon+180)/360 * (1<<zoom) )) ;
    ret.setY(int( (1 - log(tan(lat*PI/180) + 1/cos(lat*PI/180))/PI)/2 * (1<<zoom) )) ;
  return(ret);
}
/*
int COsm::mkdirRec(const QDir &di)
{
    QString dirname=di.path();
    int ret=_mkdir(dirname.toLatin1());
    if(ret)
    {
        ret=errno;
        if(ret==ENOENT)
        {
            QFileInfo fi(dirname);
            QString parent=fi.path();
            if(parent.size()>2)    // nicht c:
            {
                ret=mkdirRec(parent);
                if(!ret)
                    ret=_mkdir(dirname.toLatin1());
            }
        }
        else if(ret==EEXIST)
            ret=0;
    }
    return ret;

}

void COsm::mkdirRec(QString dirname,bool isFilename)
{

    if(isFilename)
    {
        QFileInfo fi(dirname);
        dirname=fi.path();
    }

    QDir dir;
    dir.mkpath(dirname);

}
*/


void COsm::convGpxAscShrt(QString zielTxt, QString zielGps, QString quell, bool conv2asc,QString creator)
{
    QFile fileT(zielTxt),fileG(zielGps),fileQ(quell);
    QString wptF,wpt;
    bool writeTxt=FALSE;
    if(fileG.open( QIODevice::WriteOnly)==0 ||  fileQ.open(QIODevice::ReadOnly)==0)
        return;
    if(!zielTxt.isEmpty())
    {
        writeTxt=fileT.open(QIODevice::WriteOnly)!=0;
//int a=	fileT.Open(zielTxt,CFile::modeCreate);
//writeTxt =a!=0;
    }
    if(conv2asc)
    {
        wpt=QString("; This Ascii GPS was generated by %1\r\n").arg(creator);
        if(writeTxt)
            wptF="%1, %2, \"OSMBug #%3\"\r\n";
        else
            wptF="%1, %2, \"%3 Closed=%4 ID=%5\"\r\n";
    }
    else
    {
        wpt="<?xml version=\"1.0\" encoding=\"ISO-8859-1\" standalone=\"no\" ?>\r\n<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" creator=\"";
        wpt+=creator+"\" version=\"1.1\"\r\nxmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\r\n";
        wpt+="xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\r\n";
        wptF="<wpt lon=\"%1\" lat=\"%2\"><desc><![CDATA[OSMBug #%3]]></desc></wpt>\r\n";
    }
    fileG.write(wpt.toLatin1());
    QString gpxFile;
    QTextStream in(&fileQ);
    gpxFile = in.readAll();
    fileQ.close();
    int pos=-1,bugNr=1;
    while((pos=gpxFile.indexOf("<wpt",pos+1))!=-1)
    {
        QString lat,lon,desc,closed,id;
        lat=gpxFile.mid(gpxFile.indexOf("lat=",pos)+5,20);
        lat=lat.left(lat.indexOf('\"'));
        lon=gpxFile.mid(gpxFile.indexOf("lon=",pos)+5,20);
        lon=lon.left(lon.indexOf('\"'));
        desc=gpxFile.mid(gpxFile.indexOf("<desc><![CDATA[",pos)+15);
        desc=desc.left(desc.indexOf("]]></desc>"));
        closed=gpxFile.mid(gpxFile.indexOf("<closed>",pos)+8,1);
        id=gpxFile.mid(gpxFile.indexOf("<id>",pos)+4,30);
        id=id.left(id.indexOf("</id>"));

        if(writeTxt)
        {
            wpt=QString("OSMBug #%1:%2 Closed:%3 ID:%4\r\n").arg(bugNr).arg(desc).arg(closed).arg(id);
            fileT.write(wpt.toLatin1());
            wpt=wptF.arg(lon).arg(lat).arg(bugNr++);
        }
        else
            wpt=wptF.arg(lon).arg(lat).arg(desc).arg(closed).arg(id);
        fileG.write(wpt.toLatin1());

    }
    if(!conv2asc)
    {
        wpt="</gpx>";
        fileG.write(wpt.toLatin1());
    }
    fileG.close();
    if(writeTxt)
        fileT.close();

}


QString COsm::URLDownloadToStr(QString &url)
{
    QString tmpF=tmpnam( nullptr ),ret="";

    if(urlDownload::downloadFile(url, tmpF)==0)
    {
        QFile fTmp(tmpF);
        if(fTmp.open(QIODevice::ReadOnly)==0)
            return ret;

        QTextStream in(&fTmp);
        ret = in.readAll();	 //ganzes File laden	//CHG: TAHO 2.10a SG
        fTmp.close();
        fTmp.remove();
    }
    return ret;
}


int COsm::getsaveVTileWithCache(int nr, QString &file, QString *pErrTxt)
{
    QString oldFile="";
    int ret=1;
//http://openstreetmap.teddynetz.de/latest/img/63273/63273621.img.gz
//	path2.Format("%s%d",m_tahCache,fldNr);
    file=QString("%1/%2.img.gz").arg(m_tahCache).arg(nr);
    file.replace("//","/");
    QDir dirH;
    dirH.mkpath(m_tahCache);
    if(pErrTxt)
        *pErrTxt="";
    QFileInfo fii(file);

    if(fii.exists())	// prüfen ob das File bereits besteht
    {
        qint64 alter=(QDateTime::currentDateTime().toMSecsSinceEpoch()-fii.lastModified().toMSecsSinceEpoch())/1000;
        if(alter>(3600*24*m_maxCacheDays) ||fii.size()< 100  )
        {
            oldFile=file+"_old";
            QFile fi(file);
            fi.rename(oldFile);
        }
        else
            return 0;

    }
    QString url;
    url=QString("%1/%2.img.gz").arg(m_tilesource).arg(nr);
    if((ret=urlDownload::downloadFile(url, file))==0)
    {
        QFileInfo fii(file);
        if(fii.exists())	// prüfen ob das File OK sein könnte
        {
            ret=0;
            if(pErrTxt)
                *pErrTxt="";
        }
        else
        {
            ret=1;
            if(pErrTxt)
                *pErrTxt=QString("\tURLDownloadToFile(%1,%2) -> FileNotFound\r\n").arg(url).arg(file);
        }
    }
    else if(pErrTxt)
        *pErrTxt=QString("\tURLDownloadToFile(%1,%2) -> 0x%3\r\n").arg(url).arg(file).arg(QString::number(ret,16));
    if(!oldFile.isEmpty())
    {
        QFile fi(oldFile);
        if(ret)
        {
            fi.rename(file);
            ret=2;		// Altes File behalten
            if(pErrTxt)
                *pErrTxt+="\r\n\t\tOld file kept\r\n";
        }
        else
            fi.remove();
    }
    return (ret);

}

void COsm::rmdirRec(QString dirname)
{
    QDir dir(dirname);
    if(dir.exists())
    {
        Q_FOREACH(QFileInfo subDir, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllDirs))
            rmdirRec(subDir.absoluteFilePath());
        Q_FOREACH(QFileInfo file, dir.entryInfoList(QDir::Files))
            QFile::remove(file.absoluteFilePath());
        dir.rmdir(dirname);
    }

}
