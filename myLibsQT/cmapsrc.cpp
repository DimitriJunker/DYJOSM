#include "cmapsrc.h"
#include <qmap.h>
#include <QTextStream>
#include <cosm.h>
#include <QDate>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <cosmopt.h>
// mapSrc.cpp: Implementierung der Klasse mapSrc.
//
    /*neu für DYJ 1.03b */
//////////////////////////////////////////////////////////////////////



/* 20.4.13 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ 1.05i'
    Obsolete Quellen werden ignoriert, derzeit sind dies:
    {"Osmarender","Cycle1","Cycle2"}

 * 13.07.15 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.01d'
Die lonvia Tiles sind umgezogen, um so etwas besser zu behandeln habe ich den Adressen Tilestamps gegeben, damit Taho entscheiden kann welche Adresse aktuell ist (exe oder defsrc)

 * 1.08.16 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.04b'
    Die Verwaltung der obsoleten Quellen überarbeitet, sie stehen jetzt auch im *.taho File

 * 26.12.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.07c'
 * Platzhalter in Quell URLs erweitert, statt $x funktioniert jetzt auch {x} und ${x},
 * Groß/Kleinschreibung ist auch egal. Außerdem gibt es einen neuen Platzhalter für IDs
 * die man bei Quellen bekommt bei denen man sich anmelden muss, wie thunderforest und maptiler.

 * 27.12.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.07d'
 * Die QuellURLs werden jetzt nur noch in den Files defsrcP.taho und mydefsrc.taho gespeichert,
 * nicht mehr im Programm. Dabei merkt sich Taho welche Quellen aus welcher Datei gelesen wurde,
 *  so kann defsrcP.taho bei einem Update einfach überschrieben werden ohne mydefsrc.taho zu
 *  ändern. Damit ist auch "Export Src" Obsolet

 * 28.12.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.07e'
 * Wegen 4.07d muß jetzt auch im Taho File markiert werden welche Quelle als Standard
 * verwende werden soll. Dazu dient Typ=100

*/
//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
QListWidget	*CMapSrc::m_ovrCtrlP;
QListWidget	*CMapSrc::m_ovrCtrlV;
QComboBox * CMapSrc::m_srcBox;
QString CMapSrc::m_mySrc;

QMap<QString,CMapSrc*> CMapSrc::m_mapSrc;

QString CMapSrc::m_src="";
QMap<QString,QString> COsm::m_osmIDs;
QMap<QString,QStringList> COsm::m_osmLogins;

CMapSrc::~CMapSrc()
{

}


CMapSrc::CMapSrc()
{
    m_name="";
    m_url="";
    m_ext="";
    m_pref="";
    m_maxZoom=0;
    m_type=MAP_UNDEF;
    m_offset=0;
    m_useOvrs[0]=m_useOvrs[1]=false;
    m_kSize=0;
    m_mapsPF=0;
}
CMapSrc *CMapSrc::initV(QString name, QString url,bool isPrivate,unsigned char maxThreads, QString pref, int offset,double kSize,int mapsPF)
{
    CMapSrc *ret=nullptr;
    if(offset==-1)	//API -> *.osm
        ret=initP(name,url,isPrivate,"",maxThreads,pref,253);
    else
        ret=initP(name,url,isPrivate,"",maxThreads,pref,254);

     if(ret)
    {
        ret->m_offset=offset;
        ret->m_kSize=kSize;
        ret->m_mapsPF=mapsPF;
    }
    return ret;
}
Q_DECLARE_METATYPE(CMapSrc*)

CMapSrc *CMapSrc::initP(QString name, QString url, bool isPrivate, QString ext, int maxThreads, QString pref, unsigned char maxZoom, int mapID, QString mapDefin)
{
    CMapSrc *ret;
    bool isNew=false;
    QMap<QString,CMapSrc *>::iterator it=m_mapSrc.find(name);
    if(it==m_mapSrc.end())
    {
        ret=new CMapSrc;
        m_mapSrc.insert(name,ret);
        isNew=true;
    }
    else
        ret=it.value();

    ret->m_isPrivate=isPrivate;
    ret->m_name=name;
    ret->m_ext=ext;
    ret->m_url=url;
    ret->m_maxThreads=maxThreads;
    ret->m_mapID=mapID;
    ret->m_mapDefin=mapDefin;
    if(pref.isEmpty())
        ret->m_pref=name.left(3);
    else
        ret->m_pref=pref;
    switch(maxZoom)
    {
    case 255:	//Pixel Overlay
        ret->m_type=MAP_OVR;
        ret->m_maxZoom=0;
        if(isNew && m_ovrCtrlP)
        {
            QListWidgetItem *newItem =new QListWidgetItem(name);
            newItem->setCheckState(Qt::Unchecked);
            newItem->setData(Qt::UserRole,QVariant::fromValue<CMapSrc*>(ret));
            m_ovrCtrlP->addItem(newItem);
            m_ovrCtrlP->sortItems();
        }
        break;
    case 253:	//*.osm
        ret->m_type=MAP_OSM;
        ret->m_maxZoom=0;
        if(isNew && m_ovrCtrlV)
        {
            name+=" (*.osm)";
            QListWidgetItem *newItem =new QListWidgetItem(name);
            newItem->setCheckState(Qt::Unchecked);
            newItem->setData(Qt::UserRole,QVariant::fromValue<CMapSrc*>(ret));
            m_ovrCtrlV->addItem(newItem);
            m_ovrCtrlV->sortItems();
        }
        break;
    case 254:	//Vector
        ret->m_type=MAP_VECT;
        ret->m_maxZoom=0;
        if(isNew && m_ovrCtrlV)
        {
            name+=" (*.img)";
            QListWidgetItem *newItem =new QListWidgetItem(name);
            newItem->setCheckState(Qt::Unchecked);
            newItem->setData(Qt::UserRole,QVariant::fromValue<CMapSrc*>(ret));
            m_ovrCtrlV->addItem(newItem);
            m_ovrCtrlV->sortItems();
         }
      break;
    default:	//Pixel Basemap
        ret->m_type=MAP_BAS;
        ret->m_maxZoom=maxZoom;
        if(isNew && m_srcBox)
            m_srcBox->addItem(name);
        break;
    }
    ret->m_useOvrs[0]=ret->m_useOvrs[1]=false;
    return ret;

}

void CMapSrc::writeTaho(QString path,bool onlyPriv)
{

    QFile fTaho(path);
    if(fTaho.open(QIODevice::WriteOnly))
    {
        QTextStream sTaho(&fTaho);

        sTaho << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
        sTaho << "<taho version=\"2.0\" creator=\"Taho.exe - http://www.dimitri-junker.de/html/openstreetmap.html\">\n";
        sTaho << "\t<mapallsrc>\n";

        QMap<QString,CMapSrc *>::iterator pos=m_mapSrc.begin();
        while (pos != m_mapSrc.end())
        {
            CMapSrc *map=pos.value();
            QString name=pos.key();
            pos++;
            if(!onlyPriv || map->m_isPrivate)
            {
                sTaho << "\t\t<src>\n";
                sTaho << "\t\t\t<name>"<< name <<"</name>\n";
                sTaho << "\t\t\t<prefix>" << map->m_pref <<"</prefix>\n";
                sTaho << "\t\t\t<url>" << map->m_url << "</url>\n";
                sTaho << "\t\t\t<ext>"<<map->m_ext<<"</ext>\n";
                sTaho << "\t\t\t<type>"<<map->m_type<<"</type>\n";
                switch(static_cast<int>(map->m_type))
                {
                case MAP_BAS:
                    sTaho << "\t\t\t<maxzoom>"<< map->m_maxZoom <<"</maxzoom>\n";
                    sTaho << "\t\t\t<QTLocID>"<< map->m_mapID <<"</QTLocID>\n";
                    sTaho << "\t\t\t<QTLocDef>"<< map->m_mapDefin <<"</QTLocDef>\n";
                    break;
                case MAP_VECT:
                    sTaho << "\t\t\t<offset>"<< map->m_offset <<"</offset>\n";
                    sTaho << "\t\t\t<mapspf>"<< map->m_mapsPF <<"</mapspf>\n";
                [[clang::fallthrough]]; case MAP_OSM:
                    sTaho << "\t\t\t<ksize>"<< map->m_kSize <<"</ksize>\n";
                    break;
                }
                sTaho << "\t\t</src>\n";
            }
        }
        sTaho << "\t</mapallsrc>\n";
        sTaho << "\t<OsmIds>\n";
        QMapIterator<QString, QString> iter(COsm::m_osmIDs);

        while(iter.hasNext())
        {
            iter.next();
            sTaho << "\t\t<ID>\n";
            sTaho << "\t\t\t<name>"<< iter.key() <<"</name>\n";
            sTaho << "\t\t\t<IDV>"<< iter.value() <<"</IDV>\n";
            sTaho << "\t\t</ID>\n";
        }
        sTaho << "\t</OsmIds>\n";
        sTaho << "\t<OsmLogins>\n";
        QMapIterator<QString, QStringList> iterL(COsm::m_osmLogins);

        while(iterL.hasNext())
        {
            iterL.next();
            sTaho << "\t\t<LOGIN>\n";
            sTaho << "\t\t\t<Domain>"<< iterL.key() <<"</Domain>\n";
            sTaho << "\t\t\t<name>"<< iterL.value()[0] <<"</name>\n";
            sTaho << "\t\t\t<pwd>"<< iterL.value()[1] <<"</pwd>\n";
            sTaho << "\t\t</LOGIN>\n";
        }
        sTaho << "\t</OsmLogins>\n";
        sTaho << "</taho>\n";
    }
}

bool CMapSrc::readTaho(CXmlFile *xTaho)
{
    bool ret=false;
    QString sOsmIDs;
    if(xTaho->readValB(sOsmIDs,"OsmIds"))
    {
        QString sID;
        int pos=0;
        while(xTaho->readValB(sID,sOsmIDs,"ID",&pos))
        {
            QString sName;
            QString sIDVal="";
            xTaho->readValB(sName,sID,"name");
            xTaho->readValB(sIDVal,sID,"IDV");
            COsm::m_osmIDs.insert(sName,sIDVal);
        }
    }
    QString sOsmLogins;
    if(xTaho->readValB(sOsmLogins,"OsmLogins"))
    {
        QString sLI;
        int pos=0;
        while(xTaho->readValB(sLI,sOsmLogins,"LOGIN",&pos))
        {
            QString sDomain;
            QString sName;
            QString sPWD;
            xTaho->readValB(sDomain,sLI,"Domain");
            xTaho->readValB(sName,sLI,"name");
            xTaho->readValB(sPWD,sLI,"pwd");
            QStringList slLogin={sName,sPWD};

            COsm::m_osmLogins.insert(sDomain,slLogin);
        }
    }
    QString sMapsrc;
    if(!xTaho->readValB(sMapsrc,"mapPubSrc"))
    {
        QString sMapsrcP;
        xTaho->readValB(sMapsrc,"mapsrc");
        xTaho->readValB(sMapsrcP,"mapovr");
        sMapsrc+=sMapsrcP;
    }
    bool isMysrc=false;
    if(sMapsrc.isEmpty())   //evtl mydefsrc.taho
    {
        xTaho->readValB(sMapsrc,"mapallsrc");
        isMysrc=true;
    }

    if(!sMapsrc.isEmpty())
    {
        QString sSrc;
        int pos=0;
        while(xTaho->readValB(sSrc,sMapsrc,"src",&pos))
        {
            QString sName;
            xTaho->readValB(sName,sSrc,"name");
            QString sURL,sExt;
            xTaho->readValB(sURL,sSrc,"url");
            xTaho->readValB(sExt,sSrc,"ext");

            if(sExt.isEmpty())
                sExt=".png";
            unsigned char maxzoom=255;
            int maxThreads =xTaho->readVal_intDef(sSrc,"maxThreads",1000);
            if(maxThreads==1000)	//altes Taho-File)
            {
                QString slUrl=sURL;
                slUrl=slUrl.toLower();
                if(slUrl.indexOf("wanderreitkarte.de")==-1)
                    maxThreads=255;
                else
                    maxThreads=1;
            }
            QString tmpstr;
            int mapId=1;
            QString mapDefin="l";
            switch(xTaho->readVal_intDef(sSrc,"type"))
            {
                case MAP_DEF:
                    m_src=sName;
                [[clang::fallthrough]]; case MAP_BAS:
                    maxzoom=static_cast<unsigned char>(xTaho->readVal_intDef(sSrc,"maxzoom",255));
                    mapId=xTaho->readVal_intDef(sSrc,"QTLocID",1);
                    xTaho->readValB(tmpstr,sSrc,"QTLocDef");
                    if(tmpstr.contains('h',Qt::CaseInsensitive))
                        mapDefin="h";
                    else
                        mapDefin="l";
                [[clang::fallthrough]]; case MAP_OVR:
                    {
                        xTaho->readValB(tmpstr,sSrc,"prefix");
                        initP(sName,sURL,isMysrc,sExt,static_cast<unsigned char>(maxThreads),tmpstr,maxzoom,mapId,mapDefin);
                    }
                    break;
                case MAP_VECT:
                    xTaho->readValB(tmpstr,sSrc,"prefix");
                    initV(sName,sURL,isMysrc,static_cast<unsigned char>(maxThreads),tmpstr,xTaho->readVal_intDef(sSrc,"offset"),xTaho->readVal_dblDef(sSrc,"ksize",1),xTaho->readVal_intDef(sSrc,"mapspf",1000));
                    break;
                case MAP_OSM:
                    xTaho->readValB(tmpstr,sSrc,"prefix");
                    initV(sName,sURL,isMysrc,static_cast<unsigned char>(maxThreads),tmpstr,-1,xTaho->readVal_dblDef(sSrc,"ksize",1),0);
            }
            ret=true;
        }
    }
    return ret;
}

void CMapSrc::destroy()
{
    QMap<QString,CMapSrc *>::iterator pos=m_mapSrc.begin();
    while (pos != m_mapSrc.end())
    {
        CMapSrc *map=pos.value();
        delete map;
        pos++;
     }

}
void CMapSrc::initBoxes(QComboBox * src,QListWidget *ovr,QListWidget *vect,int nr)
{
    m_srcBox=src;
    m_ovrCtrlP=ovr;
    m_ovrCtrlV=vect;

    QMap<QString,CMapSrc *>::iterator it=m_mapSrc.begin();
    while (it != m_mapSrc.end())
    {
        QString name=it.key();
        CMapSrc *map=it.value();
        it++;
        switch(static_cast<int>(map->m_type))
        {
            case MAP_BAS:	//Pixel Basemap
                if(m_srcBox)
                    m_srcBox->addItem(name);
                break;
            case MAP_OVR:	//Pixel Overlay
                if(ovr)
                {
                    QListWidgetItem *newItem =new QListWidgetItem(name);
                    if(map->m_useOvrs[nr])
                        newItem->setCheckState(Qt::Checked);
                    else
                        newItem->setCheckState(Qt::Unchecked);
                    newItem->setData(Qt::UserRole,QVariant::fromValue<CMapSrc*>(map));
                    m_ovrCtrlP->addItem(newItem);
                }
                break;
            case MAP_VECT:	//Vector
                if(vect)
                {
                    name +=" (*.img)";
                    QListWidgetItem *newItem =new QListWidgetItem(name);
                    if(map->m_useOvrs[nr])
                        newItem->setCheckState(Qt::Checked);
                    else
                        newItem->setCheckState(Qt::Unchecked);
                    newItem->setData(Qt::UserRole,QVariant::fromValue<CMapSrc*>(map));
                    m_ovrCtrlV->addItem(newItem);
                }
                break;
            case MAP_OSM:	//*.osm
                if(vect)
                {
                    name +=" (*.osm)";
                    QListWidgetItem *newItem =new QListWidgetItem(name);
                    if(map->m_useOvrs[nr])
                        newItem->setCheckState(Qt::Checked);
                    else
                        newItem->setCheckState(Qt::Unchecked);
                    newItem->setData(Qt::UserRole,QVariant::fromValue<CMapSrc*>(map));
                    m_ovrCtrlV->addItem(newItem);
                }
                break;
        }

    }
    if(m_srcBox && !m_src.isEmpty())
        select(m_src);
    if(m_ovrCtrlP)
        m_ovrCtrlP->sortItems();
    if(m_ovrCtrlV)
        m_ovrCtrlV->sortItems();
}
CMapSrc * CMapSrc::lookup(QString name)
{
    CMapSrc *ret=nullptr;
    QMap<QString,CMapSrc *>::iterator it=m_mapSrc.find(name);
    if(it!=m_mapSrc.end())
        ret=it.value();
    return ret;
}

void CMapSrc::resetOvr()
{
    QMap<QString,CMapSrc *>::iterator pos=m_mapSrc.begin();
    while (pos != m_mapSrc.end())
    {
        QString name=pos.key();
        pos++;
        select(name,LCC_DES);
    }

}

void CMapSrc::select(QString name,int action,int nr)
{

    QMap<QString,CMapSrc *>::iterator it=m_mapSrc.find(name);
    if(it!=m_mapSrc.end())
    {
        CMapSrc *map=it.value();
        QString name=it.key();
        if(map->m_type==MAP_BAS)
        {
            if(action==LCC_SEL )
            {
                m_src=name;
                if(m_srcBox)
                    m_srcBox->setCurrentText(name);
            }
        }
        else
        {
            switch(action)
            {
            case LCC_TOG:
                map->m_useOvrs[nr]=!map->m_useOvrs[nr];
                break;
            case LCC_DES:
                map->m_useOvrs[nr]=false;
                break;
            case LCC_SEL:
                map->m_useOvrs[nr]=true;
                break;
            }
            if(map->m_type==MAP_OVR)
            {
                if(m_ovrCtrlP)
                {
                    QList<QListWidgetItem *> items=m_ovrCtrlP->findItems(map->m_name,Qt::MatchExactly);
                    if(items.size()==1)
                        items[0]->setCheckState(map->m_useOvrs[nr]?Qt::Checked:Qt::Unchecked);
                }
            }
            else if(m_ovrCtrlV)
            {
                QList<QListWidgetItem *> items=m_ovrCtrlV->findItems(map->m_name,Qt::MatchExactly);
                if(items.size()==1)
                    items[0]->setCheckState(map->m_useOvrs[nr]?Qt::Checked:Qt::Unchecked);
            }
        }
    }

}
void CMapSrc::writeTahoSelected(QTextStream &oFile, MapType type, int nr)
{
    QString name;
    switch(static_cast<int>(type))
    {
        case MAP_BAS:
            name=m_srcBox->currentText();
            if(!name.isEmpty())
                oFile << "\t\t<sourceName>" << name << "</sourceName>\n";
            break;
        case MAP_OVR:
        case MAP_VECT:
        case MAP_OSM:
            QMap<QString,CMapSrc*>::iterator it=CMapSrc::m_mapSrc.begin();
            while (it != CMapSrc::m_mapSrc.end())
            {
                CMapSrc *map=it.value();
                if(map->m_type==type && map->m_useOvrs[nr])
                    oFile << "\t\t<overlayName>" << name << "</overlayName>\n";
                ++it;
            }
            break;
    }
}

QString CMapSrc::getSrc()
{
    return m_srcBox->currentText();
}


/*** ------------------------------------------------------------------------ ***/
/*	bool CMapSrc::hatUnbekannteID()					*/
/*								*/
/*	Parameter:	keine															*/
/*	Ergebnis:	true wenn die Quelle eine ID enthält und diese nicht definiert ist	*/
/*	30.12.18 von Dimitri Junker	DYJ 4.07c									*/
/*** ------------------------------------------------------------------------ ***/
bool CMapSrc::hatUnbekannteID()
{

    bool ret=false;
    if(m_url.contains("{ID_",Qt::CaseInsensitive))
    {
         int indx=m_url.indexOf("{ID_");
         QString idName=m_url.mid(indx+1);
         indx=idName.indexOf("}");
         if(indx>-1)
         {
             idName.truncate(indx);
             QString idValue=COsm::m_osmIDs.value(idName,"");
             if(idValue.isEmpty())
             {
                 bool ok;
                 idValue=QInputDialog::getText(nullptr,QObject::tr("Bitte ID eingeben"),idName,QLineEdit::Normal,"",&ok);
                 if(ok)
                 {
                     COsm::m_osmIDs.insert(idName,idValue);
                     writeTaho(CMapSrc::m_mySrc,true);
                 }
                 else
                 {
                     QMessageBox::warning(nullptr, "", QObject::tr("Bitte andere Quelle wählen oder sich auf der Homepage anmelden"),
                                                   QMessageBox::Ok);
                    ret=true;
                 }
             }
         }
     }
    return ret;
}
