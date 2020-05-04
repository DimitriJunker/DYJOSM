#include "cmakehtml.h" //1
#include "ui_cmakehtml.h"
#include <qdesktopservices.h>
#include <QFileDialog>
#include <QMessageBox>
#include <urldownload.h>
#include <QTextStream>

/*
 * 19.10.15 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ DYJTrack 2.02b'
 * Endlich das leidige openlayers Thema behoben, und zwar durch einfügen eines /2.11 in map.html
 */
CMakeHtml::CMakeHtml(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CMakeHtml)
{
    ui->setupUi(this);
 }

CMakeHtml::~CMakeHtml()
{
    delete ui;
}
void CMakeHtml::initDialog()
{
    ui->le_titel->setText(m_pOpt->m_title);
    ui->le_descr->setText(m_pOpt->m_descr);
    ui->le_lat->setText(QString::number(m_pOpt->m_lat,'g',10));
    ui->le_lon->setText(QString::number(m_pOpt->m_lon,'g',10));
    ui->l_outDir->setText(m_pOpt->m_out);
    ui->l_temp->setText(m_pOpt->m_templ);


    for(int z=0;z<17;z++)
        ui->cb_zoom->addItem(QString::number(z));
    ui->cb_zoom->setCurrentIndex(m_pOpt->m_zoom);


    switch(m_pOpt->m_oplSrc)
    {
        case OL_LOKAL:
            ui->rb_lokal->setChecked(true);
            ui->le_openL->setDisabled(true);
            break;
        case OL_TEMPL:
            ui->rb_Templ->setChecked(true);
            ui->le_openL->setDisabled(true);
            break;
        case OL_URL:
            ui->rb_path->setChecked(true);
            ui->le_openL->setDisabled(false);
            break;
    }
    m_oplSrc=m_pOpt->m_oplSrc;
    //:put here the abreviation of the new Language (for ex. en for english, fr for frensh, es for spanish,...
    QString lang=tr("de");
    ui->rb_de->setChecked(lang=="de");
    ui->rb_en->setChecked(lang=="en");
    ui->rb_fr->setChecked(lang=="fr");
    ui->le_openL->setText(m_pOpt->m_oplPath);


}
Q_DECLARE_METATYPE(CGPXInfo*)


void CMakeHtml::onHtmlHilfe()
{
        QString url="file:./Docu/"+tr("liesmich.pdf");
    QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

}
void CMakeHtml::onHtmlOut()
{

    QString path= QFileDialog::getExistingDirectory(this,tr("Ausgabe Verzeichnis"),ui->l_outDir->text());
    if(!path.isEmpty())
        ui->l_outDir->setText(path);

}

void CMakeHtml::onMkTempl()
{
    QString neuTmpl;

    QMessageBox::information(nullptr, tr("Template erstellen"), tr("Bis einschl. Tracks ausfüllen. Als Tracks z.B. tst.gpx eintragen. dann mit 'Karte herunterladen' File speichern."));
    if(ui->rb_de->isChecked())//Deutsch
        QDesktopServices::openUrl(QUrl("http://www.osmtools.de/easymap/index.php?lang=de&page=editor",QUrl::TolerantMode));
    else if(ui->rb_en->isChecked()) //Englisch
        QDesktopServices::openUrl(QUrl("http://www.osmtools.de/easymap/index.php?lang=en&page=editor",QUrl::TolerantMode));
    else    //Französisch
        QDesktopServices::openUrl(QUrl("http://www.osmtools.de/easymap/index.php?lang=fr&page=editor",QUrl::TolerantMode));
}

void CMakeHtml::onHtmlTempl()
{
    QString path_neu= QFileDialog::getOpenFileName(this,tr("Lade Template"),ui->l_temp->text(),tr("HTML-Files(*.html);;Alle(*.*)"));
    if(!path_neu.isEmpty())
        ui->l_temp->setText(path_neu);

}

void CMakeHtml::onMakeHtml()
{

//    QString outDir=ui->l_outDir->text();
    QDir diOut(ui->l_outDir->text());
    QStringList files=diOut.entryList();
    if(files.size()>2)	// prüfen ob das Dir Files enthält (außer . und ..)
    {
        QMessageBox::StandardButton overw;
        overw = QMessageBox::question(nullptr, tr("Dir nicht leer"), tr("Sollen die Files ggf überschrieben werden?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if(overw==QMessageBox::No)
            return;
    }

    QString html,neu;
//    QString utilJsPath=outDir+"util.js";
    urlDownload::downloadFile("http://www.osmtools.de/easymap/temp/util.js", diOut,"util.js");
//    QString mapCssPath=outDir+"map.css";
    urlDownload::downloadFile("http://www.osmtools.de/easymap/temp/map.css", diOut,"map.css");
    QFile templFile(ui->l_temp->text());
    if(!templFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, tr("Template-Fehler"), tr("Template-File nicht vorhanden"),
                                      QMessageBox::Ok);
        return;
    }

    {
        QTextStream in(&templFile);
        html = in.readAll();	 //ganzes File laden
        templFile.close();
    }
    int pos1,pos2;

    if(ui->rb_fr->isChecked())	//Französisch
    {
        pos1=html.indexOf("lang=");
        pos2=html.indexOf("lang=",pos1+1);
        if(pos1>0)
        {
            html.remove(pos1+6,2);
            html.insert(pos1+6,"fr");
            if(pos2>0)
            {
                html.remove(pos2+6,2);
                html.insert(pos2+6,"fr");
            }
        }
        pos1=html.indexOf("<title>");
        pos2=html.indexOf("</title>",pos1);
        if(pos1>0 && pos2>0)
        {
            html.remove(pos1,pos2-pos1);
            html.insert(pos1,"<title>carte");
        }
        pos1=html.indexOf("content-language");
        pos2=html.indexOf("=\"",pos1);
        if(pos1>0 && pos2>0)
        {
            html.remove(pos2+2,2);
            html.insert(pos2+2,"fr");
        }
        pos1=html.indexOf("text = new Array(");
        pos2=html.indexOf(")",pos1);
        if(pos1>0 && pos2>0)
        {
            html.remove(pos1+18,pos2-pos1-18);
            html.insert(pos1+18,"Montrez les information sur la carte\",\"Cachez les information sur la carte\"");
        }
        pos1=html.indexOf("<div id=\"descriptionToggle\"");
        if(pos1>0)
        {
            pos1=html.indexOf(">",pos1);
            pos2=html.indexOf("</div>",pos1);
            if(pos1>0 && pos2>0)
            {
                html.remove(pos1+1,pos2-pos1-1);
                html.insert(pos1+1,"Montrez les information sur la carte");
            }
        }
    }

    if(!ui->rb_Templ->isChecked())
    {
        QString oplPath;
        if(ui->rb_lokal->isChecked())
            oplPath="openlayers/OpenLayers.js";
        else
            oplPath=ui->l_temp->text();
        pos1=0;
        while((pos1=html.indexOf("src=\"",pos1))!=0)
        {
            pos1+=5;
            pos2=html.indexOf('\"',pos1);
            if(pos2>-1)
            {
                QString srcPath=html.mid(pos1,pos2-pos1);
                if(!srcPath.right(13).compare("openlayers.js",Qt::CaseInsensitive))
                {
                    html.remove(pos1,pos2-pos1);
                    html.insert(pos1,oplPath);
                    break;
                }
            }
        }

    }



    pos1=html.indexOf("lon =");
    pos2=html.indexOf(";",pos1);
    if(pos1<0 || pos2<0)
    {
        QMessageBox::warning(nullptr, tr("Template-Fehler"), tr("Template-File ungültig: lon = "),
                                      QMessageBox::Ok);
        return;
    }
    neu.sprintf("lon = %f",ui->le_lon->text().toDouble());
    html.remove(pos1,pos2-pos1);
    html.insert(pos1,neu);

    pos1=html.indexOf("lat =");
    pos2=html.indexOf(";",pos1);
    if(pos1<0 || pos2<0)
    {
        QMessageBox::warning(nullptr, tr("Template-Fehler"), tr("Template-File ungültig:lat = "),
                                      QMessageBox::Ok);
        return;
    }
    neu.sprintf("lat = %f",ui->le_lat->text().toDouble());
    html.remove(pos1,pos2-pos1);
    html.insert(pos1,neu);

    pos1=html.indexOf("zoom =");
    pos2=html.indexOf(";",pos1);
    if(pos1<0 || pos2<0)
    {
        QMessageBox::warning(nullptr, tr("Template-Fehler"), tr("Template-File ungültig:Zoom"),
                                      QMessageBox::Ok);
        return;
    }
    neu.sprintf("zoom = %d",ui->cb_zoom->currentIndex());
    html.remove(pos1,pos2-pos1);
    html.insert(pos1,neu);

    pos1=html.indexOf("<title>");
    pos2=html.indexOf("</title>",pos1);
    if(pos1<0 || pos2<0)
    {
        QMessageBox::warning(nullptr, tr("Template-Fehler"), tr("Template-File ungültig:Titel"),
                                      QMessageBox::Ok);
        return;
    }
    neu="<title>"+ toHtml(ui->le_titel->text());
    html.remove(pos1,pos2-pos1);
    html.insert(pos1,neu);

    pos1=html.indexOf("<div id=\"description\"");
    pos1=html.indexOf(">",pos1)+1;
    pos2=html.indexOf("</div>",pos1);
    if(pos1<0 || pos2<0)
    {
        QMessageBox::warning(nullptr, tr("Template-Fehler"), tr("Template-File ungültig:<div id="),
                                      QMessageBox::Ok);
        return;
    }
    html.remove(pos1,pos2-pos1);

   html.insert(pos1,  toHtml(ui->le_descr->text()));

    pos1=html.indexOf("new OpenLayers.Layer.GML(");
    pos1-=12;
    while(html.mid(pos1).indexOf("map.addLayer(") && pos1)
        pos1--;
    pos1=html.left(pos1).lastIndexOf('\n');
    pos2=html.indexOf(");",pos1)+2;
    if(pos1<0 || pos2<0)
    {
        QMessageBox::warning(nullptr, tr("Template-Fehler"), tr("Template-File ungültig:new OpenLayers"),
                                      QMessageBox::Ok);
        return;
    }
    QString fstr=html.mid(pos1,pos2-pos1);
    html.remove(pos1,pos2-pos1);
    int posI=pos1;

    // falls mehrere Tracks im Template waren die anderen löschen
    while(html.indexOf("new OpenLayers.Layer.GML(")>-1)
    {
        pos1=html.indexOf("map.addLayer(",posI);
        if(pos1<0)
            break;
        pos1=html.left(pos1).lastIndexOf('\n');
        pos2=html.indexOf(");",pos1)+2;
        if(pos1<0 || pos2<0)
            break;
        html.remove(pos1,pos2-pos1);
    }

    pos1=fstr.indexOf("OpenLayers.Layer.GML(");	//	new OpenLayers.Layer.GML("Track 1","g2m1",
    pos1=fstr.indexOf("\"",pos1);
    pos2=fstr.indexOf("\"",pos1+1);
    if(pos1<0 || pos2<0)
    {
        QMessageBox::warning(nullptr, tr("Template-Fehler"), tr("Template-File ungültig:OpenLayers.Layer.GML 1"),
                                      QMessageBox::Ok);
        return;
    }
    fstr.remove(pos1,pos2-pos1);
    fstr.insert(pos1,"\"%s");
    pos1=fstr.indexOf("\"",pos1+4);
    pos2=fstr.indexOf("\"",pos1+1);
    if(pos1<0 || pos2<0)
    {
        QMessageBox::warning(nullptr, tr("Template-Fehler"), tr("Template-File ungültig:OpenLayers.Layer.GML 2"),
                                      QMessageBox::Ok);
        return;
    }
    fstr.remove(pos1,pos2-pos1);
    fstr.insert(pos1,"\"%s");

    pos1=fstr.indexOf("strokeColor");
    pos1=fstr.indexOf("\"",pos1);
    pos2=fstr.indexOf("\"",pos1+1);
    if(pos1<0 || pos2<0)
    {
        QMessageBox::warning(nullptr, tr("Template-Fehler"), tr("Template-File ungültig:strokeColor"),
                                      QMessageBox::Ok);
        return;
    }
    fstr.remove(pos1,pos2-pos1);
    fstr.insert(pos1,"\"#%06X");
    QDir dir;
    dir.mkpath(diOut.path());

    for(int i=m_anzFiles-1;i>-1;i--)
    {
        QString ziel="";
        QListWidgetItem *item=m_pFB->item(i);
        CGPXInfo *pGi=item->data(Qt::UserRole).value<CGPXInfo *>();
        bool isFlug=false;

        QString sfName="";
        if(!pGi->m_pfad.isEmpty())
        {
            QFileInfo fi(pGi->m_pfad);
            sfName=fi.fileName();
            if(!sfName.isEmpty())
            {
                QFileInfo zi(diOut,sfName);
                ziel=zi.filePath();
                if(zi.exists())
                    QFile::remove(ziel);
                QFile::copy(pGi->m_pfad,ziel);
            }
        }
        else if(!pGi->m_route.isEmpty())
        {
            isFlug=true;
            sfName=pGi->m_route;
            sfName.replace("(","");
            sfName.replace(")","");
            sfName.replace(" ","");
            sfName.replace(".","_");
            sfName.replace(",","_");
            sfName.replace(">","_");
            sfName+=".gpx";
            QFileInfo zi(diOut,sfName);
            ziel=zi.filePath();
            pGi->writeGPX(ziel);
        }
        if(!ziel.isEmpty())
        {
            unsigned int color=pGi->m_col.get_col(isFlug,pGi->m_col.m_useTrColSp).rgb()&0xffffff;

           if (pGi->m_name.isEmpty())
               neu.sprintf(fstr.toStdString().c_str(), toHtml(sfName).toStdString().c_str(),  sfName.toStdString().c_str(), color);
            else
               neu.sprintf(fstr.toStdString().c_str(), toHtml(pGi->m_name).toStdString().c_str(),  sfName.toStdString().c_str(), color);
        }
        html.insert(posI,neu);
    }
    QFileInfo outFileI(diOut,"map.html");
    QFile outFile(outFileI.filePath());

    if(!outFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(nullptr, tr("Fehler"), tr("Kann Ausgabefile nicht erzeugen"),
                                      QMessageBox::Ok);
        return;
    }
    QTextStream out(&outFile);
    out << html;

    outFile.close();
    QFileInfo fiU(diOut,"util.js");
    if(!fiU.exists())
    {
        QFileInfo fiUQ(m_prgPath,"util.js");
        QFile::copy(fiUQ.fileName(),fiU.fileName());

    }
    QFileInfo fiM(diOut,"map.css");
    if(!fiM.exists())
    {
        QFileInfo fiMQ(m_prgPath,"map.css");
        QFile::copy(fiMQ.fileName(),fiM.fileName());

    }
    QString url="file:///"+outFileI.filePath();
    QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

}

void CMakeHtml::on_buttonBox_accepted()
{
//    UpdateData(TRUE);
    m_pOpt->m_descr=ui->le_descr->text();
    m_pOpt->m_title=ui->le_titel->text();
    m_pOpt->m_zoom=ui->cb_zoom->currentIndex();
    m_pOpt->m_out=ui->l_outDir->text();
    m_pOpt->m_templ=ui->l_temp->text();
    m_pOpt->m_lat=ui->le_lat->text().toDouble();
    m_pOpt->m_lon=ui->le_lon->text().toDouble();
    m_pOpt->m_oplSrc=m_oplSrc;
    m_pOpt->m_oplPath=ui->le_openL->text();

}
void CMakeHtml::onOL_L()
{
    m_oplSrc=OL_LOKAL;
    ui->le_openL->setEnabled(false);
}

void CMakeHtml::onOL_U()
{
    m_oplSrc=OL_URL;
    ui->le_openL->setEnabled(true);
}
void CMakeHtml::onOL_T()
{
    m_oplSrc=OL_TEMPL;
    ui->le_openL->setEnabled(false);
}
QString CMakeHtml::toHtml(QString quell)
{
    QString htmlString="";
    int len=quell.size();
    for(int i=0;i<len;i++)
    {
        if(quell.at(i)<128)
            htmlString+=quell.at(i);
        else
        {
            QString htmlZ;
            htmlZ.sprintf("&#%d;",quell.at(i).unicode());
            htmlString+=htmlZ;

        }
    }
    return htmlString;
}
void CMakeHtml::changeEvent(QEvent *e)
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
