#include "cpixmap.h"
#include <qfileinfo.h>
#include <qimage.h>
#include "cosm.h"
#include <qpainter.h>
#include <QTranslator>
#include <cmath>
#include <QTimer>
#include <QTextStream>

#define tilesize 256

/* 13.07.15 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.01c'
 * Fehler beim Tile-Download wurden nicht richtig angezeigt
 * 13.07.15 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.01f'
 * Absturz durch Zugriff auf nicht existierenden QProgressDialog behoben
 * 25.09.15 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.03a'
 * Taho liefert jetzt eine Fehlermeldung wenn das Erzeugen einer zu großen Bitmap scheitert.
 * 27.09.15 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.03c'
 * Es können jetzt als Kartengröße Werte bis 16k eingestellt werden, u.a. wegen des obigen
 * 4.10.15 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.03d'
 * bei 4.01c hatte ich noch Fehler übersehen
 *
 * 10.05.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.06a'
 * QImage.copy funktioniert nicht um zu 8 Bit Palette zu wechseln, convertToFormat funktioniert
*/

CPixmap::CPixmap(MAKEMAPSP *map, QString pfad,QString prefix)
{
    QString widthInName,file;

    m_zoom=map->zoom;
    m_xtile=map->xloop;
    m_ytile=map->yloop;
    m_pixExt=map->m_sdlm->m_pictType;
    m_maxCacheDays=map->m_sdlm->m_maxCacheDays;

    m_sizeP=map->m_sdlm->m_sizeP;
    Q_ASSERT(m_sizeP>SIZE_MIN);	//altes size ?
    switch(m_sizeP)
    {
    case 256:
        m_sizeI=0;
        break;
    case 512:
        m_sizeI=1;
        break;
    case 1024:
        m_sizeI=2;
        break;
    case 2048:
        m_sizeI=3;
        break;
    case 4096:
        m_sizeI=4;
        break;
    case 8192:
        m_sizeI=5;
        break;
    case 16384:
        m_sizeI=6;
        break;
    }
    if(m_sizeP==SIZE_FREE)
    {
        m_widthX = tilesize * map->xsize;
        m_widthY = tilesize * map->ysize;
        widthInName=QString("_p%1x%2").arg(m_widthX).arg(m_widthY);
    }
    else
    {
        m_widthX=m_widthY = m_sizeP;
        widthInName=QString("_p%1").arg(m_sizeP);
    }
    m_pfad=pfad;
    m_bpp=map->m_sdlm->m_bpp;
    m_mapID=0;
    if(m_sizeP==SIZE_NONE)
        file=QString("%1/%2/%3%4").arg(m_zoom).arg(m_xtile).arg(m_ytile).arg(m_pixExt);
    else if(m_sizeP==SIZE_QT)
    {
        m_mapID=map->m_sdlm->m_maps[0]->m_mapID;
        m_mapDefin=map->m_sdlm->m_maps[0]->m_mapDefin;
        file=QString("osm_100-%1-%2-%3-%4-%5%6").arg(m_mapDefin).arg(m_mapID).arg(m_zoom).arg(m_xtile).arg(m_ytile).arg(m_pixExt);
    }
    else if(map->m_sdlm->m_nameBy==NB_DIR)
        file=QString("%1/%2/%3").arg(m_zoom).arg(m_xtile).arg(m_ytile);
    else if(map->m_sdlm->m_nameBy==NB_COORD)
    {
        CGeoRect gr1,gr2;
        if(m_sizeP==SIZE_FREE)
        {
            Project(m_xtile,m_ytile,m_zoom, gr1);
            Project(m_xtile+map->xsize-1,m_ytile+map->ysize-1,m_zoom, gr2);
        }
        else
        {
            Project(m_xtile,m_ytile,m_zoom-m_sizeI, gr1);
            gr2=gr1;
        }
        file=QString("%1_z%2_lat%3_lon%4%5").arg(prefix).arg(m_zoom).arg((gr1.m_n+gr2.m_s)/2).arg((gr1.m_w+gr2.m_e)/2).arg(widthInName);
        file.replace(".","_");
    }
    else
        file=QString("%1_z%2_y%3_x%4%5").arg(prefix).arg(m_zoom).arg(m_ytile).arg(m_xtile).arg(widthInName);

    m_filename=file;
    m_bMap=nullptr;
}


CPixmap::CPixmap(int zoom, int xloop, int yloop, int xsize, int ysize, QString pfad,QImage **bMap, QRect rMap, int bpp)
{
    QString widthInName;
    m_sizeP=SIZE_FREE;

    m_widthX = tilesize * xsize;
    m_widthY = tilesize * ysize;
    widthInName=QString("_p%1x%2").arg(m_widthX).arg(m_widthY);
    m_pfad=pfad;
    m_bpp=bpp;
    m_bMap=bMap;
    *m_bMap=nullptr;
    m_filename="";
    m_pixExt="";
    m_zoom=zoom;
    m_xtile=xloop;
    m_ytile=yloop;
    m_maxCacheDays=0;
    m_rMap=rMap;
}

//##############################################################################
//# generates the calibration file for Pathaway for use with an windows converter
//# here can be used larger maps (size<=5)
//##############################################################################
void CPixmap::Ini_batch_PWMapConvert(QString filestub)
{

    QString outFilename=filestub+".bat";
    QFile fileOut(outFilename);
    if(fileOut.open( QIODevice::WriteOnly)!=0 )
    {
        char txt[]="echo off\n\n";
        fileOut.write(txt,static_cast<long>(strlen(txt)));
        fileOut.close();
    }
}
//##############################################################################
//# download the tiles and glow them togehter
//##############################################################################
void CPixmap::MakeMapTile(SDLM_DATA *data, CGeoRect *pgRect, bool cacheMap)
{
    int err=0,xtile1,xtile2,ytile1,ytile2;
    if(m_sizeP==SIZE_FREE)
    {
        xtile1 = m_xtile;
        xtile2 = m_xtile +m_widthX/tilesize-1;
        ytile1 = m_ytile ;
        ytile2 = m_ytile+m_widthY/tilesize-1;
    }
    else
    {
        int size_256=m_sizeP/256;
        xtile1 = m_xtile * size_256;
        xtile2 = (m_xtile * size_256) + size_256 -1;
        ytile1 = m_ytile * size_256;
        ytile2 = (m_ytile * size_256) + size_256 -1;
    }
    if(pgRect)
    {
        Project(xtile1 , ytile1 , m_zoom,*pgRect) ;
        CGeoRect gRect2;
        Project(xtile2 , ytile2 , m_zoom,gRect2) ;
        pgRect->m_s=gRect2.m_s;
        pgRect->m_e=gRect2.m_e;
    }

    static QImage lst;
    static int lstX=-1,lstY=-1,lstX1=-1,lstX2=-1,lstY1=-1,lstY2=-1;
    QImage bImg32(m_widthX, m_widthY,QImage::Format_RGB32); //32Bit/Pixel Bild
    if(bImg32.isNull())
    {
        data->errs|=ERR_PIXM_CREATE;
        return;
    }
    bImg32.fill(Qt::white);

    QPainter gImg(&bImg32);
    if(lstX==m_widthX && lstY==m_widthY && lstX1==xtile1 && lstX2==xtile2&& lstY1==ytile1 && lstY2==ytile2)
        gImg.drawImage(0,0,lst);
    else
    {
        for(int ov=0;ov<data->m_maps.size();ov++)	//CHG: TAHO 2.11a DYJ
        {
            QString tilePath;
            CMapSrc * map=static_cast<CMapSrc *>(data->m_maps[ov]);
            tilePath=QString("%1%2/").arg(data->m_outBas).arg(map->m_name);
            COsm osm(m_zoom,map->m_url,map->m_ext,tilePath,m_maxCacheDays);
            for(int x = xtile1; x <= xtile2; x++)	//Karte aus mehreren Tiles zusammensetzen.
            {
                for(int y = ytile1; y <= ytile2; y++)
                {
                    bool draw=true;	//CHG: DYJTracker 1.01a DYJ
                    QString tile1Path,errTxt;
                    switch(osm.getsaveTileWithCache(x,y,tile1Path,&errTxt))
                    {
                    case 1:	//Laden erfolglos
                        data->m_errTxts.append(errTxt);	//CHG: TAHO 2.10c DYJ
                        if(ov)
                            draw=false;
                        else
                        {
                            tile1Path=data->m_tna;
                            err|= ERR_PIXM_TILE;
                            data->m_errTxts.append(errTxt);
                        }
                        break;
                    case 2: //Laden erfolglos aber altes File vorhanden
                        data->m_errTxts.append(errTxt);
                        err|= ERR_PIXM_TILE_OLD;
                        break;
                    case 3: //y ungültig ->schwarz
                        draw=false;
                        break;
                    }
                    if(draw)
                    {
                        QImage tile(tile1Path);
                        gImg.drawImage( (x-xtile1)*tilesize, (y-ytile1)*tilesize,tile);
                    }
                }
                if(data->m_progD && data->m_progD->wasCanceled())
                {
                    data->errs|=ERR_PIXM_CANCEL;
                    return;
                }
            }
        }
        if(!lst.isNull())
            lst=QImage();
        if(cacheMap)
        {
            lst=bImg32.copy(0,0,m_widthX, m_widthY);
            lstX=m_widthX;
            lstY=m_widthY;
            lstX1=xtile1;
            lstX2=xtile2;
            lstY1=ytile1;
            lstY2=ytile2;
        }
    }
    if(data->m_progD)
    {
        data->m_progD->setValue(data->m_progD->value()+1);
        if(data->m_progD->wasCanceled())
        {
            data->errs|=ERR_PIXM_CANCEL;
            return;
        }
    }
    // save the altered image.


// Bittiefe anpassen und als File speichern und/oder als QImage zurückgeben
    bool saveFile=false;
    QString savePath=m_pfad+m_filename+m_pixExt;
    if(!m_pixExt.isEmpty())
    {
        QFileInfo fi(savePath);
        if(!fi.path().isEmpty())	//vorsichtshalber Dir erzeugen
        {
            QFileInfo fi(savePath);
            QDir dir;
            dir.mkpath(fi.path());
        }
        saveFile=true;
        if(err)
        {
            QString errTxt=QString(QObject::tr("-> %1 hat Fehler<br>\r\n")).arg(savePath);
            data->m_errTxts.append(errTxt);	//CHG: TAHO 2.10c DYJ
        }
    }
    else if(err)
        data->m_errTxts.append("-> The Image has errors<br>\r\n");
    if (saveFile||m_bMap)
    {
        bool ok=!saveFile;
        switch(data->m_bpp)
        {
        case 0:	//8Bit/Pixel
            {
            QRect rc(0, 0, bImg32.width(), bImg32.height());
            QImage bImg8 = bImg32.convertToFormat(QImage::Format_Indexed8);
            if(saveFile)
                ok=bImg8.save(savePath);
            if(m_bMap)
            {
                QImage *neuImg=new QImage;
                *neuImg=bImg8.copy(m_rMap);
                *m_bMap=neuImg;
            }
            break;
            }
        case 1:	//24Bit/Pixel
            {
            QRect rc(0, 0, bImg32.width(), bImg32.height());
            QImage bImg24(bImg32.width(), bImg32.height(), QImage::Format_RGB888);
            bImg24 = bImg32.copy(rc);
            if(saveFile)
                 ok=bImg24.save(savePath);
            if(m_bMap)
            {
                QImage *neuImg=new QImage;
                *neuImg=bImg32.copy(m_rMap);
                *m_bMap=neuImg;
            }
            break;
            }
        case 2:	//32Bit/Pixel
            if(saveFile)
                ok=bImg32.save(savePath);
            if(m_bMap)
            {
                QImage *neuImg=new QImage(m_rMap.width(),m_rMap.height(),QImage::Format_RGB32);
                *neuImg=bImg32.copy(m_rMap);
                *m_bMap=neuImg;
            }
            break;
        }
        if(!ok)
            err=ERR_PIXM_SAVE;
    }
    else
        err=ERR_PIXM_SAVE;


    data->errs|=err;
}

//##############################################################################
//# helper sub for Project()
//##############################################################################

double CPixmap::ProjectMercToLat(double mercY)
{
  return( 180/PI* atan(sinh(mercY)));
}
//##############################################################################
//# transform the x,y tilenumbers to the S,W,N,E edge ccordinates
//##############################################################################
void CPixmap::Project(int x, int y, int zoom, CGeoRect &gr)
{
    double unit = 1. / (1<<zoom);
    double relY1 = y * unit;
    double relY2 = relY1 + unit;

//	double limitY = ProjectF(85.0511);
    double limitY = PI;

    double rangeY = 2 * limitY;

    relY1 = limitY - rangeY * relY1;
    relY2 = limitY - rangeY * relY2;

    gr.m_n = ProjectMercToLat(relY1);
    gr.m_s = ProjectMercToLat(relY2);

    unit = 360. / (1<<zoom);
    gr.m_w = -180 + x * unit;

    gr.m_e=gr.m_w+unit;
}
//##############################################################################
//# helper sub for Project()
//##############################################################################
double CPixmap::ProjectF(double winkel)
{
  double lat = PI/180 * winkel;
  return(log(tan(lat) + 1/cos(lat)));
}
CPixmap::~CPixmap()
{

}


int CPixmap::MakeMapCal(bool *pMakeKal,QString &PWconvBatchname,CGeoRect *pgRect)
//##############################################################################
//# generates all the calibration files
//##############################################################################
{
    int err=0;
    CGeoRect gr;
    if(pgRect)
        gr=*pgRect;
    else if(m_sizeP==SIZE_NONE || m_sizeP==SIZE_QT)

       return ERR_PIXM_KAL;
    else
        Project(m_xtile , m_ytile , m_zoom-m_sizeI,gr) ;


//  generate calibration file for OZI: $out$file.map
    if(pMakeKal[OZI])
        err|=GenCalibrationMap_OZI(gr);
//  generate calibration file for Fugawi: $out${file}.jpr
    if(pMakeKal[FUGAWI])
        err|=GenCalibrationMap_Fugawi(gr);


//  generate calibration file for TTQV: $out${file}_$extension.cal
    if(pMakeKal[TTQV])
        err|=GenCalibrationMap_TTQV(gr);
//  generate calibration file for UI-View: $out${file}_$extension.inf
    if(pMakeKal[UIVIEW])
        err|=GenCalibrationMap_UI_VIEW(gr);

//  generate calibration file for Glopus: $out${file}.kal
    if(pMakeKal[GLOPUS])
        err|=GenCalibrationMap_Glopus(gr);
//  generate World file for GIS
    if(pMakeKal[WORLDFILE])
        err|=GenWorldFile(gr);
//  generate kml file for Google Earth
    if(pMakeKal[KML])
        err|=GenKml(gr);

//  generate calibration file for Pathaway with converter: $out${file}.pwm
    if(pMakeKal[PATHAWAY])
    {
        err|=GenCalibrationMap_PathawayConverter(gr);
//  generate calibration file for Pathaway direkt: $out${file}_$extension.pwm
        err|=GenCalibrationMap_PathawayDirect(gr);

//  generate batch sript for Pathaway converting
        err|=Gen_batch_PWMapConvert(PWconvBatchname);
    }
//  generate calibration file for GPSTuner: $out${file}.gmi
    if(pMakeKal[GMI])	//CHG: TAHO 2.10b SG
        err|=GenCalibrationMap_GMI(gr);
    return err;
}

//##############################################################################
//# generates the calibration file for OZI
//##############################################################################
int CPixmap::GenCalibrationMap_OZI(CGeoRect &gr)
{
    int width_xm1 = m_widthX-1,width_ym1 = m_widthY-1,err=0;
    QString outFilename=m_pfad+m_filename+".map";
    QFile fOut(outFilename);
    if(fOut.open( QIODevice::WriteOnly)!=0)
    {
        QString nn,ss,ww,ee;
        if (gr.m_w>0)
            ww="E";
        else
            ww="W";
        if (gr.m_e>0)
            ee="E";
        else
            ee="W";
        if (gr.m_n>0)
            nn="N";
        else
            nn="S";
        if (gr.m_s>0)
            ss="N";
        else
            ss="S";

        QTextStream out(&fOut);
        out << "OziExplorer Map Data File Version 2.2\r\n";
        out << m_filename << "\r\n";
        out << m_filename << m_pixExt << "\r\n";
        out << "1 ,Map Code,\r\n";
        out << "WGS 84,WGS 84,   0.0000,   0.0000,WGS 84\r\n";
        out << "Reserved 1\r\n";
        out << "Reserved 2\r\n";
        out << "Magnetic Variation,,,E\r\n";
        out << "Map Projection,Mercator,PolyCal,No,AutoCalOnly,No,BSBUseWPX,No\r\n";
        out << "Point01,xy,    0,    0,                 in, deg,  " << fabs(gr.m_n) << ",0.0," << nn << ",  " << fabs(gr.m_w) << ",0.0," << ww << ", grid,   , , ,N\r\n";
        out << "Point02,xy, " << width_xm1 << ", " << width_ym1 << ", in, deg,  " << fabs(gr.m_s) << ",0.0,  " << ss << ",  " << fabs(gr.m_e) << ",0.0," << ee << ", grid,   , , ,N\r\n";
        out << "Point03,xy, " << width_xm1 << ",0,             in, deg,  " << fabs(gr.m_n) << ",0.0," << nn << ",  " << fabs(gr.m_e) << ",0.0," << ee << ", grid,   , , ,N\r\n";
        out << "Point04,xy, 0, " << width_ym1 << ",            in, deg,  " << fabs(gr.m_s) << ",0.0," << ss << ",  " << fabs(gr.m_w) << ",0.0," << ww << ", grid,   , , ,N\r\n";
        out << "Projection Setup,,,,,,,,,,\r\n";
        out << "Map Feature = MF ; Map Comment = MC     These follow if they exist\r\n";
        out << "Track File = TF      These follow if they exist\r\n";
        out << "Moving Map Parameters = MM?    These follow if they exist\r\n";
        out << "MM0,Yes\r\n";
        out << "MMPNUM,4\r\n";
        out << "MMPXY,1,0,0\r\n";
        out << "MMPXY,2," << m_widthX << ",0\r\n";
        out << "MMPXY,3," << m_widthX << "," << m_widthY << "\r\n";
        out << "MMPXY,4,0," << m_widthY << "\r\n";

        out << "MMPLL,1,  " << gr.m_w << ",  " << gr.m_n << "\r\n";
        out << "MMPLL,2,  " << gr.m_e << ",  " << gr.m_n << "\r\n";
        out << "MMPLL,3,  " << gr.m_e << ",  " << gr.m_s << "\r\n";
        out << "MMPLL,4,  " << gr.m_w << ",  " << gr.m_s << "\r\n";
//  # calcualate meter per pixel
        double mm1b = 40004./360.*1000.*(gr.m_n-gr.m_s)/m_widthY  ;//Eigentlich Auflösung in OW, aber bei der OSM-Projektion ist das gleich
//  $Data =~ s/--MM1B--/$MM1B/g;
        out << "MM1B," << mm1b << "\r\n";
        out << "LL Grid Setup\r\n";
        out << "LLGRID,No,No Grid,Yes,255,16711680,0,No Labels,0,16777215,7,1,Yes,x\r\n";
        out << "Other Grid Setup\r\n";
        out << "GRGRID,No,No Grid,Yes,255,16711680,No Labels,0,16777215,8,1,Yes,No,No,x\r\n";
        out << "MOP,Map Open Position,0,0\r\n";
        out << "IWH,Map Image Width/Height," << m_widthX << "," << m_widthY << "\r\n";

        fOut.close();
    }
    else
        err=ERR_PIXM_KAL;
    return err;

}
//##############################################################################
//# generates the calibration file for FUGAWI
//##############################################################################
int CPixmap::GenCalibrationMap_Fugawi(CGeoRect &gr)
{
    //# create the FUGAWI calibration file

    int width_xm1 = m_widthX-1,width_ym1 = m_widthY-1,err=0;
    QString outFilename=m_pfad+m_filename+".jpr";
    QFile fOut(outFilename);
    if(fOut.open( QIODevice::WriteOnly)!=0)
    {
        QTextStream out(&fOut);
        out << "nm=" << m_filename << "\r\n";
        out << "it=" << m_pixExt.mid(1) << "\r\n";
        out << "dm=WGS84\r\n";
        out << "pr=Mercator\r\n";
        out << "pp=9\r\n";
        out << "rp1=" << gr.m_n << "," << gr.m_w << ",0,0\r\n";
        out << "rp2=" << gr.m_n << "," << gr.m_e << "," << width_xm1 << ",0\r\n";
        out << "rp3=" << gr.m_s << "," << gr.m_e << "," << width_xm1 << "," << width_ym1 << "\r\n";
        out << "rp4=" << gr.m_s << "," << gr.m_w << ",0," << width_ym1 << "\r\n";
        out << "vp1=0,0\r\n";
        out << "vp2=" << width_xm1 << ",0\r\n";
        out << "vp3=" << width_xm1 << "," << width_ym1 << "\r\n";
        out << "vp4=0," << width_ym1 << "\r\n";
        fOut.close();
    }
    else
        err=ERR_PIXM_KAL;
    return err;
}


int CPixmap::GenCalibrationMap_TTQV(CGeoRect &gr)

//##############################################################################
//# generates the calibration file for TTQV
//##############################################################################
{
    int width_xm1 = m_widthX-1,width_ym1 = m_widthY-1,err=0;
    QString outFilename=m_pfad+m_filename+".cal";
    QFile fOut(outFilename);
    if(fOut.open( QIODevice::WriteOnly)!=0)
    {
        QTextStream out(&fOut);
        out << "name = 10 = " << m_filename << "\r\n";
        out << "fname = 10 = " << m_filename << m_pixExt << "\r\n";
        out << "nord = 6 = " << gr.m_n << "\r\n";
        out << "sued = 6 = " << gr.m_s << "\r\n";
        out << "ost = 6 = " << gr.m_e << "\r\n";
        out << "west = 6 = " << gr.m_w << "\r\n";
        out << "scale_area = 6 =  2.980008e-010\r\n";
        out << "proj_mode = 10 = proj\r\n";
        out << "projparams = 10 = proj=merc lon_0=9\r\n";
        out << "datum1 = 10 = WGS 84# 6378137# 298.257223563# 0# 0# 0#\r\n";
        out << "c1_x = 7 =  0\r\n";
        out << "c1_y = 7 =  0\r\n";
        out << "c2_x = 7 =  " << width_xm1 << "\r\n";
        out << "c2_y = 7 =  0\r\n";
        out << "c3_x = 7 =  " << width_xm1 << "\r\n";
        out << "c3_y = 7 =  " << width_ym1 << "\r\n";
        out << "c4_x = 7 =  0\r\n";
        out << "c4_y = 7 =  " << width_ym1 << "\r\n";
        out << "c1_lat = 7 =  " << gr.m_n << "\r\n";
        out << "c1_lon = 7 =  " << gr.m_w << "\r\n";
        out << "c2_lat = 7 =  " << gr.m_n << "\r\n";
        out << "c2_lon = 7 =  " << gr.m_e << "\r\n";
        out << "c3_lat = 7 =  " << gr.m_s << "\r\n";
        out << "c3_lon = 7 =  " << gr.m_e << "\r\n";
        out << "c4_lat = 7 =  " << gr.m_s << "\r\n";
        out << "c4_lon = 7 =  " << gr.m_w << "\r\n";
        fOut.close();
    }
    else
        err=ERR_PIXM_KAL;
    return err;
}



//##############################################################################
//# generates the calibration file for Pathaway for direct use
//# the maps shall be have a size<=3 !
//##############################################################################
int CPixmap::GenCalibrationMap_Glopus(CGeoRect &gr)
{
    //# create the Glopus calibration file

    int width_xm1 = m_widthX-1,width_ym1 = m_widthY-1,err=0;
    QString outFilename=m_pfad+m_filename+".kal";
    QFile fOut(outFilename);
    if(fOut.open( QIODevice::WriteOnly)!=0)
    {
        QTextStream out(&fOut);
        out << "\r\n[Kalibrierung]\r\n\r\n";
        out << "[Calibration Point 1]\r\n";
        out << "Longitude = " << gr.m_w << "\r\n";
        out << "Latitude = " << gr.m_n << "\r\n";
        out << "Pixel = POINT(0,0)\r\n";
        out << "[Calibration Point 2]\r\n";
        out << "Longitude = " << gr.m_e << "\r\n";
        out << "Latitude = " << gr.m_s << "\r\n";
        out << "Pixel = POINT(" << width_xm1 << "," << width_ym1 << ")\r\n";
        out << "[Calibration Point 3]\r\n";
        out << "Longitude = " << gr.m_e << "\r\n";
        out << "Latitude = " << gr.m_n << "\r\n";
        out << "Pixel = POINT(" << width_xm1 << ",0)\r\n";
        out << "[Calibration Point 4]\r\n";
        out << "Longitude = " << gr.m_w << "\r\n";
        out << "Latitude = " << gr.m_s << "\r\n";
        out << "Pixel = POINT(0," << width_ym1 << ")\r\n";
        out << "[Map]\r\n";
        out << "Bitmap = " << m_filename << m_pixExt << "\r\n";
        out << "Size = SIZE(" << m_widthX << "," << m_widthY << ")\r\n";
        out << "[Projection]\r\n";
        out << "Projection = Mercator\r\n";
        out << "CoSys = Mercator\r\n";
        fOut.close();
    }
    else
        err=ERR_PIXM_KAL;
    return err;
}



//##############################################################################
//# generates the Worldfile for GIS-Programs
//##############################################################################

int CPixmap::GenWorldFile(CGeoRect &gr)
{
    int err=0;
    QString outFilename=m_pfad+m_filename+".wld";
    QFile fOut(outFilename);
    if(fOut.open( QIODevice::WriteOnly)!=0)
    {
        double merc_a = 6378137,merc_e = 8.18191908426219E-02;
//		double pi = 3.14159265358979;
        //West
        int olX = int(merc_a * gr.m_w / 180 * PI);

        //Nord
        double latitude = gr.m_n / 180 * PI;
        double e_x_sinlat = merc_e * sin(latitude);
        double tan_temp = tan(PI / 4 + latitude / 2);
        double pow_temp = pow((1 - e_x_sinlat) / (1 + e_x_sinlat),(merc_e / 2));
        double ctanz2 = tan_temp * pow_temp;
        int olY = int(merc_a * log(ctanz2));


        //Ost

        int orX = int(merc_a * gr.m_e / 180 * PI);

        //Süd
        latitude = gr.m_s / 180 * PI;
        e_x_sinlat = merc_e * sin(latitude);
        tan_temp = tan(PI / 4 + latitude / 2);
        pow_temp = pow((1 - e_x_sinlat) / (1 + e_x_sinlat),(merc_e / 2));
        ctanz2 = tan_temp * pow_temp;
        int ulY = int(merc_a * log(ctanz2));



        double m1 = double(orX - olX) / m_widthX;
        double m2 = -double(olY - ulY) / m_widthY;

        QTextStream out(&fOut);
        out << m1 << "\r\n";
        out << "0\r\n";
        out << "0\r\n";
        out << m2 << "\r\n";
        out << olX << "\r\n";
        out << olY << "\r\n";
        fOut.close();
    }
    else
        err=ERR_PIXM_KAL;
    return err;

}

//##############################################################################
//# generates the calibration file for Pathaway for use with an windows converter
//# here can be used larger maps (size<=5)
//##############################################################################
int CPixmap::GenCalibrationMap_PathawayConverter(CGeoRect &gr)
{
// create the pathaway calibration file
    int err=0;
    QString outFilename=m_pfad+m_filename+".pwm";
    QFile fOut(outFilename);
    if(fOut.open( QIODevice::WriteOnly)!=0)
    {
        QTextStream out(&fOut);

        out << "latitudeTL=" << gr.m_n << "\r\n";
        out << "longitudeTL=" << gr.m_w << "\r\n";
        out << "latitudeBR=" << gr.m_s << "\r\n";
        out << "longitudeBR=" << gr.m_e << "\r\n";
        out << "imageType=bmp\r\n";
        out << "name=" << m_filename << "\r\n";
        fOut.close();
    }
    else
        err=ERR_PIXM_KAL;
    return err;
}





//##############################################################################
//# generates the calibration file for Pathaway for direct use
//# the maps shall be have a size of 2!
//##############################################################################
int CPixmap::GenCalibrationMap_PathawayDirect(CGeoRect &gr)
{
// create the pathaway calibration file
    int err=0;
    QString outFilename;
    outFilename=QString("%1%2_%3.pwm").arg(m_pfad).arg(m_filename).arg(m_pixExt.mid(1));
    QFile fOut(outFilename);
    if(fOut.open( QIODevice::WriteOnly)!=0)
    {
        QTextStream out(&fOut);

        out << "latitudeTL=" << gr.m_n << "\r\n";
        out << "longitudeTL=" << gr.m_w << "\r\n";
        out << "latitudeBR=" << gr.m_s << "\r\n";
        out << "longitudeBR=" << gr.m_e << "\r\n";
        out << "imageType=" << m_pixExt.mid(1) << "\r\n";
        out << "name=" << m_filename << "\r\n";
        fOut.close();
    }
    else
        err=ERR_PIXM_KAL;
    return err;
}



//##############################################################################
//# generates the windows batch file for the pathaway windows converter
//##############################################################################
int CPixmap::Gen_batch_PWMapConvert(QString &filestub)
{
//  #append current file to the convertion batch file
    int err=0;
    QString outFilename=filestub+".bat";
    QFile fOut(outFilename);
    if(fOut.open( QIODevice::WriteOnly)!=0)
    {
        QTextStream out(&fOut);
        out << "if not exist $map.bmp (\n echo You need to convert the File $map.$extension\r\n";
        out << " echo to 256 color BMP format before proceeding to run this batch file!\r\n pause\r\n exit\r\n)\r\n";
        out << "PWMapConvert.exe v2,$map.bmp,8,$map.pwm\n\n";
        fOut.close();
    }
    else
        err=ERR_PIXM_KAL;
    return err;
}

int CPixmap::GenCalibrationMap_UI_VIEW(CGeoRect &gr)
{
    //# create the FUGAWI calibration file
    int err=0;

    QString outFilename=m_pfad+m_filename+".inf";
    QFile fOut(outFilename);
    if(fOut.open( QIODevice::WriteOnly)!=0)
    {
        QString nn,ss,ww,ee;
        if (gr.m_w>0)
            ww="E";
        else
            ww="W";
        if (gr.m_e>0)
            ee="E";
        else
            ee="W";
        if (gr.m_n>0)
            nn="N";
        else
            nn="S";
        if (gr.m_s>0)
            ss="N";
        else
            ss="S";
        double wm=fabs(gr.m_w);
        int wg=static_cast<int>(wm);
        wm-=wg;
        wm*=60;

        double em=fabs(gr.m_e);
        int eg=static_cast<int>(em);
        em-=eg;
        em*=60;

        double nm=fabs(gr.m_n);
        int ng=static_cast<int>(nm);
        nm-=ng;
        nm*=60;

        double sm=fabs(gr.m_s);
        int sg=static_cast<int>(sm);
        sm-=sg;
        sm*=60;

        QTextStream out(&fOut);
        QString snm,swm,ssm,sem;
        snm.sprintf("%05.2f",nm);
        swm.sprintf("%05.2f",wm);
        ssm.sprintf("%05.2f",sm);
        sem.sprintf("%05.2f",em);
        out << ng << "." << snm << nn << "," << wg << "." << swm << ww << "\r\n";
        out << sg << "." << ssm << ss << "," << eg << "." << sem << ee << "\r\n";
        out << m_filename << "\r\n";
        fOut.close();
    }
    else
        err=ERR_PIXM_KAL;
    return err;

}


int CPixmap::GenKml(CGeoRect &gr)
{
    //# create the kml-File

    int err=0;
    QString outFilename=m_pfad+m_filename+".kml";
    QFile fOut(outFilename);
    if(fOut.open( QIODevice::WriteOnly)!=0)
    {
        QTextStream out(&fOut);
        out <<"<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\r\n";	//CHG: TAHO 2.10g DYJ
        out <<"<kml>\r\n";
        out <<"\t<Document>\r\n";
        out <<"\t\t<name>" << m_filename << "</name>\r\n";
        out <<"\t\t<description>" << m_filename << "</description>\r\n";
        out <<"\t\t<Folder>\r\n";
        out <<"\t\t\t<name>" << m_filename << "</name>\r\n";
        out <<"\t\t\t<GroundOverlay>\r\n";
        out <<"\t\t\t\t<name>" << m_filename <<  m_pixExt << "</name>\r\n";
        out <<"\t\t\t\t<color>d8ffffff</color>\r\n";
        out << "\t\t\t\t<Icon>\r\n";
        out << "\t\t\t\t\t<href>" << m_filename <<  m_pixExt << "</href>\r\n";
        out << "\t\t\t\t\t<viewBoundScale>1.0</viewBoundScale>\r\n";
        out << "\t\t\t\t</Icon>\r\n";
        out << "\t\t\t\t<LatLonBox>\r\n";
        out << "\t\t\t\t\t<north>" << gr.m_n << "</north>\r\n";
        out << "\t\t\t\t\t<south>" << gr.m_s << "</south>\r\n";
        out << "\t\t\t\t\t<east>" << gr.m_e << "</east>\r\n";
        out << "\t\t\t\t\t<west>" << gr.m_w << "</west>\r\n";
        out << "\t\t\t\t\t<rotation>0.0</rotation>\r\n";
        out << "\t\t\t\t</LatLonBox>\r\n";
        out << "\t\t\t</GroundOverlay>\r\n";
        out << "\t\t</Folder>\r\n";
        out << "\t</Document>\r\n";
        out << "</kml>\r\n";
        fOut.close();
    }
    else
        err=ERR_PIXM_KAL;
    return err;

}

int CPixmap::GenCalibrationMap_GMI(CGeoRect &gr)	//CHG: TAHO 2.10b SG
{

    //# create the GPSTuner calibration file

    int width_xm1 = m_widthX-1,width_ym1 = m_widthY-1,err=0;
    QString outFilename=m_pfad+m_filename+".gmi";
    QFile fOut(outFilename);
    if(fOut.open( QIODevice::WriteOnly)!=0)
    {
        QTextStream out(&fOut);
        out << "Map Calibration data file v3.0\r\n";
        out <<  m_filename <<  m_pixExt << "\r\n";
        out <<  m_widthX << "\r\n";
        out <<  m_widthY << "\r\n";
        out << "0;0;" << gr.m_w << ";" << gr.m_n << "\r\n";
        out <<  width_xm1 << ";" << width_ym1 << ";" << gr.m_e << ";" << gr.m_s << "\r\n";
        out <<  width_xm1 << ";0;" << gr.m_e << ";" << gr.m_n << "\r\n";
        out << "0;" << width_ym1 << ";" << gr.m_w << ";" << gr.m_s << "\r\n";
        out << "Border and Scale\r\n";
        out <<  gr.m_n << ";" << gr.m_w << ";" << gr.m_s << ";" << gr.m_e << "\r\n";
        out <<  width_xm1/fabs(gr.m_w-gr.m_e) << ";" << width_ym1/fabs(gr.m_n-gr.m_s) << "\r\n";
        fOut.close();
    }
    else
        err=ERR_PIXM_KAL;
    return err;
}

void CPixmap::LoadTile(SDLM_DATA *data, int overlay)
{
    QString tilePath;
    CMapSrc * map=static_cast<CMapSrc *>(data->m_maps[overlay]);
    if(m_mapID)
    {
        QString fn=QString("osm_100-%1-%2-").arg(m_mapDefin).arg(m_mapID);
        QDir dirH;
        dirH.mkpath(data->m_outBas);
        QFileInfo fi(data->m_outBas,fn);
        tilePath=fi.filePath();
    }
    else
    {
        QFileInfo fi(data->m_outBas,map->m_name);
        tilePath=fi.filePath()+"/";
    }
    COsm osm(m_zoom,map->m_url,map->m_ext,tilePath,m_maxCacheDays);
    QString tile1Path,errTxt;
    QString addExt="";
    if(!data->m_pictType.left(4).compare(".png",Qt::CaseInsensitive))
        addExt=data->m_pictType.mid(4);
    switch(osm.getsaveTileWithCache(m_xtile,m_ytile,tile1Path,&errTxt,addExt))
    {
    case 1:	//Laden erfolglos
        data->m_errTxts.append(errTxt);
//        data->m_errTxts.append(tile1Path.mid(1)+="<br>\r\n");
        data->errs|= ERR_PIXM_TILE;
        break;
    case 2: //Laden erfolglos aber altes File vorhanden
        data->m_errTxts.append(errTxt);
//        data->m_errTxts.append(tile1Path.mid(1)+"<br>\r\n");
        data->errs|= ERR_PIXM_TILE_OLD;
        break;
    }

}

