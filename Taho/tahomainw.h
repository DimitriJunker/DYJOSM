#ifndef TAHOMAINW_H
#define TAHOMAINW_H

#include <QMainWindow>
#include <ctahoopt.h>
#include <cpixmap.h>
#include <QFileInfo>
#include <cosmopt.h>
#include <cpath.h>
#include <QTranslator>
#include <Logfile.h>
#include <QDebug>

namespace Ui {
class TahoMainW;
}

class TahoMainW : public QMainWindow
{
    Q_OBJECT

public:
    explicit TahoMainW(QWidget *parent = nullptr);
    ~TahoMainW();
    CPath m_tahoParam;
    QTranslator m_language;
    CLogfile m_log;
protected:
    void changeEvent(QEvent *e);


private:
    Ui::TahoMainW *ui;
    CTahoOpt m_opt;
    bool loadTaho(QString pfad);
    void saveTaho(QString &pfad);
    void makeVmap();
    void makePmap();
    int pictType();
    int nameBy();
    bool addMaps2ListP(QList<MAKEMAPSP *> &mmList,SDLM_DATA *data,int size,int zoom, CGeoRect &gr);
    bool addMaps2ListV(QList<MAKEMAPSV *> &mmList,SDLM_DATA *data,CGeoRect gr,CMapSrc *map);
    SDLM_DATA * initData(int size=SIZE_VECTOR);
    bool loadZoomL(int zl);
    void setAutoPath();

public slots:
    void OnSelchangeSizec(int size);
    void OnDoTaho();
    void OnZ0();
    void OnZ2();
    void OnZAll();
    void OnAutoOut();
    void OnGetOut();
    void OnHelpb();
    void OnSelchangeSourceP();
    void OnSelchangedMapType(int type);
    void OnBnClickedBbox();
    void OnCancel();
    void OnHelpB();
    void OnMapChanged(QListWidgetItem* item);

    private slots:
    void on_actionEinstellungen_speichern_triggered();
    void on_actionEinstellungen_laden_triggered();
    void on_actionOSMBugs_holen_triggered();
    void on_actionKMZ_erstellen_triggered();
    void on_actionOptionen_triggered();
    void on_actionInfo_triggered();
    void on_actionBeenden_triggered();
};

#endif // TAHOMAINW_H

