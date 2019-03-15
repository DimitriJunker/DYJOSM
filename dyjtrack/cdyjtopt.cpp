#include "cdyjtopt.h"
#include "ui_cdyjtopt.h"
//#include <QDEBUG>

CDyjtOpt::CDyjtOpt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDyjtOpt)
{
    ui->setupUi(this);
//DYJ DYJTrack 2.07j 	    m_pDefPrg=ui->rb_pfadPrg;
//DYJ DYJTrack 2.07j 	    m_pDefED=ui->rb_pfadED;
//DYJ DYJTrack 2.07j     ui->rb_pfadA->setChecked(true);
//DYJ DYJTrack 2.07j     ui->rb_pfadR->setChecked(false);
//DYJ DYJTrack 2.07j     ui->rb_pfadED->setChecked(true);
//DYJ DYJTrack 2.07j     ui->rb_pfadPrg->setChecked(false);
    ui->le_gb->setText("2");
}

CDyjtOpt::~CDyjtOpt()
{
    delete ui;
}
/*DYJ DYJTrack 2.07d Start*/
/*DYJ  Ende; alt:

void CDyjtOpt::OnExpSrc()
{
//    UpdateData(true);
    expSrc();
}
*/
void CDyjtOpt::OnUpdSrc()
{
    COsmOpt::OnUpdSrc();
}
/*DYJ DYJTrack 2.07j Start*/
/*DYJ  Ende; alt:
void CDyjtOpt::OnNormal()
{
    setDefPathIsPrg(false);
    setRelPath(false);
    setDefPathIsPrg(false);
//    UpdateData(false);

}

void CDyjtOpt::OnUsb()
{
    setDefPathIsPrg(true);
    setRelPath(true);
    setDefPathIsPrg(true);
//    UpdateData(false);

}
*/

void CDyjtOpt::change_lang(QString lang)
{
    if(lang=="Deutsch")
        qApp->removeTranslator(m_pLanguage);
    else if(lang=="English")
    {
        m_pLanguage->load("dyjtrack_en");
        qApp->installTranslator(m_pLanguage);
    }
    else if(lang=="Français")
    {
        m_pLanguage->load("dyjtrack_fr");
        qApp->installTranslator(m_pLanguage);
    }
}
void CDyjtOpt::changeEvent(QEvent *e)
{
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        //:name of the language
//        ui->cb_lang->setCurrentText(tr("Deutsch"));
        break;
    default:
        break;
    }
}
/*DYJ DYJTrack 2.07j Start*/
/*DYJ  Ende; alt:

void CDyjtOpt::setRelPath(bool isRel)
{
    ui->rb_pfadA->setChecked(!isRel);
    ui->rb_pfadR->setChecked(isRel);

}
bool CDyjtOpt::getRelPath()
{
    return ui->rb_pfadR->isChecked();

}*/

void CDyjtOpt::setAviMax(double gb)
{
    ui->le_gb->setText(QString::number(gb));
}



double CDyjtOpt::getAviMax()
{
    return ui->le_gb->text().toDouble();
}

