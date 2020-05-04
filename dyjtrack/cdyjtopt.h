#ifndef CDYJTOPT_H
#define CDYJTOPT_H

#include <QDialog>
#include "cosmopt.h"
#include <QTranslator>

namespace Ui {
class CDyjtOpt;
}

class CDyjtOpt : public QDialog, public COsmOpt
{
    Q_OBJECT

private:
    Ui::CDyjtOpt *ui;
public:
    explicit CDyjtOpt(QWidget *parent = 0);
    ~CDyjtOpt();
    QTranslator *m_pLanguage;
    void setAviMax(double gb);
    void setRelPath(bool isRel);	//Absolute Pfade
    bool getRelPath();
    double getAviMax();


protected:
    void changeEvent(QEvent *e);

public slots:
    void OnUpdSrc();
    void change_lang(QString lang);
};

#endif // CDYJTOPT_H
