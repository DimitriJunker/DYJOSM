#ifndef CGPXPROPER_H
#define CGPXPROPER_H

#include <QDialog>
#include "cgpxinfo.h"

namespace Ui {
class CGpxProper;
}

class CGpxProper : public QDialog
{
    Q_OBJECT

public:
    explicit CGpxProper(QWidget *parent = 0);
    ~CGpxProper();
    void init(CGPXInfo *gpxi, QColor colDefN, QColor colDefF);
    int getVal(CGPXInfo *gpxi);
    static QString bgcString(QColor col);

private:
    int retBut;
    QColor m_colDef;
    QColor m_color;

    Ui::CGpxProper *ui;

protected:
    void changeEvent(QEvent *e);

public slots:
    void OnCol();
    void OnBck();
    void OnNxt();
    void OnDefColChg();
};

#endif // CGPXPROPER_H
