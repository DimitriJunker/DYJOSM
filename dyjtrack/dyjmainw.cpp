#include <QFileDialog>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include <qdesktopservices.h>
#include <qclipboard.h>
#include <qtextstream.h>


#include "ui_dyjMainW.h"
#include "dyjMainW.h"
#include "cmapsrc.h"
#include "cgpxProper.h"
#include "urldownload.h"
#include <cmath>
#include "flugdialog.h"
#include "cosm.h"
#include "newairpdialog.h"
#include <cdyjtcopy.h>//DYJ DYJTrack 2.04a

#define T_SET "default.dyt"
QString OsmUrl="https://dimitrijunker.lima-city.de/OSM/";   //DYJ DYJTrack 2.06d


/*
 * 15.3.16 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ DYJTrack 2.03b'
 * Die Leseroutine für DYT Files hatte einen Fehler, dadurch ging z.B. das Datum von Flügen verloren.

 * 19.4.16 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ DYJTrack 2.03c'
 * Aus der Fileliste kann man jetzt auch einzelne Files mit der <Entf> Taste löschen
 * Aus der Fileliste kann man jetzt auch alle Files mit der <Umschalt>+<Entf> Taste löschen
 * Aus der Fileliste kann man jetzt auch den Ändern-Dialog durch Doppelklick auf das File/Flug aufrufen

 * 14.4.16 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ DYJTrack 2.03d'
 * Hilfe funktionierte nicht

 * 7.8.16 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ DYJTrack 2.04a'
 * Für Taho durchgeführte Änderungen:
 * DYJ Taho 4.04a
 * DYJ Taho 4.04b

 * 14.5.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ DYJTrack 2.06a'
    Änderte man das Startdatum eines gpx Files hatte dies keine Auswirkung auf die Anzeige und Sortierung
 * 15.5.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ DYJTrack 2.06b'
    SortByDate wurde nicht im dyt gespeichert
 * 09.06.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ DYJTrack 2.06d'
 * Die oche.de hat den Betrieb eingestellt
*/

dytMainW::dytMainW(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::dytMainW)
{

    QDir fi("G:\\Users\\Dimitri\\Dokumente\\DYJ");
    QString pfad=fi.path();


    int areaNr=0;


    ui->setupUi(this);
    m_anzFiles=0;
    m_area.invalidate();
    m_autoAreas[0]=m_autoAreas[1]=true;
    ui->cb_autoArea->setChecked(true);
    m_frames[0]=10;
    m_frames[1]=300;
    ui->tw_Bereich->setCurrentIndex(areaNr);
    for(int i=0;i<4;i++)
        m_lat[i]=m_lon[i]=0.0;
    ui->le_Lat1->setText("0");
    ui->le_Lat2->setText("0");
    ui->le_Lon1->setText("0");
    ui->le_Lon2->setText("0");
    ui->le_rand->setText(QString::number(m_frames[0]));

    m_htmlOpt.m_out=m_opt.m_tmpDir+"maptst/";
    m_htmlOpt.m_templ=m_opt.m_prgPath+"map.html";
    m_htmlOpt.m_descr="";
    m_htmlOpt.m_title="";
    m_htmlOpt.m_zoom=5;
    m_htmlOpt.m_lat=m_htmlOpt. m_lon=0;
    m_htmlOpt.m_oplSrc=OL_TEMPL;	/*DYJ 1.04e */
    m_htmlOpt.m_oplPath="";	/*DYJ 1.04e */


    m_aviOpt.m_out=m_opt.m_tmpDir+"dyt.avi";
    m_aviOpt.m_pixType=PIX_AVI;
    m_aviOpt.m_x=1920;
    m_aviOpt.m_y=1080;
    m_aviOpt.m_fps=25;
    m_aviOpt.m_lineW=3;
    m_aviOpt.m_time[0]=m_aviOpt.m_time[3]=m_aviOpt.m_time[5]=2;
    m_aviOpt.m_time[1]=10;
    m_aviOpt.m_time[4]=20;
    m_aviOpt.m_time[2]=0;
    /*DYJ DYJTrack 2.07e Start*/
    m_aviOpt.m_source=CMapSrc::m_src;
    /*DYJ  Ende; alt:
    m_aviOpt.m_source=SrcName[BAS_MAPNIK_DE];
    */

    m_pictOpt.m_out=m_opt.m_tmpDir+"dyt.png";
    m_pictOpt.m_pixType=PIX_PICT;
    m_pictOpt.m_x=1920;
    m_pictOpt.m_y=1080;
    m_pictOpt.m_lineW=3;

    /*DYJ DYJTrack 2.07e Start*/
    m_pictOpt.m_source=CMapSrc::m_src;
    /*DYJ  Ende; alt:
    m_pictOpt.m_source=SrcName[BAS_MAPNIK_DE];
    */
    ui->cbSortByDate->setChecked(true);
    OnSelchangeFiles();
    ui->progressBar->setValue(0);
    ui->progressBar->repaint();
    ui->tl_action->setText(tr("keine"));
    ui->tl_action->repaint();
    for(int h=-12;h<0;h++)
        ui->cB_TZh->addItem(QString::number(h));
    for(int h=0;h<15;h++)           //eigentlich wäre +0 und -0 nötig, wegen +0h15 und -0h15, da es aber keine -0h15,-0h30, -0h45 gibt ist dies unnötig
        ui->cB_TZh->addItem("+"+QString::number(h));
    ui->cB_TZh->setCurrentIndex(12);
    for(int m=0;m<60;m+=15)
        ui->cB_TZmin->addItem(QString::number(m));
    ui->cB_TZmin->setCurrentIndex(0);
    m_colDef_N.setRgb(255,0,0);
    ui->pbColNorm->setStyleSheet(CGpxProper::bgcString(m_colDef_N));
    m_colDef_F.setRgb(0,255,0);
    ui->pbColFlug->setStyleSheet(bgcString(m_colDef_F));
    m_airpPath[0]=m_opt.m_persPath+"airports.csv";
    m_airpPath[1]=m_opt.m_persPath+"my_airports.csv";
    QFileInfo finf(m_airpPath[0]);
    if(!finf.exists())
        urlDownload::downloadFile("http://www.ourairports.com/data/airports.csv",  m_airpPath[0]);
    ui->filesListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    bool execDone=false;

// default.dyt laden
//DYJ DYJTrack 2.07j 	   m_opt.setDefPathIsPrg(FALSE);
//DYJ DYJTrack 2.07j 	    m_opt.setRelPath(false);	//Absolute Pfade
    m_opt.setAviMax(2.);	/*DYJ 1.08a */
    m_opt.m_pLanguage=&m_language;
    if (m_opt.m_par.isEmpty())
    {
        m_projectPath = m_opt.m_persPath + T_SET;
        execDone = loadDyt(m_projectPath.makePath()) != 0;
        if (!execDone)
        {
            QString hPath = m_opt.m_prgPath + T_SET;
            execDone = loadDyt(hPath);
            if (execDone)
                /*DYJ DYJTrack 2.07j Start*/
               QFile::copy(hPath,m_opt.m_persPath + T_SET);
            /*DYJ  Ende; alt:
            {
                if (m_opt.getDefPathIsPrg())
                    m_projectPath = hPath;
                else
                    saveDyt(m_projectPath.makePath());
            }
                */
        }
    }
    else
        execDone = loadDyt(m_opt.m_par);
    /*DYJ DYJTrack 2.04a Start*/
    m_progName=this->windowTitle();
    if(execDone)
    {
        QString title= m_projectPath.makePath() + " - " + m_progName;
        this->setWindowTitle(title);
    }
    /*DYJ  Ende; alt:
    if(execDone)
    {
        QString title= m_projectPath.makePath() + " - " + windowTitle();
        this->setWindowTitle(title);
    }
    */

    OnSelchangeFiles();
}

QString dytMainW::bgcString(QColor col)
{
    QString ret=QString("background-color: rgb(%1,%2,%3)").arg(col.red()).arg(col.green()).arg(col.blue());
    return ret;
}

dytMainW::~dytMainW()
{
    delete ui;
}

void dytMainW::OnAddgpx()
{
    QStringList fileNames= QFileDialog::getOpenFileNames(this,tr("Lade GPX"),"",tr("GPX-Files(*.gpx)"));
    QStringList list = fileNames;
    int anzFiles=list.count(),i=0;
    ui->progressBar->setRange(0,anzFiles);
    ui->tl_action->setText(tr("Lade %n GPX-Datei(en)",0,anzFiles));
    ui->tl_action->repaint();
    QStringList::Iterator it = list.begin();
    while(it != list.end()) {
        addGpx(*it);
        ui->progressBar->setValue(i++);
        ui->progressBar->repaint();
        ++it;
    }


    OnSelchangeFiles();
    ui->progressBar->setValue(0);
    ui->progressBar->repaint();
    ui->tl_action->setText(tr("keine"));
    ui->tl_action->repaint();
    updateArea(false);
}
void dytMainW::OnSelchangeFiles()
{
    int anzSel=ui->filesListWidget->selectedItems().count();

    ui->but_delGpx->setEnabled(anzSel>0);
    ui->but_delAllGpx->setEnabled(anzSel>0);
    ui->but_chgGpx->setEnabled(anzSel==1);
    ui->but_gpxUp->setEnabled(anzSel==1);
    ui->but_gpxDown->setEnabled(anzSel==1);
}
void dytMainW::onDel()
{
    int select =0;
    if(ui->filesListWidget->selectedItems().count())
    {
        for(int i=ui->filesListWidget->count()-1;i>-1;i--)
        {
            QListWidgetItem * it=ui->filesListWidget->item(i);
            if(it->isSelected())
            {
                select=i;
                deleteGpx(i);
            }
        }
    }
    if(select>=ui->filesListWidget->count())
        select=ui->filesListWidget->count()-1;
    if(select>-1)
        ui->filesListWidget->item(select)->setSelected(true);
    updateArea();


}
void dytMainW::OnDelAll()
{
    while(m_anzFiles)
        deleteGpx(0);
    updateArea();
}

void dytMainW::deleteGpx(int nr)
{
    if(nr>-1 )
    {
        QListWidgetItem * item=ui->filesListWidget->takeItem(nr);
        CGPXInfo *pGi=item->data(Qt::UserRole).value<CGPXInfo *>();
        delete pGi;
        delete item;
        m_anzFiles--;
    }

}
Q_DECLARE_METATYPE(CGPXInfo*)
void dytMainW::updateArea(bool calcArea)
{
    if(	m_anzFiles)
    {
        if(calcArea)
        {
            m_area.invalidate();
            for(int i=0;i<m_anzFiles;i++)
            {
                QListWidgetItem *item=ui->filesListWidget->item(i);
                CGPXInfo *pGi=item->data(Qt::UserRole).value<CGPXInfo *>();
                m_area.add(pGi->m_area);
            }
        }
        for(int aNr=0;aNr<2;aNr++)
        {
            if(m_autoAreas[aNr])
            {
                double frame=(m_area.m_n-m_area.m_s)/100*m_frames[aNr];
                m_lat[aNr*2]=m_area.m_n+frame;
                if(m_lat[aNr*2]>85.051)
                    m_lat[aNr*2]=85.051;
                m_lat[aNr*2+1]=m_area.m_s-frame;
                if(m_lat[aNr*2+1]<-85.051)
                    m_lat[aNr*2+1]=-85.051;
                frame=(m_area.m_e-m_area.m_w)/100*m_frames[aNr];
                m_lon[aNr*2]=m_area.m_w-frame;
                m_lon[aNr*2+1]=m_area.m_e+frame;
                if((m_lon[aNr*2+1]-m_lon[aNr*2])>360)
                {
                    m_lon[aNr*2]=(m_area.m_w+m_area.m_e)/2-180;
                    m_lon[aNr*2+1]=m_lon[aNr*2]+360;
                }
            }
        }
    }
    else
        m_area.invalidate();
    int aNr=ui->tw_Bereich->currentIndex();
    if(aNr>-1 && aNr<2)
    {
        if (m_anzFiles && m_autoAreas[aNr])
        {
            ui->le_Lat1->setText(QString::number(m_lat[aNr*2],'g',10));
            ui->le_Lat2->setText(QString::number(m_lat[aNr*2+1],'g',10));
            ui->le_Lon1->setText(QString::number(m_lon[aNr*2],'g',10));
            ui->le_Lon2->setText(QString::number(m_lon[aNr*2+1],'g',10));
        }
        else
        {
            m_lat[aNr * 2] = ui->le_Lat1->text().toDouble();
            m_lat[aNr * 2 + 1] = ui->le_Lat2->text().toDouble();
            m_lon[aNr * 2] = ui->le_Lon1->text().toDouble();
            m_lon[aNr * 2 + 1] = ui->le_Lon2->text().toDouble();
        }
    }
    m_htmlOpt.m_lat=(m_lat[0]+m_lat[1])/2;
    m_htmlOpt.m_lon=(m_lon[0]+m_lon[1])/2;
    double zoom1,zoom2;
    if(m_lon[0]==m_lon[1])
        zoom1=17;
    else
        zoom1=10.7-log(fabs(m_lon[0]-m_lon[1]))/log(2);
    if(m_lat[0]==m_lat[1])
        zoom2=17;
    else
        zoom2=10.2-log(fabs(m_lat[0]-m_lat[1]))/log(2);
    m_htmlOpt.m_zoom=qMin(qMin((int)zoom1,(int)zoom2),17);


}
void dytMainW::OnUp()
{
    int index = ui->filesListWidget->currentRow();
    QListWidgetItem *currentItem = ui->filesListWidget->takeItem(index);
    ui->filesListWidget->insertItem(index-1, currentItem);
    ui->filesListWidget->setCurrentRow(index-1);
    ui->cbSortByDate->setChecked(false);
 }
void dytMainW::OnDown()
{
    int index = ui->filesListWidget->currentRow();
    QListWidgetItem *currentItem = ui->filesListWidget->takeItem(index);
    ui->filesListWidget->insertItem(index+1, currentItem);
    ui->filesListWidget->setCurrentRow(index+1);
    ui->cbSortByDate->setChecked(false);
 }

void dytMainW::OnSortByDate()
{
    if(ui->cbSortByDate->isChecked())
        ui->filesListWidget->sortItems();

}

void dytMainW::OnTZChanged()
{
    updateList();

}
void dytMainW::updateList()
{
    if(	m_anzFiles)
    {
        int offsH,offsM;
        offsH=ui->cB_TZh->currentIndex()-12;
        offsM=ui->cB_TZmin->currentIndex()*15;
        if (offsH<0)
            m_tZone = offsH*60-offsM;
        else
            m_tZone = offsH*60+offsM;
        for(int i=0;i<m_anzFiles;i++)
        {
            QListWidgetItem *item=ui->filesListWidget->item(i);
            CGPXInfo *pGi=item->data(Qt::UserRole).value<CGPXInfo *>();
            item->setText(getTextGpx(pGi));
        }
    }
}
void dytMainW::OnColN()
{

    QColor color = QColorDialog::getColor(m_colDef_N, this);
    if(color.isValid())
    {
        m_colDef_N=color;
        ui->pbColNorm->setStyleSheet(bgcString(m_colDef_N));
    }
}

void dytMainW::OnColF()
{
    QColor color = QColorDialog::getColor(m_colDef_F, this);
    if(color.isValid())
    {
        m_colDef_F=color;
        ui->pbColFlug->setStyleSheet(bgcString(m_colDef_F));
    }

}

/*** ------------------------------------------------------------------------ ***/
/*	öffnet Formular zum hinzufügen einer Flugstrecke 							*/
/*																				*/
/*	Parameter:	keine															*/
/*	Ergebnis:	keines															*/
/*	06.11 von Dimitri Junker	DYJ 1.04g										*/
/*** ------------------------------------------------------------------------ ***/

void dytMainW::OnFly()
{
    flugDialog dlg;
    dlg.setStart(QDateTime::currentDateTime());
    dlg.exec();

    if(dlg.result()==QDialog::Accepted)
    {
        QString strecke=dlg.getStrecke();
        QDateTime start=dlg.getStart();
        if(!start.isValid())
            start=QDateTime::currentDateTime();
        start.setTimeSpec(Qt::OffsetFromUTC);
        start.setUtcOffset(m_tZone*60);
        addFlight(dlg.exportGpx,strecke,start);
    }
    OnSelchangeFiles();
    ui->tl_action->setText(tr("keine"));
    ui->tl_action->repaint();
    updateArea(false);

}
/*** ------------------------------------------------------------------------ ***/
/*	Fügt eine Flugstrecke ein													*/
/*																				*/
/*	Parameter:	keine															*/
/*	Ergebnis:	keines															*/
/*	06.11 von Dimitri Junker	DYJ 1.04g										*/
/*** ------------------------------------------------------------------------ ***/
void dytMainW::addFlight(bool exportGpx, QString route, QDateTime start, QString name, TrackCol *pCol, bool rev)
{
    TrackCol col;
    if(pCol)
        col=*pCol;
    CGPXInfo *pGi=new(CGPXInfo);
    QString attr,trkpt;
    pGi->m_dateTime=start;
    int nach=route.indexOf('>');
    if(nach>1)
    {
        CGeoPoint gPt1=airp(route.left(nach-1)),gPt2;
        gPt2=airp(route.mid(nach+1));
        if(gPt1.m_lat<1000 &&gPt1.m_lon<1000&&gPt2.m_lat<1000&&gPt2.m_lon<1000)/*DYJ 1.05c */
        {
            pGi->m_area.add(gPt1);
            pGi->m_trkPts.append(gPt1);

            addGK(pGi,gPt1,gPt2);
        }
    }
    if(pGi->m_trkPts.isEmpty())
    {
        delete pGi;
        return;
    }
    if(exportGpx)
    {
        QString fileName= QFileDialog::getSaveFileName(this,tr("Speicher GPX"),"",tr("GPX-Files(*.gpx)"));

        if(!fileName.isEmpty())
            pGi->writeGPX(fileName);
        delete pGi;
    }
    else
    {
        m_area.add(pGi->m_area);
        pGi->m_route=route;
        if(!name.isEmpty())
            pGi->m_name=name;
        else
            pGi->m_name=route;
        pGi->m_rev=rev;
        pGi->m_col=col;
        addGpxI(pGi);
        m_anzFiles++;
    }

}



/*** ------------------------------------------------------------------------ ***/
/*	rekursives Hinzufügen von Punkten entlang eines Großkreises 				*/
/*	Auflösung: 1 Grad															*/
/*	Parameter:	pGi:	da sollen die Punkte rein								*/
/*				start und end: Start-und Endpunkt								*/
/*	Ergebnis:	keines															*/
/*	06.11 von Dimitri Junker	DYJ 1.04g										*/
/*** ------------------------------------------------------------------------ ***/

void dytMainW::addGK(CGPXInfo *pGi, CGeoPoint start, CGeoPoint end)
{
    double dlat=start.m_lat-end.m_lat;
    double dlon=start.m_lon-end.m_lon;
    if(dlon>180)
        dlon=360-dlon;
    if(dlon<-180)
        dlon=360+dlon;
    double ds=dlat*dlat+dlon*dlon;
    if(ds<1)
    {
        pGi->m_area.add(end);
        pGi->m_trkPts.append(end);
    }
    else
    {
        CGeoPoint mitte;
        double x,y,z,r,fkt=PI/180;
        x=cos(start.m_lat*fkt)*cos(start.m_lon*fkt)+cos(end.m_lat*fkt)*cos(end.m_lon*fkt);
        y=cos(start.m_lat*fkt)*sin(start.m_lon*fkt)+cos(end.m_lat*fkt)*sin(end.m_lon*fkt);
        z=sin(start.m_lat*fkt)+sin(end.m_lat*fkt);
        r=sqrt(x*x+y*y+z*z);

        mitte.m_lat=asin(z/r)/fkt;
        mitte.m_lon=atan2(y,x)/fkt;

        addGK(pGi,start,mitte);
        addGK(pGi,mitte,end);
    }
}


/*** ------------------------------------------------------------------------ ***/
/*	ermittelt Koordinaten eines Flughafens, entweder aus den angegebenen 		*/
/*	Koordinaten oder aus dem Flughafen Code und der airports.csv Liste			*/
/*	Parameter:	Strecke															*/
/*	Ergebnis:	Koordinaten oder 1000,1000 falls Code unbekannt					*/
/*	06.11 von Dimitri Junker	DYJ 1.04g										*/
/*** ------------------------------------------------------------------------ ***/

CGeoPoint dytMainW::airp(QString name_c)
{
    static QMap<QString,CGeoPoint> airports;
    name_c=name_c.trimmed();
    CGeoPoint ret;
    int pos=name_c.indexOf('(');

    if(pos>-1)
    {
        QRegExp rx("([-+]?[0-9]*\\.[0-9]+|[-+]?[0-9]+)");
        int posDbl=rx.indexIn(name_c.mid(pos));
        if(posDbl>-1)
        {
            ret.m_lat=rx.cap(1).toDouble();
            ret.m_lon=rx.cap(2).toDouble();
        }
     }
    else
    {
        if(airports.isEmpty())
        {
            CGeoPoint aktPt;
 //           airports.InitHashTable(99971);
            for(int i=0;i<2;i++)
            {

                QFile fListe(m_airpPath[i]);
                if(fListe.open(QIODevice::ReadOnly))
                {
                    int filePos=0,fileLen=fListe.size(),dispAt;
                    if(i)
                        dispAt=1000000000;
                    else
                    {
                        dispAt=0;
                        ui->progressBar->setRange(0,fileLen);
                        ui->tl_action->setText(tr("Lade airports.csv"));
                        ui->tl_action->repaint();
                    }
                    QTextStream liste(&fListe);
                    if(!i)
                        liste.readLine(); // Titelzeile
                    while(!liste.atEnd())
                    {
                        QString token[14];
                        QString zeile=liste.readLine();
                        filePos+=zeile.size();
                        if(filePos>dispAt)
                        {
                            ui->progressBar->setValue(filePos);
                            ui->progressBar->repaint();
                            dispAt+=fileLen/100;
                        }
                        int pos=0;
                        for(int i=0;i<14;i++)
                        {
                            int tokenLen;
                            if(zeile[pos]=='\"')
                            {
                                tokenLen=zeile.mid(++pos).indexOf("\"");
                                token[i]=zeile.mid(pos,tokenLen);
                                pos+=tokenLen+1;
                                tokenLen=zeile.mid(pos).indexOf(",");
                            }
                            else
                            {
                                tokenLen=zeile.mid(pos).indexOf(",");
                                token[i]=zeile.mid(pos,tokenLen);
                            }
                            pos+=tokenLen+1;
                        }
                        aktPt.init(token[4].toDouble(),token[5].toDouble());
                        if(token[1].size()==4)
                            airports.insert(token[1],aktPt);
                        if(token[13].size()==3)
                            airports.insert(token[13],aktPt);
                    }
                    fListe.close();
                }
            }
        }
        ui->progressBar->setValue(0);
        ui->progressBar->repaint();
        ui->tl_action->setText(tr("keine"));
        ui->tl_action->repaint();
        name_c=name_c.toUpper();
        QMap<QString,CGeoPoint>::iterator it=airports.find(name_c);
        if(it==airports.end())
        {
            newAirpDialog dlg;
            dlg.setName(name_c);
            dlg.exec();

            if(dlg.result()==QDialog::Accepted)
            {
                ret=dlg.getCoord();
                QFile fMyListe(m_airpPath[1]);
                if(fMyListe.open(QIODevice::Append))
                {
                    QTextStream liste(&fMyListe);
                    liste << ",";
                    liste << name_c;
                    liste << ",,,";
                    liste << ret.m_lat;
                    liste << ",";
                    liste << ret.m_lon;
                    liste << ",,,,,,,,";
                    liste << name_c;
                    liste << "\n";
                    fMyListe.close();
                    airports.insert(name_c,ret);
                }
            }
            else
                ret.m_lat=ret.m_lon=1000;
        }
        else
            ret=it.value();
    }
    return ret;

}

void dytMainW::addGpx(QString pfad,QString name,TrackCol *pCol,QDateTime odt,bool rev)
{
    TrackCol col;
    if(pCol)
        col=*pCol;
    CGPXInfo *pGi=new(CGPXInfo);
    CXmlFile xFile;
    int aktPos=0;
    if(!xFile.read2Buf(pfad))
        return;
    QString attr,trkpt;
    xFile.readValB(trkpt,"trkpt");
    if(!odt.isValid())
    {
       QString time;
        xFile.readValB(time,trkpt,"time");
 //       int year,month,day,hour,min,sec;
        pGi->m_dateTime=QDateTime::fromString(time,"yyyy-MM-dd'T'hh:mm:ss'Z'");          // <time>2013-08-17T07:59:35Z</time>
    }
    else
        pGi->m_dateTime=odt;
     if(!pGi->m_dateTime.isValid())
    {
        QFileInfo fi(pfad);
        pGi->m_dateTime=fi.lastModified();
    }
    CGeoPoint gPtLst(0,0);
    while(!(attr=xFile.readAttr("trkpt",&aktPos)).isEmpty())
    {
        CGeoPoint gPt(xFile.readAttrValD(attr,"lat"),xFile.readAttrValD(attr,"lon"));
        pGi->m_area.add(gPt);
        pGi->m_trkPts.append(gPt);
        gPtLst=gPt;
    }
    if(pGi->m_trkPts.isEmpty())
    {
        delete pGi;
        return;
    }
    m_area.add(pGi->m_area);
    pGi->m_pfad=pfad;
    if (name.isEmpty())
    {
        QFileInfo fi(pfad);
        name = fi.fileName();
    }
    pGi->m_name=name;
    pGi->m_rev=rev;
    pGi->m_col=col;
    addGpxI(pGi);
    m_anzFiles++;

}
void dytMainW::addGpxI(CGPXInfo *pGi, bool sort)
{
    QListWidgetItem *item=new QListWidgetItem;
    item->setData(Qt::UserRole,QVariant::fromValue<CGPXInfo*>(pGi));
    item->setText(getTextGpx(pGi));

    ui->filesListWidget->addItem(item);
     ui->filesListWidget->repaint();
    if(sort && ui->cbSortByDate->isChecked())
        ui->filesListWidget->sortItems();
 //    UpdateData(FALSE);
//   Invalidate();h
//    UpdateWindow();
}
QString dytMainW::getTextGpx(CGPXInfo *pGi)
{
    QDateTime locTime=pGi->m_dateTime;
    locTime.setTimeSpec(Qt::OffsetFromUTC);
    locTime.setUtcOffset(m_tZone*60);
    QString text=locTime.toString("yyyy.MM.dd HH:mm:ss");
 //           .Format("%Y.%m.%d %H:%M:%S ");
    text +="  "+pGi->m_name;
    return text;
}
void dytMainW::OnChgGpx()
{
    int sel = ui->filesListWidget->currentRow();
    while(sel>-1)
    {
        QListWidgetItem * item=ui->filesListWidget->item(sel);
        CGPXInfo *pGiSel=item->data(Qt::UserRole).value<CGPXInfo *>();
        CGpxProper dlg;
        dlg.init(pGiSel,m_colDef_N,m_colDef_F);
        dlg.exec();
        if(dlg.result()==QDialog::Accepted)
        {
            switch(dlg.getVal(pGiSel))
            {
                case 0:     // ok
                    sel=-1;
                    break;
                case -1:    // <<
                    if(sel)
                        sel--;
                    else
                        sel=-1;
                    break;
                case 1:     // >>
                    if(sel<(m_anzFiles-2))
                        sel++;
                    else
                        sel=-1;
                    break;
            }
        }
        else
            sel=-1;     //Abbruch
        /*DYJ DYJTrack 2.06a Start*/
        item->setText(getTextGpx(pGiSel));
        OnSortByDate();
        /*DYJ  Ende; */
    }
}
void dytMainW::OnSelChgArea()
{
    int areaNr=ui->tw_Bereich->currentIndex();

    //aktuelle Werte sichern
    m_lat[2-2*areaNr]=ui->le_Lat1->text().toDouble();
    m_lat[3-2*areaNr]=ui->le_Lat2->text().toDouble();
    m_lon[2-2*areaNr]=ui->le_Lon1->text().toDouble();
    m_lon[3-2*areaNr]=ui->le_Lon2->text().toDouble();

    //neue Werte setzen
    CpyArea2Form();
}
/*** ------------------------------------------------------------------------ ***/
/*	void dytMainW::OnManChgArea()                               				*/
/*	Manuell eingegebene Werte für die Area werden ausgelesen    				*/
/*	Parameter:	keine															*/
/*	Ergebnis:	keines															*/
/*	4.6.17 von Dimitri Junker	DYJ 2.05c           							*/
/*** ------------------------------------------------------------------------ ***/
void dytMainW::OnManChgArea()
{
    int areaNr=ui->tw_Bereich->currentIndex();

    //aktuelle Werte sichern
    m_lat[2*areaNr]=ui->le_Lat1->text().toDouble();
    m_lat[2*areaNr+1]=ui->le_Lat2->text().toDouble();
    m_lon[2*areaNr]=ui->le_Lon1->text().toDouble();
    m_lon[2*areaNr+1]=ui->le_Lon2->text().toDouble();
    if(m_lat[2*areaNr]<m_lat[2*areaNr+1])
    {
        ui->le_Lat1->setText(QString::number(m_lat[areaNr*2+1]));
        ui->le_Lat2->setText(QString::number(m_lat[areaNr*2]));
        OnManChgArea();

    }
}

void dytMainW::CpyArea2Form()
{
    int areaNr=ui->tw_Bereich->currentIndex();

    ui->cb_autoArea->setChecked(m_autoAreas[areaNr]);
    ui->le_rand->setText(QString::number(m_frames[areaNr]));

    ui->le_Lat1->setText(QString::number(m_lat[areaNr*2]));
    ui->le_Lat2->setText(QString::number(m_lat[areaNr*2+1]));
    ui->le_Lon1->setText(QString::number(m_lon[areaNr*2]));
    ui->le_Lon2->setText(QString::number(m_lon[areaNr*2+1]));
    enableArea();
}
void dytMainW::enableArea()
{
    bool enable=ui->cb_autoArea->isChecked()==false;
    ui->le_Lat1->setEnabled(enable);
    ui->le_Lat2->setEnabled(enable);
    ui->le_Lon1->setEnabled(enable);
    ui->le_Lon2->setEnabled(enable);

}
void dytMainW::OnChgRand()
{
    int areaNr=ui->tw_Bereich->currentIndex();
    m_frames[areaNr]=ui->le_rand->text().toDouble();
    updateArea(false);
}
void dytMainW::OnChgAutoArea()
{
    int areaNr=ui->tw_Bereich->currentIndex();
    m_autoAreas[areaNr]=ui->cb_autoArea->isChecked();
    enableArea();
    updateArea(false);

}
void dytMainW::OnLoadDyt()
{
    QString path= QFileDialog::getOpenFileName(this,tr("Lade dyt Files"),"",tr("DYT-Files(*.dyt)"));

    if(!path.isEmpty())
    {
        int areaNr=ui->tw_Bereich->currentIndex();
        if(areaNr)
            loadDyt(path,DYT_ZOOM);
        else
            loadDyt(path,DYT_MAIN);
        m_autoAreas[areaNr]=false;
        ui->cb_autoArea->setChecked(false);
        CpyArea2Form();
    }
}

void dytMainW::OnSaveDyt()
{
    QString path= QFileDialog::getSaveFileName(this,tr("Speicher dyt Files"),"",tr("DYT-Files(*.dyt)"));

    if(!path.isEmpty())
    {
        QFileInfo fi(path);
        if(fi.exists())	// prüfen ob das File bereits besteht
        {
            QMessageBox::StandardButton overwrite;
            overwrite = QMessageBox::question(nullptr, tr("File überschreiben?"), tr("Das File besteht bereits, soll es überschrieben werden?"),
                                          QMessageBox::Yes|QMessageBox::No);

            if(overwrite!=QMessageBox::No)
                return;
        }
        FILE *gDyt;
        fopen_s(&gDyt,path.toLatin1(), "w");
        if(gDyt)
        {
            fprintf(gDyt,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
            fprintf(gDyt,"<dyjtrack version=\"1.0\" creator=\"dyjtrack.exe - http://www.dimitri-junker.de/html/openstreetmap.html\">\n");

            int areaNr=ui->tw_Bereich->currentIndex()*2;

            fprintf(gDyt,"\t<area>\n");
            fprintf(gDyt,"\t\t<auto>0</auto>\n");
            fprintf(gDyt,"\t\t<frame>0</frame>\n");
            fprintf(gDyt,"\t\t<lat1>%f</lat1>\n",m_lat[areaNr]);
            fprintf(gDyt,"\t\t<lat2>%f</lat2>\n",m_lat[areaNr+1]);
            fprintf(gDyt,"\t\t<lon1>%f</lon1>\n",m_lon[areaNr]);
            fprintf(gDyt,"\t\t<lon2>%f</lon2>\n",m_lon[areaNr+1]);
            fprintf(gDyt,"\t</area>\n");

            fprintf(gDyt,"</dyjtrack>\n");
            fclose(gDyt);
        }
    }
 }

void dytMainW::OnBbox()
{
    /*DYJ DYJTrack 2.06d Start*/
    QString url=OsmUrl+"bbox-tool/bbox.html";
    QDesktopServices::openUrl(QUrl(url) );
    /*DYJ  Ende; alt:
    QDesktopServices::openUrl(QUrl("http://www.oche.de/~junker/OSM/bbox-tool/bbox.html") );
    */
    QMessageBox::StandardButton bbox;
    bbox = QMessageBox::question(nullptr, tr("Bereich wählen"), tr("Wähle Bereich in Browser-Fenster und kopiere das <bbox...> unten links in die Zwischenablage, dann klicke den Knopf 'OK'"),
                                  QMessageBox::Yes|QMessageBox::Cancel);

    if(bbox==QMessageBox::Yes)
    {
        int pos1;
        QClipboard *clipboard = QApplication::clipboard();
        QString url = clipboard->text();
        pos1 = url.indexOf("<bbox") + 6;
        if(pos1>-1)
        {
// Eine URL wie man sie erhält wenn man auf https://dimitrijunker.lima-city.de/OSM/bbox-tool/bbox.html geht, dort einen Bereich wählt,

// <bbox e="6.1081" n="50.7914" s="50.761" w="6.0542"/>

            int posN = url.indexOf("n=", pos1) + 3;
            int posS = url.indexOf("s=", pos1) + 3;
            int posE = url.indexOf("e=", pos1) + 3;
            int posW = url.indexOf("w=", pos1) + 3;
            if (posN > 0 && posS > 0 && posE > 0 && posW > 0)
            {
                int areaNr = ui->tw_Bereich->currentIndex();
                QRegExp rx("([-+]?[0-9]*\\.[0-9]+|[-+]?[0-9]+)");
                int posDbl=rx.indexIn(url.mid(posW));
                if(posDbl>-1)
                {
                    m_lon[2 * areaNr]=rx.cap(1).toDouble();
                    posDbl=rx.indexIn(url.mid(posN));
                    if(posDbl>-1)
                    {
                        m_lat[2 * areaNr]=rx.cap(1).toDouble();
                        posDbl=rx.indexIn(url.mid(posE));
                        if(posDbl>-1)
                        {
                            m_lon[2 * areaNr + 1]=rx.cap(1).toDouble();
                            posDbl=rx.indexIn(url.mid(posS));
                            if(posDbl>-1)
                            {
                                m_lat[2 * areaNr+1]=rx.cap(1).toDouble();
                                if (m_lon[2 * areaNr]<-179.99)
                                    m_lon[2 * areaNr] = -179.9;
                                if (m_lon[2 * areaNr + 1]>179.99)
                                    m_lon[2 * areaNr + 1] = 179.99;
                                m_autoAreas[areaNr]=false;
                                ui->cb_autoArea->setChecked(false);
                                enableArea();
                            }

                        }

                    }
                }

            }
        }
    }
    CpyArea2Form();
}



bool dytMainW::loadDyt(const QString &path, int what)
{
    CXmlFile xDyt;
    QString source;
    if(!xDyt.read2Buf(path))
        return false;
    QString sDyt;
    xDyt.readValB(sDyt,"dyjtrack");
    if(sDyt.isEmpty())
    {
        QString msg=QString(tr("%1 scheint kein gültiges dyt-File zu sein")).arg(path);
        QMessageBox::warning(nullptr, tr("File nicht gefunden"),msg,
                                      QMessageBox::Ok);
        return false;
    }

    if((what&DYT_OPT)!=0)
    {
        QString sOpt,tmp;	//CHG: TAHO 2.10f DYJ
        xDyt.readValB(sOpt,sDyt,"options");
        /*DYJ DYJTrack 2.07j Start*/
        if(sOpt.isEmpty())
            sOpt=sDyt;
        /*DYJ  Ende; alt:
        if(sOpt.isEmpty())
        {
            sOpt=sDyt;
            m_opt.setRelPath(false);
        }
        else
            m_opt.setRelPath(xDyt.readVal_intDef(sOpt,"relPath",0));
        */
        QString lang="";
        if(xDyt.readValB(lang,sOpt,"language"))
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
                m_language.load("dyjtrack_"+lang);
                qApp->installTranslator(&m_language);
            }
         }
         m_opt.setAviMax(xDyt.readVal_dblDef(sOpt, "aviMaxLen", 2.));
         ui->cbSortByDate->setChecked(xDyt.readVal_intDef(sOpt, "sortByDate", 1));   /*DYJ DYJTrack 2.06b */
         //DYJ DYJTrack 2.06d Start
         if(CXmlFile::readValB(tmp,sOpt,"OsmUrl") && !tmp.isEmpty())
             OsmUrl=tmp;
         //DYJ  Ende;

    }
    if((what&DYT_FILES)!=0)
    {
        QString gpxfiles;
        xDyt.readValB(gpxfiles,sDyt,"gpxfiles");
        if(!gpxfiles.isEmpty())
        {
            m_colDef_N=QColor::fromRgb(xDyt.readVal_intDef(gpxfiles,QString("color"),m_colDef_N.rgb()&0xffffff));
            m_colDef_F=QColor::fromRgb(xDyt.readVal_intDef(gpxfiles,"colorF",m_colDef_F.rgb()&0xffffff));
            QString gpxPath="",tmp,gpxRoute="";
            xDyt.readValB(tmp,sDyt,"timezone");
            QRegExp rx("([-+]?[0-9]+)");
            int posInt=rx.indexIn(tmp);
            if(posInt>1)
            {
                int tzh,tzm;
                tzh=rx.cap(1).toInt();
                tzm=rx.cap(2).toInt();
                if(tzh<0)
                    m_tZone=tzh*60-tzm;
                else
                    m_tZone=tzh*60+tzm;
            }
            QString gpxfile;
            int aktPos=0;
            int anz_gpx=xDyt.readVal_intDef(gpxfiles,QString("anzgpxfiles"),-1);
            if(anz_gpx==-1)
            {
                ui->progressBar->setRange(0,gpxfiles.size());
                ui->tl_action->setText(tr("Lade GPX-Dateien"));
            }
            else
            {
                ui->progressBar->setRange(0,anz_gpx);
                ui->tl_action->setText(tr("Lade %n GPX-Dateie(n)",0,anz_gpx));
            }

            ui->tl_action->repaint();
            while(xDyt.readValB(gpxfile,gpxfiles,"gpxfile",&aktPos))
            {
                if(anz_gpx==-1)
                    ui->progressBar->setValue(aktPos);
                else
                    ui->progressBar->setValue(ui->progressBar->value()+1);
                ui->progressBar->repaint();

                if(xDyt.readValB(tmp,gpxfile,"path"))
                {
//DYJ DYJTrack 2.07j 	                    if(m_opt.getRelPath())
//DYJ DYJTrack 2.07j 	                        gpxPath=CPath::absPath(tmp,gpxPath);
 //DYJ DYJTrack 2.07j 	                   else
                    gpxPath=tmp;
                    gpxRoute="";
                }
                else	/*DYJ 1.05b */
                {
                    xDyt.readValB(gpxRoute,gpxfile,"route");
                    gpxPath="";
                }
                QString name;
                xDyt.readValB(name,gpxfile,"name");
                int iCol=xDyt.readVal_intDef(gpxfile,"color",0x1000000);
                TrackCol col;
                /*DYJ DYJTrack 2.06c Start*/
                switch(iCol)
                {
                case 0x1000000:
                    col.m_useTrColSp=TC_AUTO;
                    break;
                case 0x2000000:
                    col.m_useTrColSp=TC_STD;
                    break;
                case 0x3000000:
                    col.m_useTrColSp=TC_FLUG;
                    break;
                default:
                    col.setCol(iCol);
                }

                /*DYJ  Ende; alt:
                if(iCol!=0x1000000)
                    col.setCol(iCol);
                */
                xDyt.readValB(tmp,gpxfile,"start");
                QDateTime odt;
                /*DYJ DYJTrack 2.03b Start*/
                odt=QDateTime::fromString(tmp,"d.M.yyyy H:m:s");
                /*DYJ  Ende; alt:
                odt.fromString(tmp);
                */
                int rev=xDyt.readVal_intDef(gpxfile,"reverse",0);
                if(!gpxPath.isEmpty())
                    addGpx(gpxPath,name,&col,odt,rev!=0);
                else if(!gpxRoute.isEmpty())
                    addFlight(false,gpxRoute,odt,name,&col,rev!=0);
            }
        }
    }
    if((what&DYT_AREAS)!=0)
    {
        QString areas[3];
        xDyt.readValB(areas[0],sDyt,"mainarea");
        xDyt.readValB(areas[1],sDyt,"zoomarea");
        xDyt.readValB(areas[2],sDyt,"area");
        if(!areas[2].isEmpty())
            areas[ui->tw_Bereich->currentIndex()]=areas[2];
        if((what&DYT_ZOOM)==0)
            areas[1]="";
        if((what&DYT_MAIN)==0)
            areas[0]="";
        for(int aNr=0;aNr<2;aNr++)
        {
            if(!areas[aNr].isEmpty())
            {
                m_autoAreas[aNr]=xDyt.readVal_intDef(areas[aNr],"auto",1);
                m_frames[aNr]=xDyt.readVal_intDef(areas[aNr],"frame",1);
                if(!m_autoAreas[aNr])
                {
                    m_lat[2*aNr]=xDyt.readVal_dblDef(areas[aNr],"lat1",m_lat[2*aNr]);
                    m_lat[2*aNr+1]=xDyt.readVal_dblDef(areas[aNr],"lat2",m_lat[2*aNr+1]);
                    m_lon[2*aNr]=xDyt.readVal_dblDef(areas[aNr],"lon1",m_lon[2*aNr]);
                    m_lon[2*aNr+1]=xDyt.readVal_dblDef(areas[aNr],"lon2",m_lon[2*aNr+1]);
                    ui->le_Lat1->setText(QString::number(m_lat[aNr*2],'g',10));
                    ui->le_Lat2->setText(QString::number(m_lat[aNr*2+1],'g',10));
                    ui->le_Lon1->setText(QString::number(m_lon[aNr*2],'g',10));
                    ui->le_Lon2->setText(QString::number(m_lon[aNr*2+1],'g',10));
                }
            }
        }
    }
    updateArea(false);
    if((what&DYT_MAPS)!=0)
    {
        QString htmlmap;
        xDyt.readValB(htmlmap,sDyt,"htmlmap");
        if(!htmlmap.isEmpty())
        {
            m_htmlOpt.m_lat=xDyt.readVal_dblDef(htmlmap,"lat",0);
            m_htmlOpt.m_lon=xDyt.readVal_dblDef(htmlmap,"lon",0);
            m_htmlOpt.m_zoom=xDyt.readVal_intDef(htmlmap,"zoom",12);
            xDyt.readValB(m_htmlOpt.m_title,htmlmap,"title");
            xDyt.readValB(m_htmlOpt.m_descr,htmlmap,"descr");
            xDyt.readValB(m_htmlOpt.m_templ,htmlmap,"template");
            xDyt.readValB(m_htmlOpt.m_oplPath,htmlmap,"openlayers");
            if(m_htmlOpt.m_oplPath.size()<2)
            {
                m_htmlOpt.m_oplSrc=m_htmlOpt.m_oplPath.toInt();
                m_htmlOpt.m_oplPath="";
            }
            else
                m_htmlOpt.m_oplSrc=OL_URL;
            QString tmp;
            xDyt.readValB(tmp,htmlmap,"output");


//DYJ DYJTrack 2.07j 	            if(m_opt.getRelPath())
//DYJ DYJTrack 2.07j 	                m_htmlOpt.m_out=CPath::absPath(tmp,path);
//DYJ DYJTrack 2.07j 	            else
            m_htmlOpt.m_out=tmp;

        }
// Overlays deaktivieren
        CMapSrc::resetOvr();


//Avimap
        QString avimap;
        xDyt.readValB(avimap,sDyt,"avimap");
        if(!avimap.isEmpty())
        {
            m_aviOpt.m_x=xDyt.readVal_intDef(avimap,"x",1920);
            m_aviOpt.m_y=xDyt.readVal_intDef(avimap,"y",1080);
            m_aviOpt.m_fps=xDyt.readVal_intDef(avimap,"fps",25);
            for(int i=0;i<6;i++)
            {
                QString tag=QString("time_%1").arg(i);
                m_aviOpt.m_time[i]=xDyt.readVal_intDef(avimap,tag,m_aviOpt.m_time[i]);
            }
            xDyt.readValB(m_aviOpt.m_source,avimap,"source");

            int aktPos=0;
            while(xDyt.readValB(source,avimap,"overlayName",&aktPos))
                CMapSrc::select(source);
            QString tmp;
            xDyt.readValB(tmp,avimap,"output");
//DYJ DYJTrack 2.07j 	            if(m_opt.getRelPath())
//DYJ DYJTrack 2.07j 	                m_aviOpt.m_out=CPath::absPath(tmp,path);
//DYJ DYJTrack 2.07j 	            else
            m_aviOpt.m_out=tmp;
        }
//Pictmap
        QString pictmap;
        xDyt.readValB(pictmap,sDyt,"pictmap");
        if(!pictmap.isEmpty())
        {
            m_pictOpt.m_x=xDyt.readVal_intDef(pictmap,"x",1920);
            m_pictOpt.m_y=xDyt.readVal_intDef(pictmap,"y",1080);
            xDyt.readValB(m_pictOpt.m_source,pictmap,"source");
            int aktPos=0;

            while(xDyt.readValB(source,pictmap,"overlayName",&aktPos))
                CMapSrc::select(source);
            QString tmp;
            xDyt.readValB(tmp,pictmap,"output");
//DYJ DYJTrack 2.07j 	            if(m_opt.getRelPath())
//DYJ DYJTrack 2.07j 	                m_pictOpt.m_out=CPath::absPath(tmp,path);
//DYJ DYJTrack 2.07j 	            else
            m_pictOpt.m_out=tmp;
        }
    }
    if(what==DYT_ALL)
        m_projectPath=path;
    ui->progressBar->setValue(0);
    ui->progressBar->repaint();
    ui->tl_action->setText(tr("keine"));
    ui->tl_action->repaint();
    return true;
}

void dytMainW::saveDyt(QString path,QString *expDir)
{
    QFile gDyt(path);
    if(gDyt.open(QIODevice::WriteOnly))
    {
        QTextStream oDyt(&gDyt);
        oDyt << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
        oDyt << "<dyjtrack version=\"1.0\" creator=\"dyjtrack.exe - http://www.dimitri-junker.de/html/openstreetmap.html\">\n";
        oDyt << "\t<options>\n";
        //:put here the abreviation of the new Language (for ex. en for english, fr for frensh, es for spanish,...
        oDyt << "\t\t<language>"<< tr("de") << "</language>\n";
//DYJ DYJTrack 2.07j 	        oDyt << "\t\t<relPath>" <<  (m_opt.getRelPath() || (expDir != nullptr)) << "</relPath>\n";
        oDyt << "\t\t<aviMaxLen>" << m_opt.getAviMax() << "</aviMaxLen>\n";
        oDyt << "\t\t<sortByDate>" << ui->cbSortByDate->isChecked() << "</sortByDate>\n";   /*DYJ DYJTrack 2.06b */
        oDyt << "\t\t<OsmUrl>" << OsmUrl << "</OsmUrl>\n";	//DYJ DYJTrack 2.06d
        oDyt << "\t</options>\n";
        oDyt << "\t<gpxfiles>\n";
        QString tmp;
        tmp.sprintf("\t\t<color>0x%.6X</color>\n",m_colDef_N.rgb()&0xffffff);
        oDyt << tmp;
        tmp.sprintf("\t\t<colorF>0x%.6X</colorF>\n",m_colDef_F.rgb()&0xffffff);
        oDyt << tmp;
        int offsH=ui->cB_TZh->currentIndex()-12,offsM=ui->cB_TZmin->currentIndex()*15;
        tmp.sprintf("\t\t<timezone>%d:%d</timezone>\n",offsH,offsM);
        oDyt << tmp;
        if(	m_anzFiles)
        {
//          m_tZone=COleDateTimeSpan(0,offsH,offsM,0);
            oDyt << "\t\t<anzgpxfiles>" <<  m_anzFiles << "</anzgpxfiles>\n";
            for(int i=0;i<m_anzFiles;i++)
            {
                QListWidgetItem *item=ui->filesListWidget->item(i);
                CGPXInfo *pGi=item->data(Qt::UserRole).value<CGPXInfo *>();
                oDyt << "\t\t<gpxfile>\n";
                if(!pGi->m_route.isEmpty())
                    oDyt << "\t\t\t<route>" << pGi->m_route << "</route>\n";
                if(!pGi->m_pfad.isEmpty())
                    writeDytPathLine(oDyt,"\t\t\t<path>%s</path>\n", pGi->m_pfad,path,expDir);
                oDyt << "\t\t\t<name>" << pGi->m_name << "</name>\n";
                /*DYJ DYJTrack 2.06c Start*/
                switch(pGi->m_col.m_useTrColSp)
                {
                case TC_AUTO:
                    tmp.sprintf("\t\t\t<color>0x1000000</color>\n");
                    break;
                case TC_STD:
                    tmp.sprintf("\t\t\t<color>0x2000000</color>\n");
                    break;
                case TC_FLUG:
                    tmp.sprintf("\t\t\t<color>0x3000000</color>\n");
                    break;
                default:
                    tmp.sprintf("\t\t\t<color>0x%.6X</color>\n",pGi->m_col.get_col(false,TC_RGB).rgb()&0xffffff);
                }

                /*DYJ  Ende; alt:
                if(pGi->m_col.m_useDef)
                    tmp.sprintf("\t\t\t<color>0x1000000</color>\n");
                else
                    tmp.sprintf("\t\t\t<color>0x%.6X</color>\n",pGi->m_col.get_col(false).rgb()&0xffffff);
                */
                oDyt << tmp;
                oDyt << "\t\t\t<start>" << pGi->m_dateTime.toString("d.M.yyyy h:m:s") << "</start>\n";
                oDyt << "\t\t\t<reverse>" << pGi->m_rev << "</reverse>\n";
                oDyt << "\t\t</gpxfile>\n";
            }
        }
        oDyt << "\t</gpxfiles>\n";

        oDyt << "\t<mainarea>\n";
        oDyt << "\t\t<auto>" << m_autoAreas[0] << "</auto>\n";
        oDyt << "\t\t<frame>" << m_frames[0] << "</frame>\n";
        if(!m_autoAreas[0])
        {
            oDyt << "\t\t<lat1>" << m_lat[0] << "</lat1>\n";
            oDyt << "\t\t<lat2>" << m_lat[1] << "</lat2>\n";
            oDyt << "\t\t<lon1>" << m_lon[0] << "</lon1>\n";
            oDyt << "\t\t<lon2>" << m_lon[1] << "</lon2>\n";
        }
        oDyt << "\t</mainarea>\n";

        oDyt << "\t<zoomarea>\n";
        oDyt << "\t\t<auto>" << m_autoAreas[1] << "</auto>\n";
        oDyt << "\t\t<frame>" << m_frames[1] << "</frame>\n";
        if(!m_autoAreas[1])
        {
            oDyt << "\t\t<lat1>" << m_lat[2] << "</lat1>\n";
            oDyt << "\t\t<lat2>" << m_lat[3] << "</lat2>\n";
            oDyt << "\t\t<lon1>" << m_lon[2] << "</lon1>\n";
            oDyt << "\t\t<lon2>" << m_lon[3] << "</lon2>\n";
        }
        oDyt << "\t</zoomarea>\n";

        oDyt << "\t<htmlmap>\n";
        oDyt << "\t\t<lat>" << m_htmlOpt.m_lat << "</lat>\n";
        oDyt << "\t\t<lon>" << m_htmlOpt.m_lon << "</lon>\n";
        oDyt << "\t\t<zoom>" << m_htmlOpt.m_zoom << "</zoom>\n";
        oDyt << "\t\t<title>" << m_htmlOpt.m_title << "</title>\n";
        oDyt << "\t\t<descr>" << m_htmlOpt.m_descr << "</descr>\n";
        writeDytPathLine(oDyt,"\t\t<template>%s</template>\n", m_htmlOpt.m_templ,path,expDir);
        writeDytPathLine(oDyt,"\t\t<output>%s</output>\n", m_htmlOpt.m_out,path);
        if(m_htmlOpt.m_oplSrc==OL_URL)
            oDyt << "\t\t<openlayer>" << m_htmlOpt.m_oplPath << "</openlayer>\n";
        else
            oDyt << "\t\t<openlayer>" << m_htmlOpt.m_oplSrc << "</openlayer>\n";

        oDyt << "\t</htmlmap>\n";

        oDyt << "\t<avimap>\n";
        oDyt << "\t\t<size>\n";
        oDyt << "\t\t\t<x>" << m_aviOpt.m_x << "</x>\n";
        oDyt << "\t\t\t<y>" << m_aviOpt.m_y << "</y>\n";
        oDyt << "\t\t\t<fps>" << m_aviOpt.m_fps << "</fps>\n";
        oDyt << "\t\t</size>\n";
        oDyt << "\t\t<duration>\n";
        for(int i=0;i<6;i++)
            oDyt << "\t\t\t<time_" << i << ">" << m_aviOpt.m_time[i] << "</time_" << i << ">\n";
        oDyt << "\t\t</duration>\n";
        oDyt << "\t\t<source>" << m_aviOpt.m_source << "</source>\n";

        CMapSrc::writeTahoSelected(oDyt,MAP_OVR,PIX_AVI);

        writeDytPathLine(oDyt,"\t\t<output>%s</output>\n", m_aviOpt.m_out,path);
        oDyt << "\t</avimap>\n";

        oDyt << "\t<pictmap>\n";
        oDyt << "\t\t<size>\n";
        oDyt << "\t\t\t<x>" << m_pictOpt.m_x << "</x>\n";
        oDyt << "\t\t\t<y>" << m_pictOpt.m_y << "</y>\n";
        oDyt << "\t\t</size>\n";
        oDyt << "\t\t<source>" << m_pictOpt.m_source << "</source>\n";


        CMapSrc::writeTahoSelected(oDyt,MAP_OVR,PIX_PICT);

        writeDytPathLine(oDyt,"\t\t<output>%s</output>\n", m_pictOpt.m_out,path);
        oDyt << "\t</pictmap>\n";

        oDyt << "</dyjtrack>\n";
        gDyt.close();
    }

}
/*** ------------------------------------------------------------------------ ***/
/*	void CDyjTrackDlg::writeDytPathLine(QTextStream &oDyt,QString format, QString qPfad, QString dytPfad, QString *expDir)		*/
/*	Schreibt eine Zeile fürs DYT die einen Pfad enthält, entweder Absolut,		*/
/*	relativ oder für Export inc. File kopieren									*/
/*	Parameter:	oDytt: Das Zielfile												*/
/*				format: Zeilenformat											*/
/*				qPfad:	Der zu schreibende Pfad									*/
/*				dytPfad: Pfad des DYT											*/
/*				expDir: nullptr oder Verzeichnis für Export						*/
/*	Ergebnis:	keines															*/
/*	06.11 von Dimitri Junker	DYJ 1.03d										*/
/*** ------------------------------------------------------------------------ ***/

void dytMainW::writeDytPathLine(QTextStream &oDyt,QString format, QString qPfad, QString dytPfad, QString *expDir)
{
    QString txt;
    if(expDir!=nullptr)
    {
        int num=0;
        CPath pExpFile,pqPfad;
        pExpFile=*expDir;
        pqPfad=qPfad;
        pExpFile.setFile(qPfad);
        QString expFileC=pExpFile.makePath();
        while(!QFile::copy(qPfad,expFileC))
        {
            //dann gibt es zufällig 2 Files mit dem gleichen Namen
            QString pqFname=pqPfad.makePath(PATH_FN);
            char expFname[_MAX_FNAME];
            sprintf_s(expFname,"%s_%d",pqFname.toStdString().c_str(),++num);
            pExpFile.setComp(expFname,PATH_FN);

            expFileC=pExpFile.makePath();
        }
        txt.sprintf(format.toStdString().c_str(),CPath::relPath(expFileC,dytPfad).toStdString().c_str());
    }
//DYJ DYJTrack 2.07j 	    else if(m_opt.getRelPath())
//DYJ DYJTrack 2.07j 	        txt.sprintf(format.toStdString().c_str(),CPath::relPath(qPfad,dytPfad).toStdString().c_str());
    else
        txt.sprintf(format.toStdString().c_str(),qPfad.toStdString().c_str());
    oDyt << txt;
}
void dytMainW::on_actionBild_erstellen_triggered()
{
    CMakePix makePict;
    makePict.m_anzFiles=m_anzFiles;
    for(int i=0;i<2;i++)
    {
        makePict.m_lat[i]=m_lat[i];
        makePict.m_lon[i]=m_lon[i];
    }
    makePict.m_prgPath=m_opt.m_prgPath;
    makePict.m_pFB=ui->filesListWidget;
    makePict.m_pOpt=&m_pictOpt;
    makePict.initDialog();
    makePict.exec();

}

void dytMainW::on_actionFilm_erzeugen_triggered()
{
    CMakePix makeAvi;
    makeAvi.m_anzFiles=m_anzFiles;
    for(int i=0;i<4;i++)
    {
        makeAvi.m_lat[i]=m_lat[i];
        makeAvi.m_lon[i]=m_lon[i];
    }
    makeAvi.m_prgPath=m_opt.m_prgPath;
    makeAvi.m_pFB=ui->filesListWidget;
    makeAvi.m_pOpt=&m_aviOpt;
    double maxAviLen = 1024. * 1024. * 1024. * m_opt.getAviMax();	// Umrechnung von GB in Bytes
    makeAvi.m_maxAviLen = (qint64)maxAviLen;	/*DYJ 1.08a */
    makeAvi.initDialog();
    makeAvi.exec();

}
/*DYJ DYJTrack 2.05b Start*/
void dytMainW::on_actionNeues_Projekt_triggered()
{
    OnDelAll();
    m_projectPath="";
    this->setWindowTitle(m_progName);
}
/*DYJ  Ende; */

void dytMainW::on_actionProjekt_laden_triggered()
{
    loadProject(FALSE);
}
void dytMainW::on_actionProjekt_hinzu_laden_triggered()
{
    loadProject(TRUE);
}
void dytMainW::loadProject(bool add)
{
    QString pfad;
    if(m_projectPath.isEmpty())
    {
        /*DYJ DYJTrack 2.07j Start*/
        QFileInfo fi(COsmOpt::m_persPath,T_SET);
        /*DYJ  Ende; alt:
        QFileInfo fi(m_opt.defPath(),T_SET);
        */
        pfad=fi.filePath();
    }
    else
        pfad=m_projectPath.makePath(PATH_FULL);
    QString path= QFileDialog::getOpenFileName(this,tr("Lade Projekt"),pfad,tr("Projekte(*.dyt)"));
    if(path.isNull())
        return;

    if(m_anzFiles && !add)
    {
        while(m_anzFiles)
            deleteGpx(0);
        updateArea(TRUE);
    }
    loadDyt(path);
    QString title;
    if (!add)
    {
        /*DYJ DYJTrack 2.04a Start*/
        /*DYJ DYJTrack 2.05a Start*/
        title = m_progName + " - " + pfad;
        /*DYJ  Ende; alt:
        title = m_progName + " - " + title;
        */
        /*DYJ  Ende; alt:
        title=this->windowTitle();
        int pos;
        pos = title.lastIndexOf('-');
        if (pos > -1)
            title.remove(0, pos + 2);
        title = path + " - " + title;
        */
        this->setWindowTitle(title);
    }
}

void dytMainW::on_actionProjekt_speichern_triggered()
{
    /*DYJ DYJTrack 2.05a Start*/
    if(m_projectPath.isEmpty() || !m_projectPath.makePath(PATH_FILE).compare(T_SET))
    /*DYJ  Ende; alt:
    if(m_projectPath.isEmpty())
    */
        on_actionProjekt_speichern_unter_triggered();
    QString path=m_projectPath.makePath();
    saveDyt(path);

}

void dytMainW::on_actionProjekt_speichern_unter_triggered()
{
    QString pfad,file;
    if(m_projectPath.isEmpty())
    {
        /*DYJ DYJTrack 2.07j Start*/
        pfad=COsmOpt::m_persPath;
        /*DYJ  Ende; alt:
        pfad=m_opt.defPath();
        */

        file=T_SET;
    }
    else
    {
        pfad=m_projectPath.makePath(PATH_PATH);
        file=m_projectPath.makePath(PATH_FILE);
    }
    QString path= QFileDialog::getSaveFileName(this,tr("Speicher Projekt"),"",tr("Projekt(*.dyt)"));
    if(path.isNull())
        return;
    m_projectPath=path;
    saveDyt(path);
    QString title;
    title=this->windowTitle();
    int pos;
    pos = title.lastIndexOf('-');
    if (pos > -1)
        title.remove(0, pos + 2);
    title = path + " - " + title;
    this->setWindowTitle(title);


}
/*** ------------------------------------------------------------------------ ***/
/*	void CDyjTrackDlg::on_actionProjekt_exportieren_triggered()													*/
/*	Projekt exportieren															*/
/*	Parameter:	keine															*/
/*	Ergebnis:	keines															*/
/*	06.11 von Dimitri Junker	DYJ 1.03d										*/
/*** ------------------------------------------------------------------------ ***/

void dytMainW::on_actionProjekt_exportieren_triggered()
{
    QString path= QFileDialog::getSaveFileName(this,tr("Speicher Projekt"),"",tr("Projekt(*.dyt)"));
    if(path.isNull())
        return;
    if(path.right(4).compare(".dyt",Qt::CaseSensitive)==0)
    {
        QString dir=path.left(path.length()-4)+"_Files/";
        COsm::rmdirRec(dir);
        QDir dirH;
        dirH.mkpath(dir);
        saveDyt(path,&dir);
    }

}

void dytMainW::on_actionBeenden_triggered()
{
    while(m_anzFiles)
        deleteGpx(0);

    qApp->quit();

}

void dytMainW::on_actionHTML_erstellen_triggered()
{
    updateArea(false);
    CMakeHtml makeHtml;
    makeHtml.m_anzFiles=m_anzFiles;
    makeHtml.m_prgPath=m_opt.m_prgPath;
    makeHtml.m_pFB=ui->filesListWidget;
    makeHtml.m_pOpt=&m_htmlOpt;

    makeHtml.initDialog();
    makeHtml.exec();

}

void dytMainW::on_actionOptionen_triggered()
{
//DYJ Taho 4.07d 	       m_opt.m_reloadSrc=FALSE;
    m_opt.exec();
//DYJ Taho 4.07d 	       if(m_opt.m_reloadSrc)
        /*DYJ DYJTrack 2.07j Start*/
//DYJ Taho 4.07d 	           m_opt.loadTahoSrc( false);
        /*DYJ  Ende; alt:
        m_opt.loadTahoSrc(m_opt.getDefPathIsPrg());
        */

}
/*** ------------------------------------------------------------------------ ***/
/*	void dytMainW::on_actionInfo_triggered()                        			*/
/*	Zeigt Copyright Dialog an                                               	*/
/*	Parameter:	keine															*/
/*	Ergebnis:	keines															*/
/*	07.8.16 von Dimitri Junker	DYJ 2.04a                                       */
/*** ------------------------------------------------------------------------ ***/

void dytMainW::on_actionInfo_triggered()
{

    QString vers=QString("%1 (%2)").arg(m_progName).arg(__DATE__);

    CDyjTCopy to(vers);
    to.exec();
}
void dytMainW::on_actionHilfe_triggered()
{
/*DYJ DYJTrack 2.03d Start*/
    QString url="file:./Docu/"+tr("liesmich.pdf");
/*DYJ  Ende; alt:
    QString url="file:///./Docu/"+tr("liesmich.pdf");
*/
    QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

}
void dytMainW::changeEvent(QEvent *e)
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
