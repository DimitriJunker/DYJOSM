#ifndef CPIXMAP_H
#define CPIXMAP_H
#include <qvector.h>
#include <qstring.h>
#include "cmapsrc.h"
#include "cgeorect.h"
#include <QProgressDialog>


enum {EVT_PROGDIA,EVT_LD_FILE,EVT_PROG,EVT_END};

typedef struct
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned int anz;
    bool be;
}HISTELEM;

#define ERR_PIXM_TILE 1
#define ERR_PIXM_TILE_OLD 2
#define ERR_PIXM_SAVE 16
#define ERR_PIXM_KAL 32
#define ERR_PIXM_CANCEL 64
#define ERR_PIXM_CREATE 128
enum {FUGAWI,GLOPUS,GMI,KML,OZI,PATHAWAY,TTQV,UIVIEW,WORLDFILE,ANZ_KAL};	//CHG: TAHO 2.10b SG

enum {NB_NUM,NB_COORD,NB_DIR};
struct SDLM_DATA
{
    int m_sizeP;
    int m_bpp;
    QString m_outBas;
    QString m_PWconvBatchname;
    QVector<CMapSrc *> m_maps;
    bool m_makeKal[ANZ_KAL];
    int m_done;
    int m_nameBy;
    bool m_vector;
    unsigned int m_maxCacheDays;
    int errs;
    QStringList m_errTxts;
    QString m_tna;
    QString m_pictType;
    QString m_packer;
    QString m_unGz;
    QProgressDialog *m_progD;
    QString m_offDir;
};

typedef struct
{
    int	zoom;
    int xloop;
    int yloop;
    int xsize;
    int ysize;
    SDLM_DATA *m_sdlm;
}MAKEMAPSP;
typedef struct
{
    CGeoRect rect;
    CMapSrc *map;
    SDLM_DATA *m_sdlm;

}MAKEMAPSV;

#define MAKEARGB(a, r, g, b) \
                (((ARGB) ((a) & 0xff) << ALPHA_SHIFT)| \
                 ((ARGB) ((r) & 0xff) << RED_SHIFT)  | \
                 ((ARGB) ((g) & 0xff) << GREEN_SHIFT)| \
                 ((ARGB) ((b) & 0xff) << BLUE_SHIFT))

#define ERR_PIXMAP 0x100

class CPixmap
{
public:
    int m_sizeI;
    int GenCalibrationMap_GMI(CGeoRect &gr);
    int GenKml(CGeoRect &gr);
    CPixmap(int zoom, int xloop,int yloop,int xsize,int ysize,QString pfad,QImage **bMap,QRect rMap,int bpp);
    CPixmap(MAKEMAPSP *map, QString pfad, QString prefix="OSM");
    void LoadTile(SDLM_DATA *data, int overlay=0);
    unsigned int m_maxCacheDays;
    int MakeMapCal(bool *pMakeKal,QString &PWconvBatchname,CGeoRect *pgRect=nullptr);
    void MakeMapTile(SDLM_DATA *data, CGeoRect *pgRect=nullptr, bool cacheMap=false);
    static void Ini_batch_PWMapConvert(QString filestub);
    virtual ~CPixmap();
    QString m_filename;

private:
    int GenWorldFile(CGeoRect &gr);
    int GenCalibrationMap_UI_VIEW(CGeoRect &gr);
    QRect m_rMap;
    QImage ** m_bMap;
    void clone32_8(QImage &bImg8,QImage &bImg32,bool volatile &cancel);
    int elimCol(HISTELEM *hist,unsigned int &anz,unsigned int nr,unsigned int beGrenze);
    double ProjectF(double winkel);
    static void Project(int x,int y, int zoom,CGeoRect &gr);
    static double ProjectMercToLat(double mercY);
    int Gen_batch_PWMapConvert(QString &filestub);
    int GenCalibrationMap_PathawayDirect(CGeoRect &gr);
    int GenCalibrationMap_PathawayConverter(CGeoRect &gr);
    int GenCalibrationMap_Glopus(CGeoRect &gr);
    int GenCalibrationMap_TTQV(CGeoRect &gr);
    int GenCalibrationMap_Fugawi(CGeoRect &gr);
    int GenCalibrationMap_OZI(CGeoRect &gr);
//QImage    int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
    int m_zoom;
    int m_sizeP;

    int m_bpp;
    int m_ytile;
    int m_xtile;
    int m_widthX;
    int m_widthY;
    int m_mapID;
    QString m_mapDefin;
    QString m_pixExt;
    QString m_pfad;
};
enum{SIZE_MIN=9,SIZE_FREE,SIZE_NONE,SIZE_VECTOR,SIZE_QT};

#endif // CPIXMAP_H
