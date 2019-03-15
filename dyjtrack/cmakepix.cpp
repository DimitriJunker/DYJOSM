#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qdesktopservices.h>
#include <qmath.h>
#include <QDebug>

#include "ui_cmakepix.h"
#include "cmakepix.h"
#include "cmapsrc.h"
#include <cavifile.h>
#include "cosm.h"
#include "cpixmap.h"
#include "urldownload.h"


/*
 * 19.10.15 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ DYJTrack 2.02a'
 * Durch 'DYJ Taho 4.03d' nötige Änderung. Die Fehlerbehandlung war teils selber Fehlerhaft.
 * 10.3.16 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ DYJTrack 2.03a'
 * Die Tiles wurden nicht aus dem Cache genommen sondern immer wieder neu geladen -> sehr langsam
 */
Q_DECLARE_METATYPE(CGPXInfo*)
Q_DECLARE_METATYPE(CMapSrc*)

CMakePix::CMakePix(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CMakePix)
{
    ui->setupUi(this);
}

CMakePix::~CMakePix()
{
    delete ui;
}
void CMakePix::OnSelOut()
{
    QString outFile=ui->l_aviOutp->text();
    if(m_pOpt->m_pixType==PIX_AVI)
        outFile= QFileDialog::getSaveFileName(this,tr("Speicher Film"),outFile,tr("Filme(*.avi)"),nullptr,QFileDialog::DontConfirmOverwrite);
    else
        outFile= QFileDialog::getSaveFileName(this,tr("Speicher Bild"),outFile,tr("JPG-Files(*.jpg);;Portable Network Graphics (*.png);;Windows Bitmap(*.bmp);;Tagged Image File Format(*.tiff)"),nullptr,QFileDialog::DontConfirmOverwrite);
    if(!outFile.isNull())
        ui->l_aviOutp->setText(outFile);
}
void CMakePix::OnPict()
{
    QString outFile=ui->l_aviOutp->text();
    QFileInfo fi(outFile);
    QDir dirH;
    /*DYJ DYJTrack 2.07c Start*/

    CMapSrc *map;
    if((map=CMapSrc::lookup(CMapSrc::getSrc()))==nullptr)
        return;
    else if(map->hatUnbekannteID())
       return;
    /*DYJ  Ende; */
    dirH.mkpath(fi.path());
    if(fi.exists())	// prüfen ob das File bereits besteht
    {
        QMessageBox::StandardButton overw;
        overw = QMessageBox::question(nullptr, tr("File existiert"), tr("Soll das File überschrieben werden?"),
                                      QMessageBox::Yes|QMessageBox::No);

        if(overw==QMessageBox::No)
            return;
    }

    makePict();
}



void CMakePix::OnAvi()
{
    QString outFile=ui->l_aviOutp->text();
    QFileInfo fi(outFile);
    QDir dirH;
    /*DYJ DYJTrack 2.07c Start*/

    CMapSrc *map;
    if((map=CMapSrc::lookup(CMapSrc::getSrc()))==nullptr)
        return;
    else if(map->hatUnbekannteID())
       return;
    /*DYJ  Ende; */
    dirH.mkpath(fi.path());
//    CWnd* pWnd,*pCancel=GetDlgItem(IDCANCEL);
    int msgNr = 0;
    if(fi.exists())
        msgNr = 1;
    QDir dir=fi.dir();
    QString fiName=fi.completeBaseName();
    QString fiExt=fi.suffix();
    QString suchStr1=fiName+"_*."+fiExt;
    QString suchStr2=fiName+".m3u";


    QStringList nameFilter1;
    nameFilter1 << suchStr1<<suchStr2;
    QFileInfoList list1 = dir.entryInfoList( nameFilter1, QDir::Files );
    if(!list1.isEmpty())
        msgNr += 2;
    QMessageBox::StandardButton overw=QMessageBox::Yes;
    switch (msgNr)
    {
        case 1:	//File ist vorhanden
            overw = QMessageBox::question(nullptr, tr("File existiert"), tr("Soll das File überschrieben werden?"),
                                      QMessageBox::Yes|QMessageBox::No);
        break;
        case 2:	//evtl Folgefile vorhanden
            overw = QMessageBox::question(nullptr, tr("Ein Folgefile (falls der Film zu lang ist) besteht bereits"), tr("Soll es ggf. überschrieben werden?"),
                                      QMessageBox::Yes|QMessageBox::No);
            break;
        case 3:	//Start und Folgefile vorhanden
            overw = QMessageBox::question(nullptr, tr("Das File und Folgefiles (falls der Film zu lang ist) bestehen bereits"), tr("Sollen sie ggf. überschrieben werden?"),
                                      QMessageBox::Yes|QMessageBox::No);
            break;
    }
    if(overw!=QMessageBox::No)
    {
        if (!makeAvi())
        {
            QString url="file:///"+m_showFile;
            QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));
        }
    }
}

void CMakePix::makePict()
{
//    MSG msg;
    m_onErrTile=OE_NUROK;
    QProgressDialog progress(tr("Erstelle Bild..."), tr("Abbruch"), 0, m_anzFiles+5, this);
    progress.setWindowModality(Qt::ApplicationModal);
    QRect qRect;

    int x=ui->le_aviX->text().toInt(),y= ui->le_aviY->text().toInt();
    QImage imMap(x, y,QImage::Format_RGB32);
    QPainter paMap(&imMap);
    int zoom=loadBmMap(&paMap,&qRect,&progress);
    progress.setValue(5);
    if(zoom<0)
        return;
    int i,maxLen=qMax(qRect.width(),qRect.height());
    int *skal=new int[static_cast<unsigned long>(maxLen)];
    double fakt=static_cast<double>(x)/qRect.width();
    for(i=0;i<maxLen;i++)
        skal[i]=static_cast<int>(i*fakt+.5);
    int zoomF=1<<zoom;
    double subX=qRect.x()-.5;
    double subY=qRect.y()-128.*zoomF-.5;
    progress.setLabelText(tr("Erstelle Wegpunktliste"));
    double deg2rad=acos(-1.)/180;
    double faktX=256./360.*zoomF;
    double faktY=-128./360.*zoomF/deg2rad;
    subX-=128*zoomF;
    for(i=0;i<m_anzFiles;i++)
    {
        int lstX=0,lstY=0;
        bool first=true;
        progress.setValue(progress.value()+1);
        if (progress.wasCanceled())
            break;

        QListWidgetItem *item=m_pFB->item(i);
        CGPXInfo *pGi=item->data(Qt::UserRole).value<CGPXInfo *>();
        /*DYJ DYJTrack 2.06c Start*/
        QColor col=pGi->m_col.get_col(pGi->m_pfad.isEmpty(),pGi->m_col.m_useTrColSp);
        /*DYJ  Ende; alt:
        QColor col=pGi->m_col.get_col(pGi->m_pfad.isEmpty());
        */
        QPen pen(col);
        pen.setWidth(ui->le_aviLineW->text().toInt());
        paMap.setPen(pen);

        QList<CGeoPoint>::Iterator aktPos=pGi->m_trkPts.end();
        while(aktPos!=pGi->m_trkPts.begin())
        {
            aktPos--;
            CGeoPoint gpAkt=*aktPos;
            if(gpAkt.m_lat<85.05&& gpAkt.m_lat>-85.05)
            {
                if(gpAkt.m_lon<m_lon[0])
                    gpAkt.m_lon+=360;
                double dX=gpAkt.m_lon*faktX-subX;
                double slat=sin(gpAkt.m_lat*deg2rad);
                double dY=log((1+slat)/(1-slat))*faktY-subY;
                int x=skal[static_cast<int>(dX)],y=skal[static_cast<int>(dY)];
                if(first)
                    first=false;
                else
                    paMap.drawLine(lstX,lstY,x,y);
                lstX=x;
                lstY=y;
            }
            else
                first=true;
        }
    }
    if(!imMap.save(ui->l_aviOutp->text()))
        QMessageBox::warning(nullptr, tr("Fehler"),tr("kann File nicht schreiben, evtl falsches Format!"),QMessageBox::Ok);
    QString url="file:///"+ui->l_aviOutp->text();
    QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

}


void CMakePix::initDialog()
{
    bool isAvi=m_pOpt->m_pixType==PIX_AVI;
    ui->pb_avi->setEnabled(isAvi);
    ui->pb_avi->setVisible(isAvi);

    ui->le_aviFps->setEnabled(isAvi);
    ui->le_aviFps->setVisible(isAvi);
    ui->le_aviT1->setEnabled(isAvi);
    ui->le_aviT1->setVisible(isAvi);
    ui->le_aviT2->setEnabled(isAvi);
    ui->le_aviT2->setVisible(isAvi);
    ui->le_aviT3->setEnabled(isAvi);
    ui->le_aviT3->setVisible(isAvi);
    ui->le_aviT4->setEnabled(isAvi);
    ui->le_aviT4->setVisible(isAvi);
    ui->le_aviT5->setEnabled(isAvi);
    ui->le_aviT5->setVisible(isAvi);
    ui->le_aviT6->setEnabled(isAvi);
    ui->le_aviT6->setVisible(isAvi);
    ui->l_fps->setVisible(isAvi);

    ui->gb_Dauer->setVisible(isAvi);

    ui->pb_pict->setEnabled(m_pOpt->m_pixType==PIX_PICT);
    ui->l_aviOutp->setText(m_pOpt->m_out);
    ui->le_aviX->setText(QString::number(m_pOpt->m_x,'g',10));
    ui->le_aviY->setText(QString::number(m_pOpt->m_y,'g',10));
    ui->le_aviFps->setText(QString::number(m_pOpt->m_fps));
    ui->le_aviLineW->setText(QString::number(m_pOpt->m_lineW));

    ui->le_aviT1->setText(QString::number(m_pOpt->m_time[0]));
    ui->le_aviT2->setText(QString::number(m_pOpt->m_time[1]));
    ui->le_aviT3->setText(QString::number(m_pOpt->m_time[2]));
    ui->le_aviT4->setText(QString::number(m_pOpt->m_time[3]));
    ui->le_aviT5->setText(QString::number(m_pOpt->m_time[4]));
    ui->le_aviT6->setText(QString::number(m_pOpt->m_time[5]));

    if(m_pOpt->m_pixType==PIX_AVI)
        this->setWindowTitle(tr("Filmausgabe"));
    else
        this->setWindowTitle(tr("Bildausgabe"));
//    m_ovrCtrl.init(200);
    CMapSrc::initBoxes(ui->cb_quell,ui->lw_over,nullptr,m_pOpt->m_pixType);
    ui->cb_quell->setCurrentText(m_pOpt->m_source);


}

int CMakePix::loadMap(QPainter *paMap,QRect *qRect,QProgressDialog *pProg,CGeoRect *area,bool cacheMap)
{
    int zoom=loadBmMap(paMap,qRect,pProg,area, cacheMap);
    if(zoom<0 )
        return -1;

    return zoom;
}

int CMakePix::makeAvi()
{
    int err = 0;

    QImage imMap(ui->le_aviX->text().toInt(), ui->le_aviY->text().toInt(),QImage::Format_RGB32);
    QPainter paMap(&imMap);

    m_onErrTile=OE_WEITER;

//    LPBITMAPINFOHEADER pBMIH = (LPBITMAPINFOHEADER) new
  //      char[sizeof(BITMAPINFOHEADER) ];
    LPBITMAPINFOHEADER pBMIH =new BITMAPINFOHEADER[sizeof(BITMAPINFOHEADER) ];
    pBMIH->biSize = sizeof(BITMAPINFOHEADER);
    pBMIH->biWidth = ui->le_aviX->text().toInt();
    pBMIH->biHeight = ui->le_aviY->text().toInt();
    pBMIH->biPlanes = 1;
    pBMIH->biBitCount =static_cast<WORD>(32);
    pBMIH->biCompression = BI_RGB;
    pBMIH->biSizeImage = 0;
    pBMIH->biXPelsPerMeter = 0;
    pBMIH->biYPelsPerMeter = 0;
    pBMIH->biClrUsed = 0;
    pBMIH->biClrImportant = 0;

    QRect qRect;
    m_showFile = ui->l_aviOutp->text();
    CAVIFile aviFile(&m_showFile,ui->le_aviFps->text().toInt(), m_maxAviLen);

    CGeoRect dummy;	//zur Initialisierung von loadMap
    if (loadMap(&paMap, &qRect,nullptr, &dummy) == -1)
    {
        err = 1002;
        QMessageBox::warning(nullptr, tr("Fehler"),tr("Bereich ungültig!"),QMessageBox::Ok);
    }
    else
        err = aviFile.fileOpen();
    CGeoRect zoomArea(m_lat[2],m_lat[3],m_lon[2],m_lon[3],85.0511);
    int iProg=0;
    QProgressDialog progress(tr("Erstelle Film..."), tr("Abbruch"), 0, 100, this);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.show();
    QApplication::processEvents();
    if(!err)
    {
        int frames=ui->le_aviT1->text().toInt()+ui->le_aviT2->text().toInt()+ui->le_aviT3->text().toInt()+ui->le_aviT4->text().toInt()+ui->le_aviT5->text().toInt()+ui->le_aviT6->text().toInt();
        frames*=ui->le_aviFps->text().toInt();
        progress.setMaximum(frames+m_anzFiles);

        //AVI-Zoom-Vorspann

        if(ui->le_aviT1->text().toInt())
        {
            if(loadMap(&paMap,&qRect,&progress,&zoomArea,TRUE)==-1)
                err=1001;
            else
            {
                progress.setLabelText(tr("Schreibe AVI-Zoom-Vorspann"));
                int i;
                for (i = 0; i<(ui->le_aviT1->text().toInt()*ui->le_aviFps->text().toInt()) && !err; i++)
                {
                    progress.setValue(iProg++);
                    if (progress.wasCanceled())
                        err=1004;
                    else
                        err = aviFile.writeFrame(imMap,pBMIH);
                }
            }
        }
    }

    // AVI-Zoom Hauptteil

    if(!err && ui->le_aviT2->text().toInt())
    {
        int framesZ = ui->le_aviT2->text().toInt()*ui->le_aviFps->text().toInt();
        progress.setLabelText(tr("Schreibe AVI-Zoom Hauptteil"));
        int f;
        CGeoPoint mPkt1,mPkt2;
        mPkt1.m_lat=(m_lat[2]+m_lat[3])/2;
        mPkt1.m_lon=(m_lon[2]+m_lon[3])/2;
        mPkt2.m_lat=(m_lat[0]+m_lat[1])/2;
        mPkt2.m_lon=(m_lon[0]+m_lon[1])/2;
        double b1,b2,h1,h2,fb,fh;
        h1=m_lat[2]-m_lat[3];
        b1=m_lon[3]-m_lon[2];
        h2=m_lat[0]-m_lat[1];
        b2=m_lon[1]-m_lon[0];
        fb=pow((b2/b1),1./framesZ);
        fh=pow((h2/h1),1./framesZ);
        double b=b1,h=h1,sh=0,sb=0,s_lat,s_lon;

        for(f=0;f<framesZ && !err;f++)
        {
            b*=fb;
            sb+=b;
            h*=fh;
            sh+=h;
        }
        b=b1;
        h=h1;
        s_lat=(mPkt2.m_lat-mPkt1.m_lat)/sh;
        s_lon=(mPkt2.m_lon-mPkt1.m_lon)/sb;
        CGeoPoint mPkt=mPkt1;

        for(f=0;f<framesZ && !err;f++)
        {
            b*=fb;
            h*=fh;
            mPkt.m_lon+=b*s_lon;
            mPkt.m_lat+=h*s_lat;
            zoomArea.m_n=mPkt.m_lat+h/2;
            zoomArea.m_s=mPkt.m_lat-h/2;
            zoomArea.m_w=mPkt.m_lon-b/2;
            zoomArea.m_e=mPkt.m_lon+b/2;
            if(loadMap(&paMap,&qRect,&progress,&zoomArea,TRUE)==-1)
            {
                err=1001;
                break;
            }
            progress.setValue(iProg++);
            if (progress.wasCanceled())
                err=1004;
            else
                err = aviFile.writeFrame(imMap,pBMIH);
        }
    }
    if(!err && (ui->le_aviT3->text().toInt()+ui->le_aviT4->text().toInt()+ui->le_aviT5->text().toInt()+ui->le_aviT6->text().toInt())>0)
    {
        QList<QPoint> route;
        bool ok;
        if((ui->le_aviT5->text().toInt()+ui->le_aviT6->text().toInt())>0)	//dann brauchen wir die Karte und die Route
            ok=makeMapAndPtlist(route,paMap,&progress,FALSE);
        else							//dann nur die leere Karte
            ok=(loadMap(&paMap,&qRect)!=-1);
        iProg+=m_anzFiles;
        if(ok)
        {
            unsigned int i, frames = (ui->le_aviT3->text().toUInt() + ui->le_aviT4->text().toUInt())*ui->le_aviFps->text().toUInt();

            if(!err && frames>0)
            {
                progress.setLabelText(tr("Schreibe AVI-Zoom-Nachsp+Route:Vorsp."));

                for(i=0;i<frames && !err;i++)
                {
                    progress.setValue(iProg++);
                    if (progress.wasCanceled())
                        err=1004;
                    else
                        err = aviFile.writeFrame(imMap,pBMIH);
                }
            }
            if(!err && (ui->le_aviT5->text().toInt()+ui->le_aviT6->text().toInt())>0)
            {
                frames = ui->le_aviT5->text().toUInt()*ui->le_aviFps->text().toUInt();
                if(frames<2)
                    frames=2;	// wg. nächster Zeile
                double ptsPF=double(route.count())/(frames-1),rstPt=0;	//frames-1 damit nicht durch Rundungsfehler o.ä. die letzten Pixel fehlen
                QPen stift(TrackCol::m_normal);
                stift.setCapStyle(Qt::RoundCap);
                stift.setWidth(ui->le_aviLineW->text().toInt());
                paMap.setRenderHint(QPainter::Antialiasing,true);
                paMap.setPen(stift);
                progress.setLabelText(tr("Schreibe AVI-Route:Hauptteil"));
                int aktPos = 0;
                for(i=0;i<frames && !err;i++)
                {
                    progress.setValue(iProg++);
                    rstPt+=ptsPF;
                    while (aktPos < route.size() && rstPt>0)
                    {
                        QPoint ptAkt=route.at(aktPos);
                        if(ptAkt.x()==-1)
                        {
                            stift.setColor(QColor::fromRgb(static_cast<unsigned int>(ptAkt.y())));
                            paMap.setPen(stift);
                        }
                        else
                            paMap.drawPoint(ptAkt);
                        rstPt--;
                        aktPos++;
                    }
                    if (progress.wasCanceled())
                        err=1004;
                    else
                        err = aviFile.writeFrame(imMap,pBMIH);
                }
                progress.setLabelText(tr("Schreibe AVI-Route:Nachspann"));
                for (int f = 0; f<(ui->le_aviT6->text().toInt()*ui->le_aviFps->text().toInt()) && !err; f++)
                {
                    progress.setValue(iProg++);
                    if (progress.wasCanceled())
                        err=1004;
                    else
                        err = aviFile.writeFrame(imMap,pBMIH);
                }
            }
        }
    }
    if (err)
        qDebug()<<"Fehler:"<< err;

    return err;
}


void CMakePix::OnGpx()
{
    QRect qRect;
    int zoom=loadBmMap(nullptr,&qRect);
    if(zoom<0|| !m_anzFiles)
        return;


    double lon1=static_cast<double>(qRect.x())/256/ pow(2.0, zoom) * 360.0 - 180;
    double lon2=static_cast<double>(qRect.x()+qRect.width())/256/ pow(2.0, zoom) * 360.0 - 180;
    double n = PI - 2.0 * PI * qRect.y()/256 / pow(2.0, zoom);
    double lat1=180.0 / PI * atan(0.5 * (exp(n) - exp(-n)));
    n = PI - 2.0 * PI * (qRect.y()+qRect.height())/256 / pow(2.0, zoom);
    double lat2=180.0 / PI * atan(0.5 * (exp(n) - exp(-n)));

    QString outFile=ui->l_aviOutp->text();
    QFileInfo fi(outFile);
    QDir dir=fi.dir();
    QString file=fi.baseName()+".dyt";
    fi.setFile(dir,file);
    QString path= QFileDialog::getSaveFileName(this,tr("Speicher GPX"),fi.filePath(),tr("GPX-Files(*.gpx);;dyjtrack-File (*.dyt)"));
    if(path.isEmpty())
        return;
    fi.setFile(path);
    QString ext=fi.suffix();
    if(!ext.compare("gpx",Qt::CaseInsensitive))
    {
        FILE *fGpx;
        fopen_s(&fGpx,path.toLatin1(), "w");
        if(fGpx)
        {
            fprintf(fGpx,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
            fprintf(fGpx,"<gpx version=\"1.0\" creator=\"dyjtrack.exe - http://www.dimitri-junker.de/html/openstreetmap.html\"\n");
            fprintf(fGpx,"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
            fprintf(fGpx,"xmlns=\"http://www.topografix.com/GPX/1/0\"\n");
            fprintf(fGpx,"xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">\n");
            fprintf(fGpx,"<bounds minlat=\"%f\" minlon=\"%f\" maxlat=\"%f\" maxlon=\"%f\"/>\n",lat2,lon1,lat1,lon2);
            fprintf(fGpx,"<trk>\n\t<trkseg>\n");
            fprintf(fGpx,"\t\t<trkpt lat=\"%f\" lon=\"%f\">\n\t\t</trkpt>\n",lat1,lon1);
            fprintf(fGpx,"\t\t<trkpt lat=\"%f\" lon=\"%f\">\n\t\t</trkpt>\n",lat1,lon2);
            fprintf(fGpx,"\t\t<trkpt lat=\"%f\" lon=\"%f\">\n\t\t</trkpt>\n",lat2,lon2);
            fprintf(fGpx,"\t\t<trkpt lat=\"%f\" lon=\"%f\">\n\t\t</trkpt>\n",lat2,lon1);
            fprintf(fGpx,"\t\t<trkpt lat=\"%f\" lon=\"%f\">\n\t\t</trkpt>\n",lat1,lon1);
            fprintf(fGpx,"\t</trkseg>\n</trk>\n</gpx>\n");
            fclose(fGpx);

        }
    }
    else
    {
        FILE *fDyt;
        fopen_s(&fDyt,path.toLatin1(), "w");
        if(fDyt)
        {
            fprintf(fDyt,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
            fprintf(fDyt,"<dyjtrack version=\"1.0\" creator=\"dyjtrack.exe - http://www.dimitri-junker.de/html/openstreetmap.html\">\n");

            fprintf(fDyt,"\t<area>\n");
            fprintf(fDyt,"\t\t<auto>0</auto>\n");
            fprintf(fDyt,"\t\t<frame>0</frame>\n");
            fprintf(fDyt,"\t\t<lat1>%f</lat1>\n",lat1);
            fprintf(fDyt,"\t\t<lat2>%f</lat2>\n",lat2);
            fprintf(fDyt,"\t\t<lon1>%f</lon1>\n",lon1);
            fprintf(fDyt,"\t\t<lon2>%f</lon2>\n",lon2);
            fprintf(fDyt,"\t</area>\n");

            fprintf(fDyt,"</dyjtrack>\n");
            fclose(fDyt);
        }
    }

}

int CMakePix::loadBmMap(QPainter *paMap,QRect *qRect, QProgressDialog *pProg,CGeoRect *area,bool cacheMap)
{
    double deg2rad=PI/180;
    double lat1,lat2,lon1,lon2;
    if(area)
    {
        if(!area->isValid())
            return 0;
        lat1=area->m_n;
        lat2=area->m_s;
        lon1=area->m_w;
        lon2=area->m_e;
    }
    else
    {
        lat1=m_lat[0];
        lat2=m_lat[1];
        lon1=m_lon[0];
        lon2=m_lon[1];
    }

    // Kartenausschnitt bestimmen
    int zoom=0;
    if(lat1>85.051)
    {
        lat1=85.051;
        if(lat2>85.05)
            lat2=85.05;
    }
    if(lat2<-85.051)
    {
        lat2=-85.051;
        if(lat1<-85.05)
            lat1=-85.05;
    }
    double dX1=(lon1+180)*256/360;
    double dX2=(lon2+180)*256/360;
    double slat=sin(lat1*deg2rad);
    double dY1=128-log((1+slat)/(1-slat))/2*256/360/deg2rad;
           slat=sin(lat2*deg2rad);
    double dY2=128-log((1+slat)/(1-slat))/2*256/360/deg2rad;

    CMapSrc *map;
    if((map=CMapSrc::lookup(CMapSrc::getSrc()))==nullptr)
        return -1;

    int sizeX=ui->le_aviX->text().toInt(),sizeY= ui->le_aviY->text().toInt();

    while((dX2-dX1)<sizeX && (dY2-dY1)<sizeY && zoom<map->m_maxZoom)
    {
        dX1*=2;
        dX2*=2;
        dY1*=2;
        dY2*=2;
        zoom++;
    }
    if(((dX2-dX1)/sizeX)<((dY2-dY1)/sizeY))
    {
        double wh=(dY2-dY1)/sizeY*sizeX/2;
        double xm=(dX2+dX1)/2;
        dX1=xm-wh;
        dX2=xm+wh;
    }
    else
    {
        double hh=(dX2-dX1)/sizeX*sizeY/2;
        double ym=(dY1+dY2)/2;
        dY1=ym-hh;
        if(dY1<0)
            dY1=0;
        dY2=dY1+hh+hh;
    }
//	double faktor=sizeX/(dX2-dX1);
    int x1=static_cast<int>(dX1+.5),y1=static_cast<int>(dY1+.5),x2=static_cast<int>(dX2+.5),y2=static_cast<int>(dY2+.5);	//gerundet
    *qRect= QRect(x1,y1,x2-x1+1,y2-y1+1);
    if(paMap)	// sosnt nur Umriß bestimmen
    {
        int tx1=x1/256,tx2=x2/256,ty1=y1/256,ty2=y2/256;	//Tilenummern
        if(x1<0)
            tx1--;
        if(y1<0)
            ty1--;
    //Karte laden
        SDLM_DATA data;
        data.m_outBas="tmp/";
        QString tah=data.m_outBas+"tah/";
        QString out=data.m_outBas+"map/";
        QDir dirH;
        if(!dirH.mkpath(tah)||!dirH.mkpath(out))
        {
            QMessageBox::warning(nullptr, tr("Fehler"),tr("Kann Ausgabeverzeichnis nicht erstellen!"),QMessageBox::Ok);
            return -1;
        }
        data.m_maps.insert(0,map);

        for(int row=0;row<ui->lw_over->count();row++)
        {
            QListWidgetItem *item=ui->lw_over->item(row);
            if(item->checkState()==Qt::Checked)
            {
                CMapSrc *map=item->data(Qt::UserRole).value<CMapSrc*>();
                data.m_maps.append(map);
                QString dir=QString("%1%2/").arg(data.m_outBas).arg(map->m_name);
                if(!dirH.mkpath(dir))
                {
                    QMessageBox::warning(nullptr, tr("Fehler"),tr("Kann Ausgabeverzeichnis nicht erstellen!"),QMessageBox::Ok);
                    return -1;
                }

            }
        }

        data.m_sizeP=SIZE_FREE;
        data.m_nameBy=NB_NUM;
        data.m_maxCacheDays=1;
        data.m_errTxts.clear();
        data.m_tna=m_prgPath+T_NA;
        data.m_progD=pProg;
        data.errs=0;        /*DYJ DYJTrack 2.02a */
        QFileInfo fi(data.m_tna);

        if(!fi.exists())	// prüfen ob das File bereits besteht
        /*DYJ DYJTrack 2.06d Start*/
        {
            QString url=OsmUrl+"taho/tna.png";
            urlDownload::downloadFile(url, data.m_tna);
        }
        /*DYJ  Ende; alt:
            urlDownload::downloadFile("http://www.oche.de/~junker/OSM/taho/tna.png", data.m_tna);
        */
        data.m_bpp=2;
        QImage *pbMap1_1=nullptr;	// unscalierte Karte
        QRect ausschn(x1-tx1*256,y1-ty1*256,qRect->width(),qRect->height());
        QString outP=data.m_outBas+"map/";
        CPixmap pixm(zoom,tx1,ty1,tx2-tx1+1,ty2-ty1+1,outP,&pbMap1_1,ausschn,data.m_bpp);
        pixm.m_maxCacheDays=data.m_maxCacheDays;/*DYJ DYJTrack 2.03a */
/*DYJ DYJTrack 2.02a Start*/
        pixm.MakeMapTile(&data,nullptr,cacheMap);
        if(data.errs &&m_onErrTile!=OE_IGN)
        {
            QString errTxt,detTxt="";

            if((data.errs&ERR_PIXM_TILE)==ERR_PIXM_TILE)
                errTxt=tr("Mindestens ein Tile konnte nicht herruntergeladen werden.");
            else if((data.errs&ERR_PIXM_TILE_OLD)==ERR_PIXM_TILE_OLD)
               errTxt=tr("Mindestens ein Tile konnte nicht herruntergeladen werden, aber es gab noch eine alte Version.");
//DYJ Taho 4.07f             else if((data.errs&(ERR_PIXM_TILE_MAPNIK|ERR_PIXM_TILE_MAPNIK_OLD))>0)
//DYJ Taho 4.07f                errTxt=tr("Mindestens ein Tile konnte nicht herruntergeladen werden, es wurde die mapnik Version verwendet.");
            else if((data.errs&(ERR_PIXM_SAVE|ERR_PIXM_SAVE))>0)
               errTxt=tr("Konnte Karte nicht speichern.");
            else if((data.errs&ERR_PIXM_CANCEL)>0)
               errTxt=tr("Abbruch durch User");
            else
               errTxt=tr("unbekannter Fehler");

            QMessageBox mb(QMessageBox::Warning,tr("Karte laden"),errTxt);
            if((data.errs&ERR_PIXM_CANCEL)>0)
                mb.addButton(QMessageBox::Abort);
            else if(m_onErrTile==OE_WEITER && (data.errs&ERR_PIXM_CANCEL)==0)
/*DYJ  Ende; alt:
        int errs=pixm.MakeMapTile(&data,nullptr,cacheMap);
        if(errs &&m_onErrTile!=OE_IGN)
        {
            QString errTxt,detTxt="";

            if((errs&ERR_PIXM_TILE)==ERR_PIXM_TILE)
                errTxt=tr("Mindestens ein Tile konnte nicht herruntergeladen werden.");
            else if((errs&ERR_PIXM_TILE_OLD)==ERR_PIXM_TILE_OLD)
               errTxt=tr("Mindestens ein Tile konnte nicht herruntergeladen werden, aber es gab noch eine alte Version.");
            else if((errs&(ERR_PIXM_TILE_MAPNIK|ERR_PIXM_TILE_MAPNIK_OLD))>0)
               errTxt=tr("Mindestens ein Tile konnte nicht herruntergeladen werden, es wurde die mapnik Version verwendet.");
            else if((errs&(ERR_PIXM_SAVE|ERR_PIXM_SAVE))>0)
               errTxt=tr("Konnte Karte nicht speichern.");
            else if((errs&ERR_PIXM_CANCEL)>0)
               errTxt=tr("Abbruch durch User");
            else
               errTxt=tr("unbekannter Fehler");

            QMessageBox mb(QMessageBox::Warning,tr("Karte laden"),errTxt);
            if((errs&ERR_PIXM_CANCEL)>0)
                mb.addButton(QMessageBox::Abort);
            else if(m_onErrTile==OE_WEITER && (errs&ERR_PIXM_CANCEL)==0)
*/
            {
                mb.addButton(QMessageBox::Abort);
                mb.addButton(QMessageBox::Ignore);
            }
            else
                mb.addButton(QMessageBox::Ok);
            for(int i=0;i<data.m_errTxts.size();i++)
                detTxt+=data.m_errTxts.at(i)+"\n";
            data.m_errTxts.clear();
            mb.setDetailedText(detTxt);
            switch(mb.exec())
            {
            case QMessageBox::Abort:
                zoom=-1;
                break;
            case QMessageBox::Ignore:
                m_onErrTile=OE_IGN;
                break;
            }
        }
        //	Skalieren
        if(zoom>-1)     //sonst war Laden abgebrochen
        {
            QRect videoSize(0,0,sizeX,sizeY);
            paMap->drawImage(videoSize,*pbMap1_1);
        }
        delete(pbMap1_1);
    }
    return zoom;

}



void CMakePix::OnHelp()
{
    /*DYJ DYJTrack 2.03d Start*/
        QString url="file:./Docu/"+tr("liesmich.pdf");
    /*DYJ  Ende; alt:
        QString url="file:///./Docu/"+tr("liesmich.pdf");
    */
    QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

}

bool CMakePix::makeMapAndPtlist(QList<QPoint> &route,QPainter &paMap,QProgressDialog *pProg,bool doInit)
{
    QRect qRect;
    int zoom;
    if((zoom=loadMap(&paMap,&qRect,pProg,nullptr,doInit))==-1)
        return FALSE;
//	CListBox* pFB= (CListBox*)GetDlgItem(IDC_FILES);
    int i, maxLen=qMax(qRect.width(),qRect.height());
    int *skal=new int[static_cast<unsigned long>(maxLen)];
    int x=ui->le_aviX->text().toInt(),y= ui->le_aviY->text().toInt();
    double fakt=static_cast<double>(x)/qRect.width();
    for(i=0;i<maxLen;i++)
        skal[i]=static_cast<int>(i*fakt+.5);
    int zoomF=1<<zoom;
    double subX=qRect.x()-.5;
    double subY=qRect.y()-128.*zoomF-.5;
    char **pkte = new char*[ static_cast<unsigned long>(y) ];
    for(int y1=0;y1<y;y1++)
    {
        pkte[y1]=new char[static_cast<unsigned long>(x) ];
        memset(pkte[y1],0,static_cast<unsigned long>(x));
    }

    pProg->setLabelText(tr("Erstelle Wegpunktliste"));


    for(i=0;i<m_anzFiles;i++)
    {
        pProg->setValue(pProg->value()+1);
        if (pProg->wasCanceled())
            return FALSE;

        QListWidgetItem *item=m_pFB->item(i);

        addGpx2PtList(route,item->data(Qt::UserRole).value<CGPXInfo *>(),subX,subY,skal,zoom,pkte);
    }
    return TRUE;

}

void CMakePix::addGpx2PtList(QList<QPoint> &route,CGPXInfo *pGi, double subX,double subY,int *skal,int zoom,char **pkte)
{
    double deg2rad=acos(-1.)/180;
    int zoomF=1<<zoom;
    /*DYJ DYJTrack 2.06c Start*/
    QPoint lst(-1,pGi->m_col.get_col(pGi->m_pfad.isEmpty(),pGi->m_col.m_useTrColSp).rgb()&0xffffff);
    /*DYJ  Ende; alt:
    QPoint lst(-1,pGi->m_col.get_col(pGi->m_pfad.isEmpty()).rgb()&0xffffff);
    */
     double faktX=256./360.*zoomF;
    double faktY=-128./360.*zoomF/deg2rad;
    route.append(lst);	//Farbinfo
    subX-=128*zoomF;

    QList<CGeoPoint>::iterator aktIt,startIt,endIt;
    int step;
    if(pGi->m_rev)
    {
        startIt=pGi->m_trkPts.end()-1;
        endIt=pGi->m_trkPts.begin()-1;
        step=-1;
    }
    else
    {
        startIt=pGi->m_trkPts.begin();
        endIt=pGi->m_trkPts.end();
        step=1;
    }
    for(aktIt=startIt;aktIt!=endIt;aktIt+=step)
    {
        CGeoPoint gpAkt=*aktIt;
        if(gpAkt.m_lat<85.05&& gpAkt.m_lat>-85.05)
        {
            if(gpAkt.m_lon<m_lon[0])
                gpAkt.m_lon+=360;
            double dX=gpAkt.m_lon*faktX-subX;
            double slat=sin(gpAkt.m_lat*deg2rad);
            double dY=log((1+slat)/(1-slat))*faktY-subY;
            QPoint ptAkt(skal[static_cast<int>(dX)],skal[static_cast<int>(dY)]);
            if(lst.x()==-1)
                route.append(ptAkt);
            else if(ptAkt!=lst)
                addLine2route(route,lst,ptAkt,pkte);
            lst=ptAkt;
        }
        else
            lst.setX(-1);

    }
}

void CMakePix::addLine2route(QList<QPoint> &route,QPoint &start,QPoint &end,char **pkte)
{
    if(abs(start.x()-end.x())>1 || abs(start.y()-end.y())>1)
    {
        QPoint mid((start.x()+end.x())/2,(start.y()+end.y())/2);
        addLine2route(route,start,mid,pkte);
        addLine2route(route,mid,end,pkte);
    }

    else if(end.x()>=0&&end.x()<ui->le_aviX->text().toInt() && end.y()>=0&&end.y()<ui->le_aviY->text().toInt() && pkte[end.y()][end.x()]==0)
    {
        route.append(end);
        pkte[end.y()][end.x()]=1;
    }
}



void CMakePix::on_buttonBox_accepted()
{
    m_pOpt->m_out=ui->l_aviOutp->text();
    m_pOpt->m_x=ui->le_aviX->text().toUInt();
    m_pOpt->m_y=ui->le_aviY->text().toUInt();
    m_pOpt->m_fps=ui->le_aviFps->text().toUInt();
    m_pOpt->m_lineW=ui->le_aviLineW->text().toUInt();
    m_pOpt->m_time[0]=ui->le_aviT1->text().toUInt();
    m_pOpt->m_time[1]=ui->le_aviT2->text().toUInt();
    m_pOpt->m_time[2]=ui->le_aviT3->text().toUInt();
    m_pOpt->m_time[3]=ui->le_aviT4->text().toUInt();
    m_pOpt->m_time[4]=ui->le_aviT5->text().toUInt();
    m_pOpt->m_time[5]=ui->le_aviT6->text().toUInt();
    m_pOpt->m_source=ui->cb_quell->currentText();
    for(int row=0;row<ui->lw_over->count();row++)
    {
        QListWidgetItem *item=ui->lw_over->item(row);
        CMapSrc *map=item->data(Qt::UserRole).value<CMapSrc*>();
        map->m_useOvrs[m_pOpt->m_pixType]=(item->checkState()==Qt::Checked);
    }

}
void CMakePix::changeEvent(QEvent *e)
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
