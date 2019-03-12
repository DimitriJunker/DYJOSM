// TahoOpt.cpp: Implementierungsdatei
//


//CHG: TAHO 2.10g DYJ
//Die *.taho benutzten das falsche charset
/*
* 1.12.13 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 3.08b'
Nötige Änderungen zur Anpassung an Visual Studio 2013
 * 30.12.18 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.07j'
 * Seit Taho mit einem Installer installiert wird machen die Optionen
 * zu den Pfaden keinen Sinn mehr, also habe ich sie gelöscht

*/
#include "ctahoopt.h"
#include "ui_ctahoopt.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QSettings>
#include <QThread>

QString m_osmUrl="https://dimitrijunker.lima-city.de/OSM/";

CTahoOpt::CTahoOpt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CTahoOpt)
{
    ui->setupUi(this);
    m_tasks = abs(QThread::idealThreadCount());

    m_saveButton = ui->buttonBox->addButton(tr("Speichern"), QDialogButtonBox::ApplyRole);
    m_saveButton->setIcon(QPixmap(":/images/document-save.png"));
    connect(ui->buttonBox, &QDialogButtonBox::clicked,
            this, &CTahoOpt::onButtonBox_clicked);
}

CTahoOpt::~CTahoOpt()
{
    delete ui;
}

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTahoOpt

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTahoOpt
void CTahoOpt::OnUpdSrc()
{
    COsmOpt::OnUpdSrc();
}

void CTahoOpt::change_lang(QString lang)
{
    if(lang=="Deutsch")
        qApp->removeTranslator(m_pLanguage);
    else if(lang=="English")
    {
        m_pLanguage->load("taho_en");
        qApp->installTranslator(m_pLanguage);
    }
    else if(lang=="Français")
    {
        m_pLanguage->load("taho_fr");
        qApp->installTranslator(m_pLanguage);
    }
}
/* 2do
bool CTahoOpt::OnInitDialog()
{
    CMultiLanguageDialog::OnInitDialog();

    adjustLanguage(getLang());
    UpdateData(FALSE);
    CSpinButtonCtrl *pSpin=(CSpinButtonCtrl *)GetDlgItem(IDC_TASKS);
    pSpin->SetRange(1,100);
    pSpin->SetPos(m_tasks);
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}
*/
void CTahoOpt::OnZip()
{
    QFileInfo fi(getZipPath());
    QString path_neu= QFileDialog::getOpenFileName(this,tr("Packer"),getZipPath(),tr("Programm(*.exe);;Alle(*.*)")); //2dowindows
    if(path_neu.isEmpty())
        return;
    setZipPath(path_neu);


    QFileInfo fiZip(getZipPath());
    QString prgName=fiZip.baseName().toLower();

    if(prgName.contains("7z"))
    {
        setZipPar("a -tzip $Z @$L");
        setUnGzPar("x $Q -o$Z");
    }
    else if(prgName.contains("winrar"))
    {
        setZipPar("a -afzip $Z @$L");
        setUnGzPar("x $Q $Z");
    }
    else if(prgName.contains("winzip"))
    {
        setZipPar("-min -a $Z @$L");
        setUnGzPar("-min -e $Q $Z");
    }
}

void CTahoOpt::setCache(int tage)
{
    ui->sbCache->setValue(tage);
}

int CTahoOpt::getCache()
{
    return ui->sbCache->value();
}

QString	CTahoOpt::getZipPath()
{
    return ui->le_zip->text();
}

void CTahoOpt::setZipPath(QString path)
{
    ui->le_zip->setText(QDir::toNativeSeparators(path));
}

QString	CTahoOpt::getOffDirPath()
{
    return ui->la_offDir->text();
}

void CTahoOpt::setOffDirPath(QString path)
{
    ui->la_offDir->setText(path);
}

QString	CTahoOpt::getZipPar()
{
    return ui->le_parKMZ->text();
}

void CTahoOpt::setZipPar(QString par)
{
    ui->le_parKMZ->setText(par);
}

QString	CTahoOpt::getUnGzPar()
{
    return ui->le_parGZ->text();
}

void CTahoOpt::setUnGzPar(QString par)
{
    ui->le_parGZ->setText(par);
}

void CTahoOpt::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

void CTahoOpt::on_pb_offDir_clicked()
{
    const auto path_neu= QFileDialog::getExistingDirectory(this,tr("OfflineDir"),getOffDirPath());
    if(path_neu.isEmpty())
        return;
    setOffDirPath(QDir::toNativeSeparators(path_neu));
}

void CTahoOpt::onButtonBox_clicked(QAbstractButton * btn)
{
    if (btn == m_saveButton)
    {
        saveOptions();
    }
}

void CTahoOpt::saveOptions()
{
    QSettings settings("Dimitri-Junker.de","Taho");
    settings.setValue("language",tr("de"));
    settings.setValue("zip",getZipPath());
    settings.setValue("zippar",getZipPar());
    settings.setValue("ungzpar",getUnGzPar());
    settings.setValue("OfflineDir",getOffDirPath());
    settings.setValue("maxThreads",m_tasks);
    settings.setValue("OsmUrl",m_osmUrl);
    settings.setValue("version",m_version);
 }
