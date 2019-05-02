// tahomainw.cpp : Implementierungsdatei
//
#include "tahomainw.h"
#include "ui_tahomainw.h"
#include <cpixmap.h>
#include <cmapsrc.h>
#include <QtNetwork/QNetworkRequest>
#include <QFileDialog>
#include <cgeorect.h>
#include <QTextStream>
#include <cpath.h>
#include <cosm.h>
#include <QtCore/qmath.h>
#include <QDesktopServices>
#include <QMessageBox>
#include <urldownload.h>
#include <cdokmz.h>
#include <ctahocopy.h>
#include <Logfile.h>
#include <QClipboard>
#include <Vectmap.h>
#include <QtConcurrent/QtConcurrent>
#include <qinputdialog.h>

//CHG: TAHO 2.10e DYJ
//Beim Laden eines *.taho wurde die Datenquelle (Mapnik,...) nicht übernommen

//CHG: TAHO 2.10f DYJ
/*
Speichert man Taho z.B. auf einem USB Stick kann man jetzt angeben (Optionen), daß als
default Pfad nicht "Eigene Dateien" sondern der Programm-Pfad benutzt werden soll, außerdem können die Pfade im *.Taho
(derzeit nur der Pfad des Packers) relativ zum *.taho-File gesichert werden. Hat man also taho.exe. die *.tahos und den Packer
auf dem USB-Stick funktioniert es unabhängig davon unter welchem Laufwerksbuchstaben es eingebunden wird.
Außerdem habe ich einige Einträge im *.taho in die Struktur options zusammengefaßt.

CHG: TAHO 2.10h DYJ
"aus URL" erkannte den Zoomlevel nicht mehr
* 06.11 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 3.03a'
Die maximala Anzahl Download-Threads kann jetzt eingestellt werden, z.B. für die Reit& Wanderkarte
* 07.11 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 3.03b'
Karten können weiterhin als png oder jpg gespeichert werden, allerdings können erstere jetzt auch
die Endung png.tile erhalten dies ist z.B. für den osmtracker unter android nötig.
* 07.12 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 3.06b'
png.tile wurde nicht unsichtbar beim Umstellen auf Vector

* 1.08.12 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 3.06c'
Eine weitere Änderung um den Lizenz-Bedingungen der http://www.wanderreitkarte.de/ näher zu kommen.
Diese fordert u.a. eine eigene User Agent Identifikation
* 21.04.13 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 3.07a'
osmarender existiert nicht mehr
* 1.12.13 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 3.08a'
Da die OSM-Export-Seite nicht mehr für die Zwecke dieses Programms nutzbar war habe ich eine andere Seite angepasst
und hier eingebaut.
* 21.12.13 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 3.08c'
Visual Studio 2013 unterstützt nicht mehr das Setzen von Combobox-Inhalten per Resource Editor,
deshalb müssen diese anders gefüllt werden.
* 24.01.14 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 3.10a'
Verfahren zur grafischen Auswahl des Bereichs geändert

 * 15.03.15 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.01a'
Fehler bei der Zoom-Level-Auswahl behoben
 * 25.09.15 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.03b'
 * Aus bbox wurde Süd und Nord vertauscht
 *
 * 08.06.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.05a'
 * Die oche.de hat den Betrieb eingestellt
 *
 * 25.12.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.07a'
 * Als Default Pfad für das Erzeugen von Karten wird jetzt einer unter "Dokumente" verwendet, da im Programmpfad evtl. keine Schreibrechte existieren.
 *
 * 25.12.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.07b'
 * Beim Default Path gab es Probleme mit dem letzten backslash
 *
 * 4.12.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.07k'
 * Auch die Releseversion schreibt jetzt ein Logfile
*/

#define T_SET "default.taho"
#define T_NA "tna.png"


TahoMainW::TahoMainW(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TahoMainW)
{
    ui->setupUi(this);
    ui->cb_auto->setChecked(true);
    ui->le_out->setEnabled(false);
    ui->pb_out->setEnabled(false);


    ui->cb_bpp->setCurrentIndex(2);
    m_opt.setCache(7);
    ui->rb_nbNr->setChecked(true);
    m_tahoParam.setFile("");
    ui->rb_png->setChecked(true);
    m_opt.m_pLanguage=&m_language;
    m_opt.m_version=this->windowTitle();

    m_opt.m_version.remove(0,5);


    for(int i=1;i<19;i++)
        ui->lw_zoom->item(i-1)->setCheckState(Qt::Unchecked);
    for(int i=0;i<ANZ_KAL;i++)
        ui->lw_kal->item(i)->setCheckState(Qt::Unchecked);

    CMapSrc::initBoxes(ui->cb_quelle,ui->lw_overl,ui->lw_vect);
    ui->cb_size->setCurrentIndex(4);
//    OnSelchangeSizec(4);

    ui->tw_mapType->setCurrentIndex(0);

    bool execDone=false;

// default.taho laden
    OnSelchangeSourceP();

    if(m_opt.m_par.isEmpty())
    {
        m_opt.m_tahoPfad=m_opt.m_persPath+T_SET;
        execDone=loadTaho(m_opt.m_tahoPfad);    //1. Versuch unter eigene Dateien
        if(!execDone)
        {
            execDone=loadTaho(m_opt.m_prgPath+T_SET);       //dann Versuch 2 unter eigene Dateien
            if(execDone)        //ist im Prg-Ordner, soll aber in Eigene Dateien Sein
                saveTaho(m_opt.m_tahoPfad);
        }
    }
    else
        execDone=loadTaho(m_opt.m_par); //  Dann wurde ein Taho übergeben.




    /*2doUA
    DWORD len;
    UrlMkGetSessionOption(URLMON_OPTION_USERAGENT,m_opt.m_useragent[0].GetBuffer(1001),1000,&len,0);
    m_opt.m_useragent[0].ReleaseBuffer();
    m_opt.m_useragent[1]=this->windowTitle();
    m_opt.m_useragent[2]="Taho";
*/
}
TahoMainW::~TahoMainW()
{
    delete ui;
}




enum MAPTYPES {MAP_PNG,MAP_JPG,MAP_PNG_TILE};


#define T_SET "default.taho"
#define T_NA "tna.png"
//#define BIF_NEWDIALOGSTYLE	   0x0040
//http://mt1.google.com/vt/lyrs=m@113&hl=de&x=33883&y=22006&z=16
/////////////////////////////////////////////////////////////////////////////
// TahoMainW Dialogfeld

static int SizeP[]={SIZE_FREE,SIZE_NONE,SIZE_QT,256,512,1024,2048,4096,8192,16384,-1};

/*
void TahoMainW::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(TahoMainW)
    DDX_Control(pDX, IDC_KAL, m_kalCtrl);
    DDX_Control(pDX, IDC_ZOOM, m_zoomCtrl);
    DDX_Control(pDX, IDC_TAB_FTYP, m_tabCtrl);
    DDX_Control(pDX, IDC_VECTOR, m_ovrCtrlV);
    DDX_Control(pDX, IDC_OVERLAY_P, m_ovrCtrlP);
    DDX_Control(pDX, IDC_SOURCE_P, m_srcBox);
    DDX_Text(pDX, IDC_LAT_1, m_lat1);
    DDX_Text(pDX, IDC_LAT_2, m_lat2);
    DDX_Text(pDX, IDC_LON_1, m_lon1);
    DDX_Text(pDX, IDC_LON_2, m_lon2);
    DDX_Check(pDX, IDC_AUTO_OUT, m_AutoOut);
    DDX_Text(pDX, IDC_OUTP, m_output);
    DDX_Radio(pDX, IDC_NR, m_nameBy);
    DDX_CBIndex(pDX, IDC_SIZEC, m_sizeN);
    DDX_CBIndex(pDX, IDC_BPP, m_bpp);
    DDX_Radio(pDX, IDC_FTYPE, m_nPictType);
    //}}AFX_DATA_MAP
    CChkListCtrl *ctrl;
    if (m_tabCtrl.GetCurSel() == 1)
        ctrl = &m_ovrCtrlV;
    else
    {
        for (int i = 0; i < m_zoomCtrl.GetItemCount(); i++)
        {
            int nr = atoi(m_zoomCtrl.GetItemText(i, 0));
            if (nr)
            {
                if (pDX->m_bSaveAndValidate)
                    m_bZoom[nr] = m_zoomCtrl.GetCheck(i);
                else
                    m_zoomCtrl.SetCheck(i, m_bZoom[nr]);
            }
        }
        LVFINDINFO lvfi;
        lvfi.flags = LVFI_STRING;
        for (int i = 0; i<ANZ_KAL; i++)
        {
            lvfi.psz = KalNames[i];
            int idx = m_kalCtrl.FindItem(&lvfi);
            if (idx>-1)
            {
                if (pDX->m_bSaveAndValidate)
                    m_bMakeKal[i] = m_kalCtrl.GetCheck(idx);
                else
                    m_kalCtrl.SetCheck(idx, m_bMakeKal[i]);
            }
        }
        ctrl = &m_ovrCtrlP;
    }
    for (int i = 0; i < ctrl->GetItemCount(); i++)
    {
        CMapSrc *map = (CMapSrc *)ctrl->GetItemData(i);
        if (map)
        {
            if (pDX->m_bSaveAndValidate)
                map->m_useOvrs[0] = ctrl->GetCheck(i);
            else
                ctrl->SetCheck(i, map->m_useOvrs[0]);
        }
    }
    DDX_Control(pDX, IDC_BPP, m_bppCB);
    DDX_Control(pDX, IDC_SIZEC, m_sizeCB);
}


*/
/////////////////////////////////////////////////////////////////////////////
// TahoMainW Nachrichten-Handler




void TahoMainW::OnDoTaho()
{
//2doUA    UrlMkSetSessionOption(URLMON_OPTION_USERAGENT,(void *)LPCTSTR(m_opt.m_useragent[m_opt.m_uaID]),m_opt.m_useragent[m_opt.m_uaID].GetLength(),0);
    if(ui->tw_mapType->currentIndex())
        makeVmap();
    else
        makePmap();
}

void TahoMainW::OnZ0()
{
    for(int i=1;i<19;i++)
        ui->lw_zoom->item(i)->setCheckState(Qt::Unchecked);

}

void TahoMainW::OnZ2()
{
    int i;
    for(i=2;i<19;i+=2)
        ui->lw_zoom->item(i-1)->setCheckState(Qt::Checked);
    for(i=1;i<19;i+=2)
        ui->lw_zoom->item(i-1)->setCheckState(Qt::Unchecked);

}


void TahoMainW::OnZAll()
{
    for(int i=1;i<19;i++)
        ui->lw_zoom->item(i-1)->setCheckState(Qt::Checked);

}



void TahoMainW::OnAutoOut()
{
    OnSelchangeSourceP();
    ui->le_out->setDisabled(ui->cb_auto->isChecked());
    ui->pb_out->setDisabled(ui->cb_auto->isChecked());
}


void TahoMainW::OnGetOut()
{

    QString path= QFileDialog::getExistingDirectory(this,tr("Ausgabe Verzeichnis"),ui->le_out->text());
    if(!path.isEmpty())
        ui->le_out->setText(path);
 }



//Erzeugt ein TAHO-File mit den Einstellungen

void TahoMainW::on_actionEinstellungen_speichern_triggered()
{

    QString path= QFileDialog::getSaveFileName(this,tr("Speicher Einstellungen"),m_opt.m_tahoPfad,tr("TAHO-File(*.taho)"));
    if(!path.isNull())
        saveTaho(path);
}
void TahoMainW::saveTaho(QString &path)
{
    CGeoRect gr(ui->le_lat1->text().toDouble(),ui->le_lat2->text().toDouble(),ui->le_lon1->text().toDouble(),ui->le_lon2->text().toDouble());

    if(!gr.isValid())
    {
        QMessageBox::warning(nullptr, tr("Speichern"), tr("Die Koordinaten sind ungültig"));
        return;
    }
//Taho-File öffnen falls gewünscht
    QFile fTaho(path);
    if(fTaho.open(QIODevice::WriteOnly))
    {
        QTextStream oTaho(&fTaho);
        oTaho << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
        oTaho << "<taho version=\"1.0\" creator=\"Taho.exe - http://www.dimitri-junker.de/html/openstreetmap.html\">\n";
/*        oTaho << "\t<options>\n";
        //:put here the abreviation of the new Language (for ex. en for english, fr for frensh, es for spanish,...
        oTaho << "\t\t<language>" << tr("de") <<"</language>\n";
        oTaho << "\t\t<zip>" << m_opt.getZipPath() << "</zip>\n";
        oTaho << "\t\t<zippar>" << m_opt.getZipPar() << "</zippar>\n";
        oTaho << "\t\t<ungzpar>" << m_opt.getUnGzPar() << "</ungzpar>\n";
        oTaho << "\t\t<OfflineDir>" << m_opt.getOffDirPath() << "</OfflineDir>\n";
        oTaho << "\t\t<maxThreads>" << m_opt.m_tasks << "</maxThreads>\n";
        oTaho << "\t\t<OsmUrl>" << m_opt.m_osmUrl << "</OsmUrl>\n";
        oTaho << "\t</options>\n";
*/
        oTaho << "\t<params>\n";
        oTaho << "\t\t<lat1>" << gr.m_n << "</lat1>\n";
        oTaho << "\t\t<lat2>" << gr.m_s << "</lat2>\n";
        oTaho << "\t\t<lon1>" << gr.m_w << "</lon1>\n";
        oTaho << "\t\t<lon2>" << gr.m_e << "</lon2>\n";
        bool vector=ui->tw_mapType->currentIndex()==1;
        if(vector)
        {
            oTaho << "\t\t<pictType>-1</pictType>\n";
            CMapSrc::writeTahoSelected(oTaho,MAP_VECT);
            CMapSrc::writeTahoSelected(oTaho,MAP_OSM);
        }
        else
        {
            oTaho << "\t\t<pictType>" << pictType() << "</pictType>\n";
            oTaho << "\t\t<size>" << SizeP[ui->cb_size->currentIndex()] << "</size>\n";
            oTaho << "\t\t<bpp>" << ui->cb_bpp->currentText() << "</bpp>\n";
            for(int i=0;i<ANZ_KAL;i++)
                oTaho << "\t\t<" << ui->lw_kal->item(i)->text() << ">" << ui->lw_kal->item(i)->checkState()/2 << "</" <<ui->lw_kal->item(i)->text() <<">\n";
            for(int zooml=1;zooml<19;zooml++)
                oTaho << "\t\t<zooml" << zooml << ">" << static_cast<int>(loadZoomL(zooml))<< "</zooml" << zooml <<">\n";
            CMapSrc::writeTahoSelected(oTaho,MAP_BAS);
            CMapSrc::writeTahoSelected(oTaho,MAP_OVR);
        }

        if(ui->cb_auto->isChecked())
            oTaho << "\t\t<out></out>\n";
        else
            oTaho << "\t\t<out>" << ui->le_out->text() << "</out>\n";

        oTaho << "\t\t<nameBy>" << nameBy() << "</nameBy>\n";
        oTaho << "\t\t<cacheDays>" << m_opt.getCache() << "</cacheDays>\n";
        oTaho << "\t</params>\n";
        oTaho << "</taho>\n";
        fTaho.close();
        m_tahoParam=path;

    }
}
int TahoMainW::pictType()
{
    if(ui->rb_png->isChecked())
        return MAP_PNG;
    else if(ui->rb_jpg->isChecked())
        return MAP_JPG;
    else
        return MAP_PNG_TILE;
}
int TahoMainW::nameBy()
{
    if(ui->rb_nbNr->isChecked())
        return NB_NUM;
    else if(ui->rb_nbKoord->isChecked())
        return NB_COORD;
    else
        return NB_DIR;
}

void TahoMainW::on_actionEinstellungen_laden_triggered()
{

    QString path= m_tahoParam.makePath(PATH_PATH);
    path=QFileDialog::getOpenFileName(this,tr("Lade Einstellungen"),path,tr("TAHO-File(*.taho)"));
    if(!path.isNull())
        loadTaho(path);

}
//Rückgabe: Wurde ShellExecute ausgeführt?
bool TahoMainW::loadTaho(QString pfad)
{
    bool ret=false;
    QString sTaho;
    if(m_opt.loadTahoSrc(pfad,&sTaho))
        OnSelchangeSourceP();
    QString source;
    m_opt.setOffDirPath("");
    m_opt.setZipPath("");
    if(!sTaho.isEmpty())
    {
        m_opt.m_tahoPfad=pfad;
        QSettings settings("Dimitri-Junker.de","Taho");
        QString vers=settings.value("Version").toString();
        if(vers.isEmpty())
        {
            QString sOpt,tmp;	//CHG: TAHO 2.10f DYJ
            if(!CXmlFile::readValB(sOpt,sTaho,"options"))	//Taho Vers. <2.10
            {
                sOpt=sTaho;
                if(CXmlFile::readValB(tmp,sOpt,"zip") && !tmp.isEmpty())
                    m_opt.setZipPath(tmp);
                if(CXmlFile::readValB(tmp,sTaho,"zippar")&& !tmp.isEmpty())
                    m_opt.setZipPar(tmp);
            }
            else
            {


                CXmlFile::readValB(tmp,sOpt,"zip");
                m_opt.setZipPath(tmp);
                CXmlFile::readValB(tmp,sTaho,"zippar");
                m_opt.setZipPar(tmp);
                CXmlFile::readValB(tmp,sTaho,"ungzpar");
                m_opt.setUnGzPar(tmp);
                if(CXmlFile::readValB(tmp,sOpt,"OsmUrl") && !tmp.isEmpty())
                    m_opt.m_osmUrl=tmp;
                CXmlFile::readValB(tmp,sOpt,"OfflineDir");
                m_opt.setOffDirPath(tmp);

            }
            m_opt.m_tasks=static_cast<int>(qMax(1,CXmlFile::readVal_intDef(sOpt,"maxThreads",QThread::idealThreadCount())));

            QString lang="";
            if(CXmlFile::readValB(lang,sOpt,"language"))
            {
                bool ok;
                int langNr=lang.toInt(&ok,0);
                if(ok)
                {
                    switch(langNr)
                    {
                    case 7:
                        lang="de";
                        break;
                    case 9:
                        lang="en";
                        break;
                    case 12:
                        lang="fr";
                        break;
                    }
                }
                if(lang=="de")
                    qApp->removeTranslator(&m_language);
                else
                {
                    m_language.load("taho_"+lang);
                    qApp->installTranslator(&m_language);
                }
            }
            m_opt.saveOptions();
        }
        else
        {
            QString tmp;
            m_opt.setZipPath(settings.value("zip").toString());
            m_opt.setZipPar(settings.value("zippar").toString());
            m_opt.setUnGzPar(settings.value("ungzpar").toString());
            tmp=settings.value("OsmUrl").toString();
            if(tmp.isEmpty())
                settings.setValue("OsmUrl",m_opt.m_osmUrl);
            else
                m_opt.m_osmUrl=tmp;
            m_opt.setOffDirPath(settings.value("OfflineDir").toString());
            m_opt.m_tasks=qMax(1,settings.value("maxThreads").toInt());
            QString lang=settings.value("language").toString();
            if(lang=="de")
                qApp->removeTranslator(&m_language);
            else
            {
                m_language.load("taho_"+lang);
                qApp->installTranslator(&m_language);
            }
        }
        QString sParams;
        CXmlFile::readValB(sParams,sTaho,"params");
        if(!sParams.isEmpty())
        {
            m_tahoParam=pfad;

            int pictType=CXmlFile::readVal_intDef(sParams,"pictType",0);


            if(pictType==-1)
                ui->tw_mapType->setCurrentIndex(1);	//img-Vektorkarte
            else
            {	//png oder jpg Karte
                ui->rb_jpg->setChecked(pictType==MAP_JPG);
                ui->rb_png->setChecked(pictType==MAP_PNG);
                ui->rb_pngt->setChecked(pictType==MAP_PNG_TILE);
                //size + zoomlevel
                int sizeP=CXmlFile::readVal_intDef(sParams,"size",1024);
                switch(sizeP)	//vieleicht altes size
                {
                case -1:	//alt: Frei
                    sizeP=SIZE_FREE;
                    break;
                case 0: //alt: Tiles
                    sizeP=SIZE_NONE;
                    break;
                case 1:	//alt 512*512
                    sizeP=512;
                    break;
                case 2: //alt 1024*1024
                    sizeP=1024;
                    break;
                case 3: //alt 2048*2048
                    sizeP=2048;
                    break;
                case 4: //alt 4096*4096
                    sizeP=4096;
                    break;
            }
                int sizeN=4;    //1024*1024
                for(int j=0;SizeP[j]>-1;j++)
                {
                    if(SizeP[j]==sizeP)
                    {
                        sizeN=j;
                        break;
                    }
                }
                ui->cb_size->setCurrentIndex(sizeN);

                for(int zooml=1;zooml<19;zooml++)
                {
                    QString tagnr;
                    tagnr.sprintf("zooml%d",zooml);
                    if(CXmlFile::readVal_intDef(sParams,tagnr))
                       ui->lw_zoom->item(zooml-1)->setCheckState(Qt::Checked);
                    else
                       ui->lw_zoom->item(zooml-1)->setCheckState(Qt::Unchecked);
                }
                //Bits/Pixel
                switch(CXmlFile::readVal_intDef(sParams,"bpp",2))
                {
                case 8:
                    ui->cb_bpp->setCurrentIndex(0);
                    break;
                case 24:
                    ui->cb_bpp->setCurrentIndex(1);
                    break;
                case 32:
                default:
                    ui->cb_bpp->setCurrentIndex(2);
                    break;
                }

//Kalibrationsfiles,...
                for(int i=0;i<ANZ_KAL;i++)
                {
                    if(CXmlFile::readVal_intDef(sParams,ui->lw_kal->item(i)->text()))
                        ui->lw_kal->item(i)->setCheckState(Qt::Checked);
                     else
                        ui->lw_kal->item(i)->setCheckState(Qt::Unchecked);
                }
                ui->tw_mapType->setCurrentIndex(0);
//Source
                CXmlFile::readValB(source,sParams,"sourceName");
                CMapSrc::select(source);
            }
//Koordinaten
            QString coord;
            CXmlFile::readValB(coord,sParams,"lat1");
            ui->le_lat1->setText(coord);
            CXmlFile::readValB(coord,sParams,"lat2");
            ui->le_lat2->setText(coord);
            CXmlFile::readValB(coord,sParams,"lon1");
            ui->le_lon1->setText(coord);
            CXmlFile::readValB(coord,sParams,"lon2");
            ui->le_lon2->setText(coord);

//Overlays bzw Vectormaps	//CHG: TAHO 2.11a DYJ
            // Overlays deaktivieren
            CMapSrc::resetOvr();
            int aktPos=0;
            QString overlay;
            while(CXmlFile::readValB(overlay,sParams,"overlayName",&aktPos))
                CMapSrc::select(overlay);

//Output
            QString out;
            if(CXmlFile::readVal_intDef(sParams,"AutoOut"))	//veraltet
                out="";
            else
                CXmlFile::readValB(out,sParams,"out");
            bool isAuto=out.isEmpty();
            ui->cb_auto->setChecked(isAuto);
            ui->le_out->setDisabled(isAuto);
            ui->pb_out->setDisabled(isAuto);
            if(isAuto)
                setAutoPath();
            else
                ui->le_out->setText(out);


            int nameBy=CXmlFile::readVal_intDef(sParams,"nameBy");
            ui->rb_nbNr->setChecked(nameBy==NB_NUM);
            ui->rb_nbKoord->setChecked(nameBy==NB_COORD);
            ui->rb_nbDir->setChecked(nameBy==NB_DIR);
            m_opt.setCache(static_cast<unsigned int>(CXmlFile::readVal_intDef(sParams,"cacheDays",7)));

//            QString url=m_opt.m_osmUrl+"bbox-tool/bbox.html";
//            QDesktopServices::openUrl(QUrl(url) );

            ret=true;
        }
    }
    return ret;
}


void TahoMainW::on_actionOSMBugs_holen_triggered()

{
//	bool ok=TRUE;
    CGeoRect gr(ui->le_lat1->text().toDouble(),ui->le_lat2->text().toDouble(),ui->le_lon1->text().toDouble(),ui->le_lon2->text().toDouble());
    if(gr.isValid())
    {
        QString url,ziel,zielTxt="";
        ziel= QFileDialog::getSaveFileName(this,tr("Speicher OSM Bugs"),"",tr("GPS eXchange Format(*.gpx);;Generic Ascii(*.asc);;Alle(*.*)"));
        url.sprintf("http://openstreetbugs.schokokeks.org/api/0.1/getGPX?b=%f&t=%f&l=%f&r=%f&limit=100000&open=yes",gr.m_s,gr.m_n,gr.m_w,gr.m_e);
        if(ziel.isNull())
            return;
        if(QMessageBox::question(nullptr, tr("Bugs kürzen"), tr("Sollen die Namen gekürzt und separat gespeichert werden?"),
                                      QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
        {
            zielTxt= QFileDialog::getSaveFileName(this,tr("OSM Bugs Textfile"),"",tr("Text-File(*.txt)"));
            if(ziel.isNull())
                zielTxt="";
        }
        QFileInfo fi(ziel);
        bool conv2asc=fi.suffix().compare("asc",Qt::CaseInsensitive)==0;
        if(conv2asc ||!zielTxt.isEmpty())
        {
            QString tmpF=tmpnam( nullptr );
            if(urlDownload::downloadFile(url, tmpF)==0)
                COsm::convGpxAscShrt(zielTxt,ziel,tmpF,conv2asc,"OpenStreetBugs & Taho");
            remove(tmpF.toStdString().c_str());
        }
        else
            urlDownload::downloadFile(url, ziel);
    }

}



bool TahoMainW::addMaps2ListP(QList<MAKEMAPSP *> &mmList,SDLM_DATA *data, int sizeP,int zoom, CGeoRect &gr)
{
    MAKEMAPSP mkM1;
    mkM1.m_sdlm=data;
//#-----------------------------------------------------------------------------
//# get the tile x and y coordinates
    mkM1.zoom=zoom;
    bool freesize=sizeP==SIZE_FREE;
    int size=0;
    switch(sizeP)
    {
    case 256:
        size=0;
        break;
    case 512:
        size=1;
        break;
    case 1024:
        size=2;
        break;
    case 2048:
        size=3;
        break;
    case 4096:
        size=4;
        break;
    case 8192:
        size=5;
        break;
    case 16384:
        size=6;
        break;
    }
    QPoint tile,tile2;
    tile =COsm::getTileNumber(gr.m_n,gr.m_w,zoom-size);
    tile2=COsm::getTileNumber(gr.m_s,gr.m_e,zoom-size);
    if(tile2.y()<tile.y())
    {
      int tmp=tile2.y();
      tile2.setY(tile.y());
      tile.setY(tmp);
    }

    if(tile2.x()<tile.x())
    {
      int tmp=tile2.x();
      tile2.setX(tile.x());
      tile.setX(tmp);
    }

//#-----------------------------------------------------------------------------
//# $size=5 means 2**5 * 256 pixel == 8192 pixel per side, this is the limit for my 2GB Ram laptop!
//# $size=2 means 1024x1024 pixel, best for Glopus

//#-----------------------------------------------------------------------------
//# loop through the tiles to be made
//	int width = tilesize * 1<<size;

    if(freesize)
    {
        mkM1.xloop = tile.x();
        mkM1.yloop = tile.y();
        mkM1.xsize = tile2.x()-tile.x()+1;
        mkM1.ysize = tile2.y()-tile.y()+1;
        MAKEMAPSP *newMap=new MAKEMAPSP;
        *newMap=mkM1;
        mmList.append(newMap);
    }
    else
    {
        mkM1.xsize = mkM1.ysize = 0;
        for (mkM1.xloop = tile.x() ;mkM1.xloop <= tile2.x();mkM1.xloop++)
        {
            for (mkM1.yloop = tile.y();mkM1.yloop <= tile2.y();mkM1.yloop++)
            {
                MAKEMAPSP *newMap=new MAKEMAPSP;
                *newMap=mkM1;
                mmList.append(newMap);
            }
        }
    }
    return true;
}
bool TahoMainW::addMaps2ListV(QList<MAKEMAPSV *> &mmList,SDLM_DATA *data,CGeoRect gr,CMapSrc *map)
{
    if(map->m_offset==-1)
    {
        if(map->m_kSize>0 && ((gr.m_e-gr.m_w)*(gr.m_n-gr.m_s))>map->m_kSize)
        {
            CGeoRect gr2=gr;
            if((gr.m_e-gr.m_w)>(gr.m_n-gr.m_s))
                gr.m_e=gr2.m_w=(gr.m_e+gr.m_w)/2;
            else
                gr.m_n=gr2.m_s=(gr.m_n+gr.m_s)/2;
            addMaps2ListV(mmList,data,gr,map);
            addMaps2ListV(mmList,data,gr2,map);
        }
        else
        {
            MAKEMAPSV *newMap=new MAKEMAPSV;
            newMap->m_sdlm=data;
            newMap->map=map;
            newMap->rect = gr;
            mmList.append(newMap);
        }
    }
    else
    {
        MAKEMAPSV mkM1;
        mkM1.map=map;
        mkM1.m_sdlm=data;
    //#-----------------------------------------------------------------------------
    //# get the tile x and y coordinates
        gr.m_n=qMin(gr.m_n+map->m_kSize,90.);
        gr.m_e=qMin(gr.m_e+map->m_kSize,180.);

    //#-----------------------------------------------------------------------------
    //# loop through the tiles to be made

        for (mkM1.rect.m_w = gr.m_w ;mkM1.rect.m_w < gr.m_e;mkM1.rect.m_w+=map->m_kSize)
        {
            for (mkM1.rect.m_s = gr.m_s;mkM1.rect.m_s < gr.m_n;mkM1.rect.m_s+=map->m_kSize)
            {
                MAKEMAPSV *newMap=new MAKEMAPSV;
                *newMap=mkM1;
                mmList.append(newMap);
            }
        }
    }
    return true;
}
SDLM_DATA * TahoMainW::initData(int sizeP)
{

    SDLM_DATA *data=new SDLM_DATA;
    data->errs=0;
    data->m_outBas=ui->le_out->text();
    if(!data->m_outBas.isEmpty() && !data->m_outBas.endsWith("/") &&! data->m_outBas.endsWith("\\"))
        data->m_outBas+="/";
    if(sizeP!=SIZE_VECTOR && sizeP!=SIZE_NONE&& sizeP!=SIZE_QT )	//bei Tiles auch keine Kal-Files
    {
        for(int i=0;i<ANZ_KAL;i++)
            data->m_makeKal[i]=ui->lw_kal->item(i)->checkState()==Qt::Checked;
        //initialise the Pathaway conversion batch program
        data->m_PWconvBatchname = "PWconf";
        if(data->m_makeKal[PATHAWAY])
            CPixmap::Ini_batch_PWMapConvert(data->m_PWconvBatchname);
    }
    data->m_sizeP=sizeP;
    data->m_nameBy=nameBy();
    data->m_vector=ui->tw_mapType->currentIndex()==1;
    if(data->m_vector)
        data->m_pictType=".img";
    else if(ui->rb_png->isChecked())
        data->m_pictType=".png";
    else if(ui->rb_pngt->isChecked())
        data->m_pictType=".png.tile";
    else
        data->m_pictType=".jpg";
    data->m_maxCacheDays=m_opt.getCache();
    data->m_errTxts.clear();
    data->m_packer=m_opt.getZipPath();
    data->m_offDir=m_opt.getOffDirPath();
    data->m_unGz=m_opt.getUnGzPar();

    return data;
}


// gibt true zurück wenn es enabled und checked ist
bool TahoMainW::loadZoomL(int zl)
{
    QListWidgetItem *lwi=ui->lw_zoom->item(zl-1);

    if (lwi == nullptr)
    {
        QMessageBox::warning(
                    this, tr("Warnung"),
                    tr("Zoom %1 kommt wahrscheinlich aus Konfigurationsdatei, darf aber nicht benutzt werden."));
        return false;
    }

    if((lwi->flags()&Qt::ItemIsEnabled)==0)
        return false;
    else
        return lwi->checkState()==Qt::Checked;

}

void TahoMainW::OnSelchangeSizec(int size)
{
    int breite=512;
    for(int i=1;i<ui->lw_zoom->count();i++)
    {
        QListWidgetItem *lwi=ui->lw_zoom->item(i-1);
        if(breite>=SizeP[size])
            lwi->setFlags(lwi->flags() |Qt::ItemIsEnabled);
        else
            lwi->setFlags(lwi->flags() & ~Qt::ItemIsEnabled);
        breite*=2;
    }
    ui->rb_jpg->setVisible(size!=1);
    ui->lw_kal->setVisible(size!=1);
    ui->l_kal->setVisible(size!=1);
    ui->cb_bpp->setVisible(size!=1);
    ui->l_bpp->setVisible(size!=1);
    if(size==1 && pictType()==MAP_JPG)
    {
        ui->rb_jpg->setChecked(false);
        ui->rb_png->setChecked(true);
    }
    setAutoPath();

}
void TahoMainW::setAutoPath()
{
    if(ui->cb_auto->isChecked())
    {
        QString outDir;
        if(SizeP[ui->cb_size->currentIndex()]==SIZE_QT)
        {
            outDir=m_opt.getOffDirPath();
            if(outDir.isEmpty())
            {
                m_opt.on_pb_offDir_clicked();

                if(QMessageBox::question(nullptr,tr("Optionen sichern?"),tr("Sollen die Optionen gesichert werden?"),QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
                    m_opt.saveOptions();
                outDir=m_opt.getOffDirPath();
            }

        }
        else
        {
            QString name=CMapSrc::getSrc();
            QFileInfo fi(m_opt.m_persPath,name);
            outDir=fi.filePath();

        }
        ui->le_out->setText((outDir));
    }
}
void TahoMainW::OnHelpb()
{
    OnHelpB();
}


void TahoMainW::on_actionKMZ_erstellen_triggered()
{
    if(m_opt.getZipPath().isEmpty()||m_opt.getZipPar().isEmpty())
    {
        QMessageBox::warning(nullptr, tr("Packer"), tr("Es ist kein Packer eingestellt. Bitte tragen Sie einen im folgenden Dialog ein."),
                                      QMessageBox::Ok);
        on_actionOptionen_triggered();
    }
    if(!m_opt.getZipPath().isEmpty() && !m_opt.getZipPar().isEmpty())
    {
        CDoKmz doKmz(m_opt.getZipPath(),m_opt.getZipPar());
        doKmz.exec();
    }
}
void TahoMainW::on_actionOptionen_triggered()

{
//2do    m_opt.init();

    m_opt.exec();

}

void TahoMainW::on_actionInfo_triggered()
{
    urlDownload::downloadFile("http://wieistmeinuseragent.de/", "uaid_de.html");
    urlDownload::downloadFile("http://whatsmyuseragent.com/", "uaid_en.html");
    urlDownload::downloadFile("http://www.infowebmaster.fr/outils/mon-user-agent.php", "uaid_fr.html");

    QString str=this->windowTitle();
    QString vers=QString("%1 (%2)").arg(str.mid(4)).arg(__DATE__);

    CTahoCopy to(vers);
    to.exec();
}

void loadVMapInThread( MAKEMAPSV * aktMap )
{
    CVectmap vectm(aktMap->rect,*aktMap->m_sdlm, aktMap->map);
    vectm.LoadTile(*aktMap->m_sdlm, vectm.m_filename);
}

void loadPMapInThread( MAKEMAPSP *aktMap)
{
    CMapSrc *mapSrc=static_cast<CMapSrc *>(aktMap->m_sdlm->m_maps[0]);
    bool nurTiles=aktMap->m_sdlm->m_sizeP==SIZE_NONE || aktMap->m_sdlm->m_sizeP==SIZE_QT;
    QString outPath;
    if(nurTiles)
        outPath=aktMap->m_sdlm->m_outBas;
    else
        outPath=aktMap->m_sdlm->m_outBas+"map/";

    CPixmap pixm(aktMap,outPath,mapSrc->m_pref);
    qDebug() << pixm.m_filename;

    QProgressDialog *progD=aktMap->m_sdlm->m_progD;
    if(nurTiles)
    {
        for(int ov=0;ov<aktMap->m_sdlm->m_maps.size();ov++)
           pixm.LoadTile(aktMap->m_sdlm,ov);
        delete aktMap;
        aktMap=nullptr;
    }
    else
    {
        CGeoRect gRect;
        pixm.MakeMapTile(aktMap->m_sdlm,&gRect);
        aktMap->m_sdlm->errs|=pixm.MakeMapCal(aktMap->m_sdlm->m_makeKal,aktMap->m_sdlm->m_PWconvBatchname,&gRect);
    }
    if(progD)
        progD->setValue(progD->value()+1);
}
void TahoMainW::makePmap()
{
    CGeoRect gr(ui->le_lat1->text().toDouble(),ui->le_lat2->text().toDouble(),ui->le_lon1->text().toDouble(),ui->le_lon2->text().toDouble(),85.0511);
    int minzoom=ui->cb_size->currentIndex()-2;
    if(!gr.isValid())
    {
        QMessageBox::warning(nullptr, tr("Karte erstellen"), tr("Bereich ungültig"),
                                      QMessageBox::Ok);
        return;
    }
    CMapSrc *map;
    if((map=CMapSrc::lookup(CMapSrc::getSrc()))==nullptr)
    {
        QMessageBox::warning(nullptr, tr("Karte erstellen"), tr("Quelle ungültig"),
                             QMessageBox::Ok);
        return;
    }
    if(minzoom<1)
        minzoom=1;
//2do    m_maps=m_mapNr=0;
    int size=ui->cb_size->currentIndex();
    SDLM_DATA *data=initData(SizeP[size]);
    if(SizeP[size]!=SIZE_NONE && SizeP[size]!=SIZE_QT)
    {
        QString out=data->m_outBas+"map/";
        QDir dirH;
        if(!dirH.mkpath(out))
        {
            QMessageBox::warning(nullptr, tr("Ausgabeverzeichnis"), tr("Kann Ausgabeverzeichnis nicht erzeugen"),
                                          QMessageBox::Ok);
            delete data;
            return;
        }
    }
        //psi->name="tah"
    data->m_maps.insert(0,map);
    int maxThreads=qMin(static_cast<int>(m_opt.m_tasks),map->m_maxThreads);

    QMap<QString,CMapSrc *>::iterator pos=CMapSrc::m_mapSrc.begin();
    QList<MAKEMAPSP *> mmList;
    while (pos != CMapSrc::m_mapSrc.end())
    {
        CMapSrc *map=pos.value();
        QString name=pos.key();
        pos++;
        if(map->m_useOvrs[0] && map->m_type<MAP_VECT)
        {
            data->m_maps.append(map);
            maxThreads=qMin(maxThreads,map->m_maxThreads);
            QString dir =QString("%1%2/").arg(data->m_outBas).arg(map->m_name);
            QDir dirH;
            if(!dirH.mkpath(dir))
            {
                QMessageBox::warning(nullptr, tr("Karte erstellen"), tr("Kann Ausgabeverzeichnis nicht erzeugen"),
                                     QMessageBox::Ok);
                return;
            }
        }
    }


    if(!data)
        return;
    data->m_tna=m_opt.m_prgPath+T_NA;

    QFileInfo fi(data->m_tna);

    if(!fi.exists())	// prüfen ob das File bereits besteht
    {
        QString url=m_opt.m_osmUrl+"taho/tna.png";
        urlDownload::downloadFile(url, data->m_tna);
    }

    if(gr.m_e<gr.m_w)	// über 180Grad
    {
        CGeoRect gr2=gr;
        gr2.m_e=180.;
        gr.m_w=-179.9999;
        for(int zoom=minzoom;zoom<map->m_maxZoom;zoom++)
        {
            if(loadZoomL(zoom))
            {
                if(!addMaps2ListP(mmList,data,SizeP[ui->cb_size->currentIndex()],zoom,gr2))
                    break;
            }
        }
    }
    for(int zoom=minzoom;zoom<=map->m_maxZoom;zoom++)
    {
        if(loadZoomL(zoom))
        {
            if(!addMaps2ListP(mmList,data,SizeP[ui->cb_size->currentIndex()],zoom,gr))
                break;
        }
    }
    data->m_bpp=ui->cb_bpp->currentIndex();




    if(!mmList.isEmpty())
    {
         int anzahlKarten=mmList.size();
         data->m_done=0;
         QProgressDialog progress("", tr("Abbruch"), 0, anzahlKarten, nullptr);
         progress.setWindowModality(Qt::WindowModal);
         progress.setValue(0);
         progress.setWindowTitle(tr("Lade %n Karten",nullptr,anzahlKarten));

#ifdef QT_DEBUG
         qDebug() << "Kein Multitasking";
         data->m_progD=&progress;
         for(int kNr=0;kNr<anzahlKarten;kNr++)
             loadPMapInThread(mmList[kNr]);	// kein Multitasking zum Debugging
#else

        data->m_progD=nullptr;
        qDebug() << "Multitasking mit " << maxThreads << " Threads";
        QThreadPool::globalInstance()->setMaxThreadCount(maxThreads);
        QFutureWatcher<void> fW;
        QObject::connect(&fW, SIGNAL(finished()), &progress, SLOT(reset()));
        QObject::connect(&progress, SIGNAL(canceled()), &fW, SLOT(cancel()));
        QObject::connect(&fW, SIGNAL(progressRangeChanged(int,int)), &progress, SLOT(setRange(int,int)));
        QObject::connect(&fW, SIGNAL(progressValueChanged(int)), &progress, SLOT(setValue(int)));

        fW.setFuture(QtConcurrent::map(mmList,loadPMapInThread));
        progress.exec();
        fW.waitForFinished();
#endif
    }
    {

        QMessageBox msgBox(QMessageBox::Warning,"Pixel-Tile","",QMessageBox::Ok|QMessageBox::Yes);
        msgBox.button(QMessageBox::Yes)->setText(tr("Log anzeigen"));

        if((data->errs&ERR_PIXM_CREATE)==ERR_PIXM_CREATE)
        {
            auto text = tr("Karte konnte nicht erzeugt werden, evtl. zu groß!");
            if (sizeof(quintptr) == 4) // 32b build
            {
                text += tr("\n\nIhr Programm ist 32bit, versuchen Sie 64bit Version.");
            }
            msgBox.setText( text );
        }
        else if((data->errs&ERR_PIXM_TILE)==ERR_PIXM_TILE)
            msgBox.setText( tr("Mindestens ein Tile konnte nicht heruntergeladen werden. S. Logfile"));
        else if((data->errs&ERR_PIXM_TILE_OLD)==ERR_PIXM_TILE_OLD)
            msgBox.setText( tr("Mindestens ein Tile konnte nicht heruntergeladen werden, aber es gab noch eine alte Version. S. Logfile"));
        else if(mmList.isEmpty())
        {
            msgBox.setText( tr("Nichts zu tun, evtl kein Zoomlevel ausgewählt?"));
            data->errs=1024;
        }
        else if(data->errs>0)
            msgBox.setText( tr("Unbekannter Fehler. S. Logfile"));


        while(!data->m_errTxts.isEmpty())
        {
            m_log.writeFLS(data->m_errTxts.at(0));
            data->m_errTxts.removeFirst();
        }
        m_log.closeHtml();
        if(data->errs>0)
        {
            if(msgBox.exec()==QMessageBox::Yes)
            {
                QString url="file:///"+m_log.m_path;
                QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));
            }
        }
        data->errs=0;


        delete data;
    }

}

void TahoMainW::makeVmap()
{
    if(m_opt.getZipPath().isEmpty()||m_opt.getUnGzPar().isEmpty())
    {
        QMessageBox::warning(nullptr, tr("Packer"), tr("Es ist kein Packer eingestellt. Bitte tragen Sie einen im folgenden Dialog ein."),
                                      QMessageBox::Ok);
        on_actionOptionen_triggered();
    }
    if(m_opt.getZipPath().isEmpty()||m_opt.getUnGzPar().isEmpty())
        return;

    CGeoRect gr(ui->le_lat1->text().toDouble(),ui->le_lat2->text().toDouble(),ui->le_lon1->text().toDouble(),ui->le_lon2->text().toDouble(),85.0511);
    if(!gr.isValid())
    {
        QMessageBox::warning(nullptr, tr("Karte erstellen"), tr("Bereich ungültig"),
                                      QMessageBox::Ok);
        return;
    }

//2do    m_maps=m_mapNr=0;	//???

    SDLM_DATA *data=initData();

    QMap<QString, CMapSrc*>::const_iterator i = CMapSrc::m_mapSrc.constBegin();
    QList<MAKEMAPSV *> mmList;
    int maxThreads=m_opt.m_tasks;

    while (i != CMapSrc::m_mapSrc.constEnd()) {
        CMapSrc *map=i.value();
        QString name=i.key();
        if(map->m_useOvrs[0] && map->m_type>=MAP_VECT)
        {
//			data->m_maps.Add(map);
            if(map->m_offset!=-1)
            {
                QString dir=QString("%1%2/").arg(data->m_outBas).arg(map->m_name);
                QDir dirH;
                if(!dirH.mkpath(dir))
                {
                    QMessageBox::warning(nullptr, tr("Karte erstellen"), tr("Kann Ausgabeverzeichnis nicht erzeugen"),
                                                  QMessageBox::Ok);
                    return;
                }
            }
            if(gr.m_e<gr.m_w)	// über 180Grad
            {
                CGeoRect gr2=gr;
                gr2.m_e=180.;
                gr.m_w=-179.9999;
                addMaps2ListV(mmList,data,gr2,map);
            }
            maxThreads=qMin(maxThreads,map->m_maxThreads);
            addMaps2ListV(mmList,data,gr,map);
        }
        i++;
    }

    data->m_bpp=0;
    bool inThread=false;
    if(!mmList.isEmpty())
    {
        int anzahlKarten=mmList.size();
        data->m_done=0;
        QProgressDialog progress("", tr("Abbruch"), 0, anzahlKarten*3, nullptr);
        progress.setWindowModality(Qt::WindowModal);
        progress.setValue(0);
        progress.setWindowTitle(tr("Lade %n Vektor-Karten",nullptr,anzahlKarten));

#ifdef QT_DEBUG
        qDebug() << "Kein Multitasking";
        data->m_progD=&progress;
        for(int kNr=0;kNr<anzahlKarten;kNr++)
            loadVMapInThread(mmList[kNr]);	// kein Multitasking zum Debugging
#else
        inThread=true;
        data->m_progD=nullptr;
        qDebug() << "Multitasking mit " << maxThreads << " Threads";
        QThreadPool::globalInstance()->setMaxThreadCount(maxThreads);
        QFutureWatcher<void> fW;
        QObject::connect(&fW, SIGNAL(finished()), &progress, SLOT(reset()));
        QObject::connect(&progress, SIGNAL(canceled()), &fW, SLOT(cancel()));
        QObject::connect(&fW, SIGNAL(progressRangeChanged(int,int)), &progress, SLOT(setRange(int,int)));
        QObject::connect(&fW, SIGNAL(progressValueChanged(int)), &progress, SLOT(setValue(int)));
        fW.setFuture(QtConcurrent::map(mmList,loadVMapInThread));
        progress.exec();
        fW.waitForFinished();
#endif
    }
    if(!inThread)
    {

        QMessageBox msgBox(QMessageBox::Warning,"Vector-Tile","",QMessageBox::Ok|QMessageBox::Yes);
        msgBox.button(QMessageBox::Yes)->setText(tr("Log anzeigen"));

        if((data->errs&ERR_PIXM_TILE)==ERR_PIXM_TILE)
            msgBox.setText( tr("Mindestens ein Tile konnte nicht heruntergeladen werden. S. Logfile"));
        else if((data->errs&ERR_PIXM_TILE_OLD)==ERR_PIXM_TILE_OLD)
            msgBox.setText( tr("Mindestens ein Tile konnte nicht heruntergeladen werden, aber es gab noch eine alte Version. S. Logfile"));
        else if(mmList.isEmpty())
        {
            msgBox.setText( tr("Nichts zu tun"));
            data->errs=1024;
        }
        else if(data->errs>0)
            msgBox.setText( tr("Unbekannter Fehler. S. Logfile"));




        while(!data->m_errTxts.isEmpty())	//CHG: TAHO 2.10c DYJ
        {
            m_log.writeFLS(data->m_errTxts.at(0));
            data->m_errTxts.removeFirst();
        }
        m_log.closeHtml();
        if(data->errs>0)
        {
            if(msgBox.exec()==QMessageBox::Yes)
            {
                QString url="file:///"+m_log.m_path;
                QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));
            }
        }
        data->errs=0;
    }
    delete data;

}


void TahoMainW::OnSelchangeSourceP()
{
    CMapSrc *map;
    if((map=CMapSrc::lookup(CMapSrc::getSrc()))!=nullptr)
    {
        static QString oldName="";
        if(map->hatUnbekannteID())
            CMapSrc::select(oldName);
        oldName=CMapSrc::getSrc();
        if(map->m_url.startsWith("file:///",Qt::CaseInsensitive))
        {
            QString path=map->m_url.mid(8);
            path=QFileDialog::getOpenFileName(this,tr("ein Tile wählen"),path,tr("Tiles(*.png *.jpg);;Alle(*.*)"));
            if(!path.isNull())
            {
                QFileInfo fi(path);
                map->m_ext=QString(".%1").arg(fi.suffix());
                QDir dir=fi.absoluteDir();
                if(dir.cdUp())
                {
                    if(dir.cdUp())
                        map->m_url=QString("file:///%1").arg(dir.absolutePath());
                }
            }
        }
    }

    setAutoPath();
}
void TahoMainW::OnSelchangedMapType(int type)
{
    static bool wasDir=false;
    ui->rb_nbDir->setEnabled(type==0);
    if(type)
    {
        wasDir=ui->rb_nbDir->isChecked();
        if(wasDir)
            ui->rb_nbNr->setChecked(true);
        if(ui->cb_auto->isChecked())
        {
            QFileInfo fi(m_opt.m_prgPath,"Vektor");
            ui->le_out->setText(fi.filePath()+"/");
        }

    }
    else
    {
        if(wasDir)
            ui->rb_nbDir->setChecked(true);
        OnSelchangeSourceP();
    }
}

/*
void TahoMainW::OnDblclkOverlayP(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    int nSelected = pNMListView->iItem;
    if (nSelected >= 0)
    {
        QString ovName=m_ovrCtrlP.GetItemText(nSelected,0);
        CMapSrc::select(ovName,LCC_TOG);
    }
    *pResult = 0;
}

void TahoMainW::OnDblclkOverlayV(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    int nSelected = pNMListView->iItem;
    if (nSelected >= 0)
    {
        QString ovName=m_ovrCtrlV.GetItemText(nSelected,0);
        CMapSrc::select(ovName,LCC_TOG);
    }

    *pResult = 0;
}
*/


/*** ------------------------------------------------------------------------ ***/
/*	void TahoMainW::OnBnClickedBbox()											*/
/*	Ruft Bbox-Tool auf und nimmt Resultat aus Zwischenablage an					*/
/*	Parameter:	keine															*/
/*	Ergebnis:	keines															*/
/*	24.1.14 von Dimitri Junker	DYJ 3.10a										*/
/*** ------------------------------------------------------------------------ ***/

void TahoMainW::OnBnClickedBbox()

{

    QString url=m_opt.m_osmUrl+"bbox-tool/bbox.html";
    QDesktopServices::openUrl(QUrl(url) );

    if(QMessageBox::question(nullptr, tr("BBox-Tool"), tr("Wähle Bereich in Browser-Fenster und kopiere das <bbox...> unten links in die Zwischenablage, dann klicke den Knopf 'OK'"),
                                  QMessageBox::Ok|QMessageBox::Cancel)==QMessageBox::Ok)
    {
        int pos1;
        QClipboard *clipboard = QApplication::clipboard();
        QString url = clipboard->text();
        pos1 = url.indexOf("<bbox") + 6;
        if (pos1>-1)
        {
            // Eine URL wie man sie erhält wenn man auf https://dimitrijunker.lima-city.de/OSM/bbox-tool/bbox.html geht, dort einen Bereich wählt,

            // <bbox e="6.1081" n="50.7914" s="50.761" w="6.0542"/>

            int posN = url.indexOf("n=", pos1) + 3;
            int posS = url.indexOf("s=", pos1) + 3;
            int posE = url.indexOf("e=", pos1) + 3;
            int posW = url.indexOf("w=", pos1) + 3;
            if (posN > 0 && posS > 0 && posE > 0 && posW > 0)
            {
                 QRegExp rx("([-+]?[0-9]*\\.[0-9]+|[-+]?[0-9]+)");
                 int posDbl=rx.indexIn(url.mid(posW));
                 if(posDbl>-1)
                 {
                     double lon=rx.cap(1).toDouble();
                     if(lon<-179.99)
                         lon=-179.9;
                     ui->le_lon1->setText(QString::number(lon,'g',10));
                     posDbl=rx.indexIn(url.mid(posN));
                     if(posDbl>-1)
                     {
                         ui->le_lat1->setText(QString::number(rx.cap(1).toDouble(),'g',10));
                         posDbl=rx.indexIn(url.mid(posE));
                         if(posDbl>-1)
                         {
                             lon=rx.cap(1).toDouble();
                             if(lon>179.99)
                                 lon=179.99;
                             ui->le_lon2->setText(QString::number(lon,'g',10));
                             posDbl=rx.indexIn(url.mid(posS));
                             if(posDbl>-1)
                                 ui->le_lat2->setText(QString::number(rx.cap(1).toDouble(),'g',10));
                         }
                     }
                 }
            }
        }
    }

}
void TahoMainW::OnCancel()
{
    qApp->quit();
}

void TahoMainW::on_actionBeenden_triggered()
{
    qApp->quit();

}
void TahoMainW::OnHelpB()
{
    QString url="file:///"+m_opt.m_prgPath+"Docu/"+tr("liesmich.pdf");
    QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

}
void TahoMainW::changeEvent(QEvent *e)
{
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
Q_DECLARE_METATYPE(CMapSrc*)

void TahoMainW::OnMapChanged(QListWidgetItem* item)
{
    CMapSrc *ms=item->data(Qt::UserRole).value<CMapSrc *>();
    if(item->checkState()==Qt::Checked && ms->hatUnbekannteID())
        item->setCheckState(Qt::Unchecked);
    ms->m_useOvrs[0]=item->checkState()==Qt::Checked;
}
