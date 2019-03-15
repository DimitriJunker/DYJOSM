#ifndef CDYJTCOPY_H
#define CDYJTCOPY_H

#include <QDialog>

namespace Ui {
class CDyjTCopy;
}

class CDyjTCopy : public QDialog
{
    Q_OBJECT

public:
    explicit CDyjTCopy(QString &vers, QWidget *parent = 0);
//    CDyjTCopy(QString &vers,QWidget *parent = 0);   // Standardkonstruktor

    ~CDyjTCopy();
public slots:
    void OnGpl();
    void OnDyj();
    void OnOSMc();	//DYJ Taho 4.04c
protected:
    void changeEvent(QEvent *e);


private:
    Ui::CDyjTCopy *ui;
};

#endif // CDYJTCOPY_H
