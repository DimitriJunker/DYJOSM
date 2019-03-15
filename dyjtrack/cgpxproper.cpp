#include "cgpxproper.h"
#include "ui_cgpxproper.h"
#include <QColorDialog>
/*
 * 15.5.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ DYJTrack 2.06c'
 * Man kann jetzt bei den Eigenschaften eines gpx Tracks bei der Farbe zwischen normal, Flug und beliebig wählen
 */
CGpxProper::CGpxProper(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CGpxProper)
{
    ui->setupUi(this);
    retBut=0;
}

CGpxProper::~CGpxProper()
{
    delete ui;
}
void CGpxProper::init(CGPXInfo *gpxi,QColor colDefN,QColor colDefF)
{
    ui->le_Name->setText(gpxi->m_name);
    ui->cb_Rev->setChecked(gpxi->m_rev);
    if(gpxi->m_pfad.isEmpty())
    {									//Flug
        ui->l_GpxPath->setText(gpxi->m_route);
        m_colDef=colDefF;
        /*DYJ DYJTrack 2.06c Start*/
        if(gpxi->m_col.m_useTrColSp==TC_AUTO)
            gpxi->m_col.m_useTrColSp=TC_FLUG;
        m_color=gpxi->m_col.get_col(true,gpxi->m_col.m_useTrColSp);
        /*DYJ  Ende; alt:
        m_color=gpxi->m_col.get_col(true);
        */
    }
    else
    {									//normal
        ui->l_GpxPath->setText(gpxi->m_pfad);
        m_colDef=colDefN;
        /*DYJ DYJTrack 2.06c Start*/
        if(gpxi->m_col.m_useTrColSp==TC_AUTO)
            gpxi->m_col.m_useTrColSp=TC_STD;
        m_color=gpxi->m_col.get_col(false,gpxi->m_col.m_useTrColSp);
        /*DYJ  Ende; alt:
        m_color=gpxi->m_col.get_col(false);
        */
    }
    /*DYJ DYJTrack 2.06c Start*/
    ui->rb_StdCol->setChecked(gpxi->m_col.m_useTrColSp==TC_STD);
    ui->rb_FlugCol->setChecked(gpxi->m_col.m_useTrColSp==TC_FLUG);
    ui->rb_col->setChecked(gpxi->m_col.m_useTrColSp==TC_RGB);
    /*DYJ  Ende; alt:
    ui->cb_DefCol->setChecked(gpxi->m_col.m_useDef);
    */
    ui->pb_Col->setStyleSheet(bgcString(m_color));
    ui->dte_Start->setDateTime(gpxi->m_dateTime);
    OnDefColChg();

}
QString CGpxProper::bgcString(QColor col)
{
    QString ret=QString("background-color: rgb(%1,%2,%3)").arg(col.red()).arg(col.green()).arg(col.blue());
    return ret;
}
int CGpxProper::getVal(CGPXInfo *gpxi)
{
    gpxi->m_name=ui->le_Name->text();
    gpxi->m_rev=ui->cb_Rev->isChecked();
    /*DYJ DYJTrack 2.06c Start*/
    gpxi->m_col.setCol(ui->rb_StdCol->isChecked(),ui->rb_FlugCol->isChecked(),m_color);
    /*DYJ  Ende; alt:
    gpxi->m_col.setCol(ui->cb_DefCol->isChecked(),m_color);
    */

    gpxi->m_dateTime=ui->dte_Start->dateTime();
    return retBut;
}
void CGpxProper::OnCol()
{
    QColor color = QColorDialog::getColor(m_color, this);
    if(color.isValid())
    {
        m_color=color;
        ui->pb_Col->setStyleSheet(bgcString(m_color));
    }

}

void CGpxProper::OnBck()
{
    retBut=-1;
    accepted();
}
void CGpxProper::OnNxt()
{
    retBut=1;
    accepted();
}
void CGpxProper::changeEvent(QEvent *e)
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
void CGpxProper::OnDefColChg()
{
    //DYJ DYJTrack 2.06c Start
    if(!ui->rb_col->isChecked())
    /*DYJ  Ende; alt:
    if(ui->cb_DefCol->isChecked())
    */
    {
        ui->pb_Col->setStyleSheet(bgcString(m_colDef));
        ui->pb_Col->setDisabled(true);
    }
    else
    {
        ui->pb_Col->setStyleSheet(bgcString(m_color));
        ui->pb_Col->setDisabled(false);
    }

}

