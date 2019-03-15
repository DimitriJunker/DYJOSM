#include "CDyjTCopy.h"
#include "ui_CDyjTCopy.h"
#include <QDesktopServices>
#include <QUrl>
/*
* 7.8.16 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ DYJTrack 2.04a'
    Copyright Dialog hinzugefüggt
*/
CDyjTCopy::CDyjTCopy(QString &vers,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDyjTCopy)
{
    ui->setupUi(this);
    ui->la_vers->setText(vers);
}

CDyjTCopy::~CDyjTCopy()
{
    delete ui;
}

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CDyjTCopy


/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CDyjTCopy

void CDyjTCopy::OnGpl()
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

void CDyjTCopy::OnDyj()
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

void CDyjTCopy::OnOSMc()
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

void CDyjTCopy::changeEvent(QEvent *e)
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
