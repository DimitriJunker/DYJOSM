#ifndef NEWAIRPDIALOG_H
#define NEWAIRPDIALOG_H

#include <QDialog>
#include "cgeopoint.h"

namespace Ui {
class newAirpDialog;
}

class newAirpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit newAirpDialog(QWidget *parent = 0);
    ~newAirpDialog();
    void setName(QString name);
    CGeoPoint getCoord();
protected:
    void changeEvent(QEvent *e);

private:
    Ui::newAirpDialog *ui;
};

#endif // NEWAIRPDIALOG_H
