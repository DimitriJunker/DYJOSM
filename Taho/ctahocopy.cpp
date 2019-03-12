#include "ctahocopy.h"
#include "ui_ctahocopy.h"
#include <QDesktopServices>
#include <QUrl>

CTahoCopy::CTahoCopy(QString &vers,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CTahoCopy)
{
    ui->setupUi(this);
    ui->la_vers->setText(vers);
}

CTahoCopy::~CTahoCopy()
{
    delete ui;
}

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTahoCopy


/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTahoCopy

void CTahoCopy::OnGpl()
{
    //:put here the abreviation of the new Language (for ex. en for english, fr for frensh, es for spanish,...
    QString sprache=tr("de"),url;
    if(sprache=="de")	//deutsch
        url="http://www.gnu.de/documents/gpl.de.html";
    else if(sprache=="fr")//französisch
        url="http://www.april.org/sites/default/files/groupes/trad-gpl/doc/GPLv3/www.rodage.org/gpl-3.0.fr.html?q=groupes/trad-gpl/doc/GPLv3/www.rodage.org/gpl-3.0.fr.html";
    else if(sprache=="en") //englisch
        url="http://www.gnu.org/licenses/gpl.html";
    else
        url="http://www.gnu.org/licenses/translations.html";
    QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));


}

void CTahoCopy::OnDyj()
{
    //:put here the abreviation of the new Language (for ex. en for english, fr for frensh, es for spanish,...
    QString sprache=tr("de"),url;
    if(sprache=="de")	//deutsch
        url="http://www.dimitri-junker.de/html/openstreetmap.html";
    else if(sprache=="fr")//französisch
        url="http://www.dimitri-junker.de/frz/";
    else  //englisch
        url="http://www.dimitri-junker.de/eng/html/openstreetmap.html";
    QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));


}
void CTahoCopy::OnOsmTaho()
{
    //:put here the abreviation of the new Language (for ex. en for english, fr for frensh, es for spanish,...
    QString sprache=tr("de"),url;
    if(sprache=="de")	//deutsch
        url="http://wiki.openstreetmap.org/wiki/DE:Taho#Taho.exe";
    else if(sprache=="fr")//französisch
        url="http://wiki.openstreetmap.org/wiki/FR:Taho#Taho.exe";
    else  //englisch
        url="http://wiki.openstreetmap.org/wiki/Taho#Taho.exe";
    QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

}
void CTahoCopy::OnOSMc()
{
    //:put here the abreviation of the new Language (for ex. en for english, fr for frensh, es for spanish,...
    QString sprache=tr("de"),url;
    if(sprache=="de")	//deutsch
        url="https://www.openstreetmap.org/copyright/de";
    else if(sprache=="fr")//französisch
        url="https://www.openstreetmap.org/copyright/fr";
    else  //englisch
        url="https://www.openstreetmap.org/copyright/en";
    QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

}

void CTahoCopy::changeEvent(QEvent *e)
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
