#include "flugdialog.h"
#include "ui_flugdialog.h"

flugDialog::flugDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::flugDialog)
{
    ui->setupUi(this);
    exportGpx=false;
}

flugDialog::~flugDialog()
{
    delete ui;
}
void flugDialog::setStart(QDateTime start)
{
    ui->dte_Startzeit->setDateTime(start);
}

QDateTime flugDialog::getStart()
{
    return ui->dte_Startzeit->dateTime();
}

QString flugDialog::getStrecke()
{
    QString strecke;
    if(ui->le_startLat->text().size()>4)
        strecke=ui->le_startLat->text()+","+ui->le_startLon->text();
    else
        strecke=ui->le_startLat->text()+" > ";
    if(ui->le_destLat->text().size()>4)
        strecke+=ui->le_destLat->text()+","+ui->le_destLon->text();
    else
        strecke+=ui->le_destLat->text();
return strecke;
}
void flugDialog::doGpx()
{
    exportGpx=true;
    accept();
}
void flugDialog::changeEvent(QEvent *e)
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
