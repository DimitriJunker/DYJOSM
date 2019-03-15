#include "newairpdialog.h"
#include "ui_newairpdialog.h"

newAirpDialog::newAirpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::newAirpDialog)
{
    ui->setupUi(this);
    ui->le_airpLat->setText("0");
    ui->le_airpLon->setText("0");
}

newAirpDialog::~newAirpDialog()
{
    delete ui;
}
void newAirpDialog::setName(QString name)
{
    ui->airpCode->setText(name);
}
CGeoPoint newAirpDialog::getCoord()
{
    CGeoPoint ret;
    ret.m_lat=ui->le_airpLat->text().toDouble();
    ret.m_lon=ui->le_airpLon->text().toDouble();
    return ret;
}
void newAirpDialog::changeEvent(QEvent *e)
{
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
