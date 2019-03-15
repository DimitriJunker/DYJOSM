#ifndef CMAKEPIX_H
#define CMAKEPIX_H

#include <QDialog>
#include "cgpxinfo.h"
#include <qlistwidget.h>
#include <windows.h>
#include <QProgressDialog>

#define T_NA "tna.png"

extern QString OsmUrl;   //DYJ DYJTrack 2.06d

typedef struct
{
public:
    QString m_out;
    int m_pixType;
    unsigned int	m_x;
    unsigned int	m_y;
    unsigned int	m_fps;
    unsigned int	m_lineW;
    unsigned int	m_time[6];
    QString	m_source;
}SPixOpt;
enum{PIX_PICT,PIX_AVI};
enum{OE_NUROK,OE_WEITER,OE_IGN};
namespace Ui {
class CMakePix;
}

class CMakePix : public QDialog
{
    Q_OBJECT

public:
    explicit CMakePix(QWidget *parent = 0);
    ~CMakePix();
    SPixOpt * m_pOpt;
    QListWidget* m_pFB;
    double m_lat[4];
    double m_lon[4];
    QString m_prgPath;
    int m_anzFiles;
    void makePict();
    int makeAvi();
    bool makeMapAndPtlist(QList<QPoint> &route,QPainter &paMap,QProgressDialog *prog, bool doInit=true);
    int GetEncoderClsid(const WCHAR *format, CLSID *pClsid);
    void addLine2route(QList<QPoint> &route,QPoint &start,QPoint &end,char **pkte);
    // maximale Länge eines Avi Files (def 2GB)
    qint64 m_maxAviLen;
    QString m_showFile;
    void initDialog();
protected:
    void changeEvent(QEvent *e);
public slots:
    void OnPict();
    void OnAvi();
    void OnGpx();
    void OnSelOut();
    void OnHelp();

private slots:

    void on_buttonBox_accepted();

private:
    int loadBmMap(QPainter *paMap, QRect *qRect, QProgressDialog *pProg=nullptr,CGeoRect *area=nullptr, bool cacheMap=false);
    int loadMap(QPainter *paMap,QRect *qRect,QProgressDialog *pProg=nullptr,CGeoRect *area=nullptr,bool cacheMap=false);
    void addGpx2PtList(QList<QPoint> &route,CGPXInfo *pGi, double subX,double subY,int *skal,int zoom,char **pkte);
    // aktuelle Länge des AVI Files
    qint64 m_aviLen;
    Ui::CMakePix *ui;
    int m_onErrTile;
};

#endif // CMAKEPIX_H
