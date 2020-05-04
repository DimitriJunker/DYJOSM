#ifndef DYTMAINW_H
#define DYTMAINW_H

#include <QMainWindow>
#include <QDateTime>
#include "cgeorect.h"
#include "cgpxinfo.h"
#include "cmakehtml.h"
#include "cmakepix.h"
#include "cdyjtopt.h"
#include <qtimezone.h>
#include <qcolor.h>
#include "cpath.h"
#include <QTranslator>
#include <trackcol.h>
#include <Logfile.h>

namespace Ui {
class dytMainW;
}
enum{DYT_OPT=1,DYT_FILES=2,DYT_ZOOM=4,DYT_MAIN=8,DYT_AREAS=12,DYT_MAPS=16,DYT_ALL=255};

class dytMainW : public QMainWindow
{
    Q_OBJECT

public:
    explicit dytMainW(QWidget *parent = nullptr);
    ~dytMainW();
    int m_tZone;    // diff zu UTC in min
    int m_anzFiles;
    CGeoRect m_area;
    unsigned int m_frames[2];
    bool m_autoAreas[2];
    void updateArea(bool calcArea=true);
    double	m_lat[4];
    double	m_lon[4];
    CDyjtOpt m_opt;
    SHtmlOpt m_htmlOpt;
    SPixOpt m_aviOpt;
    SPixOpt m_pictOpt;
    void deleteGpx(int nr);
    QColor colref2Qc(int cr);
    int qc2Colref(QColor qc);
    bool loadDyt(const QString &path, int what=DYT_ALL);
    void saveDyt(QString path,QString *expDir=nullptr);
    void writeDytPathLine(QTextStream &oDyt,QString format, QString qPfad, QString dytPfad, QString *expDir=nullptr);
    void loadProject(bool add);
    QTranslator m_language;
    void CpyArea2Form();
    QString m_progName;

    CLogfile m_log;

    void closeEvent(QCloseEvent *event);
signals:
protected:
    QString getTextGpx(CGPXInfo *pGi);
    QColor m_colDef_N,m_colDef_F;
    QString bgcString(QColor col);
    void addFlight(bool exportGpx,QString strecke,QDateTime start,QString name="",TrackCol *col=nullptr,bool rev=false);
    CGeoPoint airp(QString name_c);
    QString m_airpPath[2];
    void addGK(CGPXInfo *pGi, CGeoPoint start, CGeoPoint end);
    void updateList();
    void enableArea();
    CPath m_projectPath;
    void changeEvent(QEvent *e);


public slots:
    void OnAddgpx();
    void OnSelchangeFiles();
    void onDel();
    void OnDelAll();
    void OnUp();
    void OnDown();
    void OnSortByDate();
    void OnTZChanged();
    void OnColN();
    void OnColF();
    void OnFly();
    void OnChgGpx();
    void OnSelChgArea();
    void OnManChgArea();
    void OnChgRand();
    void OnChgAutoArea();
    void OnLoadDyb();
    void OnSaveDyb();
    void OnBbox();
    void on_actionNeues_Projekt_triggered();
    void on_actionProjekt_laden_triggered();
    void on_actionProjekt_hinzu_laden_triggered();
    void on_actionProjekt_speichern_triggered();
    void on_actionProjekt_speichern_unter_triggered();
    void on_actionProjekt_exportieren_triggered();
    void on_actionBeenden_triggered();
    void on_actionHTML_erstellen_triggered();
    void on_actionOptionen_triggered();
    void on_actionHilfe_triggered();
    void on_actionInfo_triggered();



    private slots:
    void on_actionBild_erstellen_triggered();

    void on_actionFilm_erzeugen_triggered();

private:
    Ui::dytMainW *ui;
    void addGpx(QString pfad,const QString name="",TrackCol *pCol=nullptr,QDateTime odt=QDateTime(),bool rev=false);
    void addGpxI(CGPXInfo *pGi,bool sort=true);

};

#endif // DYTMAINW_H
