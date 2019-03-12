#ifndef CDOKMZ_H
#define CDOKMZ_H

#include <QDialog>

namespace Ui {
class CDoKmz;
}

class CDoKmz : public QDialog
{
    Q_OBJECT

public:
    explicit CDoKmz(QWidget *parent = nullptr);
    ~CDoKmz();
    QString tmpFolder();
    QString m_tmpPath;
    QString m_zipPath;
    QString m_zipPar;
    QString	m_out;
    bool add2kmz(QString kmzPfad,QString pfad);
    CDoKmz(QString zipPath, QString zipPar, QWidget *parent = nullptr);   // Standardkonstruktor
protected:
    void changeEvent(QEvent *e);


public slots:
    void OnAddKml();
    void OnDel();
    void OnDelall();
    void OnFsKmz();
    void OnOK();
    void OnChgNumber();

private:
    Ui::CDoKmz *ui;
};

#endif // CDOKMZ_H
