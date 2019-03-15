#ifndef FLUGDIALOG_H
#define FLUGDIALOG_H

#include <QDialog>
#include <qdatetime.h>

namespace Ui {
class flugDialog;
}

class flugDialog : public QDialog
{
    Q_OBJECT

public:
    explicit flugDialog(QWidget *parent = 0);
    void setStart(QDateTime start);
    QDateTime getStart();
    QString getStrecke();
    bool exportGpx;
    ~flugDialog();
protected:
    void changeEvent(QEvent *e);
public slots:
    void doGpx();
private:
    Ui::flugDialog *ui;
};

#endif // FLUGDIALOG_H
