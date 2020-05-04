#ifndef CMAKEHTML_H
#define CMAKEHTML_H

#include <QDialog>
#include "cgpxinfo.h"
#include <qlistwidget.h>
#include <windows.h>

enum {OL_TEMPL,OL_LOKAL,OL_URL};
typedef struct
{
public:
    QString m_descr;
    QString m_title;
    int m_zoom;
    QString m_out;
    QString m_templ;
    double m_lat;
    double m_lon;
    int m_oplSrc;
    QString m_oplPath;
}SHtmlOpt;
namespace Ui {
class CMakeHtml;
}

class CMakeHtml : public QDialog
{
    Q_OBJECT

public:
    explicit CMakeHtml(QWidget *parent = 0);
    ~CMakeHtml();
    SHtmlOpt *m_pOpt;
    QListWidget* m_pFB;
    QString m_prgPath;
    int m_anzFiles;
    void initDialog();
    int m_oplSrc;
    QString toHtml(QString quell);
protected:
    void changeEvent(QEvent *e);

private:
    Ui::CMakeHtml *ui;
public slots:
    void onMakeHtml();
    void onHtmlHilfe();
    void onHtmlTempl();
    void onHtmlOut();
    void onMkTempl();
    void onOL_L();
    void onOL_U();
    void onOL_T();
private slots:
    void on_buttonBox_accepted();
};

#endif // CMAKEHTML_H
