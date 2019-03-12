#include "cosmopt.h"
#include "cxmlfile.h"
#include "cosm.h"
#include <qdir.h>
#include "cmapsrc.h"
#include <qstring.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qstandardpaths.h>
#include <QFileDialog>
#include <urldownload.h>
#include <QDate>

QString COsmOpt::m_par;
QString COsmOpt::m_prgPath;
QString COsmOpt::m_persPath;
QString COsmOpt::m_srcPath=m_persPath;

QString COsmOpt::m_tmpDir;


COsmOpt::COsmOpt()
{
    if(!m_persPath.isEmpty())	//dann waren wir schon hier
        return;
//Parameter und Programmpfad
    m_prgPath = qApp->applicationDirPath()+"/";
    QStringList pars=QApplication::instance()->arguments();
    if(pars.size()<2)
        m_par="";
    else
        m_par=pars[1];

//tmp-Pfad
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    if(paths.size())
        m_tmpDir=QString("%1/").arg(paths[0]);
    else
    {
        m_tmpDir=QString("%1tmp/").arg(m_prgPath);
        QDir dirH;
        dirH.mkpath(m_tmpDir);
    }

//Persönlicher -Pfad
    paths = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    if(paths.size())
        m_persPath=QString("%1/DYJ/OSM/").arg(paths[0]);
    else
        m_persPath=m_prgPath;

// Tile-Sourcen laden
    loadTahoSrc(false,2);

}

/*** ------------------------------------------------------------------------ ***/
/*	bool COsmOpt::loadTahoSrc(bool defPathIsPrg, int minVersion)					*/
/*								*/
/*	Parameter:	defPathIsPrg Wenn true sind die Taho-Files im Prg-Ordner, sonst im     */
/*      Dokumente Ordner, ggf werden sie dort hin verschoben                   	*/
//      minVersion minimale Versionsnummer des defsrcP.Taho
/*	Ergebnis:	Erfolgreich?													*/
/*	06.11 von Dimitri Junker	DYJ 4.07d									*/
/*** ------------------------------------------------------------------------ ***/

bool COsmOpt::loadTahoSrc(bool defPathIsPrg, int minVersion)
{
    if(defPathIsPrg)
        m_srcPath=m_prgPath;
    else
        m_srcPath=m_persPath;
    return loadTahoSrc(minVersion);
}
/*** ------------------------------------------------------------------------ ***/
/*	bool COsmOpt::loadTahoSrc(int minVersion)					*/
/*								*/
//	Parameter:	      minVersion minimale Versionsnummer des defsrcP.Taho
/*	Ergebnis:	Erfolgreich?													*/
/*	06.11 von Dimitri Junker	DYJ 4.07d									*/
/*** ------------------------------------------------------------------------ ***/

bool COsmOpt::loadTahoSrc(int minVersion)
{
    static int versuch=0;
    versuch++;
    bool ret=false;
    if((ret=loadTahoSrc(m_srcPath+T_SRC,nullptr,"",minVersion))==false && versuch<2)
        OnUpdSrc();      //Wahrscheinlich der erste Lauf nach der Installation
    versuch=0;
    CMapSrc::m_mySrc=m_persPath+T_MY_SRC;
    loadTahoSrc(CMapSrc::m_mySrc,nullptr,"",minVersion);
    return ret;
}

/*** ------------------------------------------------------------------------ ***/
/*	bool COsmOpt::loadTahoSrc(QString pfad,QString *psTaho,QString errOld, int minVersion)		*/
/*								*/
/*	Parameter:	pfad : Pfad von defsrcP oder mydefsrc							*/
//              psTaho:
//              errOld:
//      minVersion minimale Versionsnummer des defsrcP.Taho
/*	Ergebnis:	erfolgreich?															*/
/*	von Dimitri Junker									*/
/*** ------------------------------------------------------------------------ ***/

bool COsmOpt::loadTahoSrc(QString pfad,QString *psTaho,QString errOld, int minVersion)
{
    QString local,source;
    if(!psTaho)
        psTaho=&local;
    bool ret=false;
    *psTaho="";
    CXmlFile xTaho;
    if(!xTaho.read2Buf(pfad))
        return ret;
    QString versionS,attr;

    if(!xTaho.readValB(*psTaho,"taho",nullptr,&attr) && !errOld.isEmpty())
    {
        QMessageBox::StandardButton oldOpt;
        oldOpt = QMessageBox::question(nullptr, "", errOld,
                                      QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

        if(oldOpt!=QMessageBox::Cancel)
            *psTaho=xTaho.m_sBuf;
    }
//Sources
    versionS=xTaho.readAttrValS(attr,"version");
    float versionI=0.;
    if(!versionS.isEmpty())
        versionI=versionS.toFloat();

    if(!psTaho->isEmpty() && versionI>=minVersion)
        ret=CMapSrc::readTaho(&xTaho);
    return ret;
}


bool COsmOpt::OnUpdSrc()
{
    bool ret=false;
    QString urlOsmTaho="https://wiki.openstreetmap.org/wiki/DE:Taho",urlTaho="";
    QString osmTahoPath=m_tmpDir+"osm_taho.html";
    urlDownload::downloadFile(urlOsmTaho,  osmTahoPath);

    CXmlFile xTaho;
    if(xTaho.read2Buf(osmTahoPath))
    {
        QString sA,fName=T_SRC;
        int pos=0;
        while(!(sA=xTaho.readAttr("a",&pos)).isEmpty())
        {
            QString link=xTaho.readAttrValS(sA,"href");
            if(!link.right(fName.size()).compare(fName,Qt::CaseInsensitive))
            {
                urlTaho=link;
                break;
            }
        }
        if(urlTaho.isEmpty())
        {
            urlTaho="http://www.dimitri-junker.de/";
            urlTaho+=T_SRC;
        }
        urlDownload::downloadFile(urlTaho, m_srcPath+T_SRC);
        ret=loadTahoSrc();
    }
    return ret;
}
