#include "cdyjtopt.h"
#include "ui_cdyjtopt.h"
//#include <QDEBUG>

CDyjtOpt::CDyjtOpt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDyjtOpt)
{
    ui->setupUi(this);
    ui->le_gb->setText("2");
}

CDyjtOpt::~CDyjtOpt()
{
    delete ui;
}
void CDyjtOpt::OnUpdSrc()
{
    COsmOpt::OnUpdSrc();
}

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

void CDyjtOpt::setAviMax(double gb)
{
    ui->le_gb->setText(QString::number(gb));
}



double CDyjtOpt::getAviMax()
{
    return ui->le_gb->text().toDouble();
}

