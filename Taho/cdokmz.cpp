#include "cdokmz.h"
#include "ui_cdokmz.h"
#include <QFileDialog>
#include <cxmlfile.h>
#include <QTextStream>
#include <QMessageBox>
#include <windows.h>
#include <QProcess>

CDoKmz::CDoKmz(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDoKmz)
{
    ui->setupUi(this);
}

CDoKmz::~CDoKmz()
{
    delete ui;
}

// DoKmz.cpp: Implementierungsdatei
//

#include <cdokmz.h>
#include <cpath.h>
#include <cosm.h>


 /* 06.12 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 3.06a'
Wegen DYJ Doku 1.00a Start mußten auch in Taho Änderungen vorgenommen werden
*/

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CDoKmz


CDoKmz::CDoKmz(QString zipPath,QString zipPar,QWidget *parent /*= 0*/):
    QDialog(parent),
    ui(new Ui::CDoKmz)
{
    ui->setupUi(this);
    m_out ="";
    m_zipPath=zipPath;
    m_zipPar=zipPar;
    m_tmpPath="";
    ui->rb_proKml->setChecked(true);
    ui->la_out->setDisabled(true);
    ui->pb_out->setDisabled(true);
}

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CDoKmz

void CDoKmz::OnAddKml()
{
    QStringList fileNames= QFileDialog::getOpenFileNames(this,tr("Lade kml-File(s)"),"",tr("kml-Files(*.kml)"));
    QStringList list = fileNames;
    QStringList::Iterator it = list.begin();
    while(it != list.end())
    {
        ui->lw_kml->addItem(*it);
        ++it;
    }
}

void CDoKmz::OnDel()
{
    ui->lw_kml->takeItem(ui->lw_kml->currentRow());
}

void CDoKmz::OnDelall()
{
    ui->lw_kml->clear();
}

void CDoKmz::OnFsKmz()
{
    CPath cPath;
    if(m_out.isEmpty())
    {
        if(ui->lw_kml->count()>0)
        {
            QString path=ui->lw_kml->item(0)->text();
            cPath=path;
            if(ui->rb_proZoom->isChecked())
                cPath.setFile("zl.kmz");
            else
                cPath.setFile("all.kmz");
        }
    }
    else
        cPath=m_out;
    QString path= QFileDialog::getSaveFileName(nullptr,tr("Speicher KMZ Files"),cPath.makePath(PATH_PATH),tr("KMZ-Files(*.kmz)"));
    if(path.isEmpty())
       return;

    m_out=path;

}

void CDoKmz::OnOK()
{
    int zl;
    QFile *kmlOut[19],*listF[19];
    if(ui->lw_kml->count() && m_out.isEmpty())
        OnFsKmz();
    QString kmlFileO[19],kmzFile[19],kmListFile[19],fName0=m_out;

    int idx=fName0.lastIndexOf('.');
    if(idx>-1)
        fName0.resize(idx);
    for(zl=0;zl<19;zl++)
        kmlOut[zl]=nullptr;
//    CListBox* pFB= (CListBox*)GetDlgItem(IDC_LISTKML);
    int anzFiles=ui->lw_kml->count();
    bool ok=true;
    for(int i=0;i<anzFiles && ok;i++)
    {
        QString kmlFileI,pictFile="";
        kmlFileI=ui->lw_kml->item(i)->text();
        CXmlFile xKml;
        QString sFolder;
        if(xKml.read2Buf(kmlFileI))
        {
            xKml.readValB(sFolder,"Folder");
            QString sGo;
            xKml.readValB(sGo,sFolder,"GroundOverlay");
            if(!sGo.isEmpty())
            {
                int idx=kmlFileI.lastIndexOf('/');
                if(idx>-1)
                    pictFile=kmlFileI.left(idx+1);
                else
                    pictFile="";	//sollte nicht vorkommen

                QString pictFileP;
                xKml.readValB(pictFileP,sGo,"name");
                pictFile+=pictFileP;
                zl=0;
                QString zlS="";
                if(ui->rb_proKml->isChecked())  	// 1KMZ/KML
                {
                    kmzFile[0]=kmlFileI;
                    int idx=kmzFile[0].lastIndexOf('.');
                    if(idx>-1)
                        kmzFile[0].resize(idx);
                    kmzFile[0]+=".kmz";
                    kmlFileO[0]=tmpFolder();
                    kmListFile[0]=kmlFileO[0]+"doc.list";
                    kmlFileO[0]+="doc.kml";

                    kmlOut[0]=new QFile(kmlFileO[0]);
                    listF[0]=new QFile(kmListFile[0]);
                    if(kmlOut[0]->open(QIODevice::WriteOnly) && listF[0]->open(QIODevice::WriteOnly))
                    {
                        QTextStream outTs(kmlOut[0]);
                        outTs<<xKml.m_sBuf;
                        kmlOut[0]->close();
                        QTextStream listTs(listF[0]);
                        listTs << kmlFileO[0] << "\r\n" << pictFile<< "\r\n";
                        listF[0]->close();
                        ok=add2kmz(kmzFile[0],kmListFile[0]);
                    }
                }
                else if(ui->rb_proZoom->isChecked())//	1KMZ/Zoom-Level
                {
                    int idx=pictFile.indexOf("_z");
                    if(idx>-1)
                        zl=pictFile.mid(idx+2).toInt();
                    zlS=QString("_%1").arg(zl);
                }
                else	// 1KMZ
                {
                    if(!kmlOut[zl])
                    {
                        kmlFileO[zl]=tmpFolder();
                        kmListFile[zl]=kmlFileO[zl]+"doc.list";
                        kmlFileO[zl]+="doc.kml";

                        kmzFile[zl]=QString("%1%2.kmz").arg(fName0).arg(zlS);
                        kmlOut[zl]=new QFile(kmlFileO[zl]);
                        listF[zl]=new QFile(kmListFile[zl]);

                        if(kmlOut[zl]->open(QIODevice::WriteOnly) && listF[zl]->open(QIODevice::WriteOnly))
                        if(!kmlOut[zl] || !listF[zl])
                        {
                            QMessageBox::warning(nullptr, tr("KMZ-Ausgabe"), tr("Kann Ausgabefile nicht erzeugen"),
                                                          QMessageBox::Ok);
                            while(zl>0)
                            {
                                if(kmlOut[zl])
                                    kmlOut[zl]->close();
                                if(listF[zl])
                                    listF[zl]->close();
                                zl--;
                            }
                            return;
                        }
                        QTextStream outTs(kmlOut[zl]),listTs(listF[zl]);


                        outTs << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\r\n";
                        outTs << "<kml>\r\n";
                        outTs << "\t<Document>\r\n";
                        outTs << "\t\t<name>OSM</name>\r\n";
                        outTs << "\t\t<description>Zoom-Level:" <<zl << "</description>\r\n";
                        listTs<< kmlFileO[zl] << "\r\n";
                    }
                    QTextStream outTs(kmlOut[zl]),listTs(listF[zl]);
                    outTs << "\t\t<Folder>\r\n";
                    outTs << sFolder;
                    outTs << "\t\t</Folder>\r\n";
                    listTs<< pictFile << "\r\n";
                    break;
                }
            }
        }
    }
    int anz=1;
    if(ui->rb_proZoom->isChecked())//	1KMZ/Zoom-Level
        anz=19;
    else
        anz=1;

    if(!ui->rb_proKml->isChecked())  	// 1KMZ/Zoom-Level oder 1KMZ
    {
        for(zl=0;zl<anz;zl++)
        {
            if(kmlOut[zl])
            {
                QTextStream outTs(kmlOut[zl]);
                outTs << "\t</Document>\r\n";
                outTs << "</kml>\r\n";
                kmlOut[zl]->close();
                listF[zl]->close();
                ok=add2kmz(kmzFile[zl],kmListFile[zl]);
            }
        }
    }
    accept();
}

bool CDoKmz::add2kmz(QString kmzPfad, QString listPfad)
{
    QString param=m_zipPar;
    for(int i=0;i<param.size();i++)
    {
        if(param[i]=='$')
        {
            if(param[i+1]=='$')
                param.remove(i);
            else if(param[i+1]=='Z')
            {
                param.remove(i,2);
                param.insert(i,"\"\"");
                param.insert(i+1,kmzPfad);
            }
            else if(param[i+1]=='L')
            {
                param.remove(i,2);
                param.insert(i,"\"\"");
                param.insert(i+1,listPfad);
            }
        }
    }
//	param.Format("a -tzip \"%s\" \"%s\"",kmzPfad,pfad);
    QString cmd=m_zipPath+" "+param;
    QProcess *mp=new QProcess(this);
    if(!mp->startDetached(cmd))
    {
        QMessageBox::warning(nullptr, tr("KMZ-Ausgabe"), tr("Beim Aufruf des Packers trat ein Fehler auf"),
                                      QMessageBox::Ok);
        return FALSE;
    }
    return TRUE;
}


void CDoKmz::OnChgNumber()
{
    ui->la_out->setEnabled(!ui->rb_proKml->isChecked());
    ui->pb_out->setEnabled(!ui->rb_proKml->isChecked());
}


//gibt Temporären Ordner zurück, in der Form tmp\tahotmp\1 mit tmp dem System-TMP-Ordner
QString CDoKmz::tmpFolder()
{
    static int nr=0;
    QFileInfo fi0(QDir::temp(),"tahotmp");
    QDir dir0(fi0.absoluteFilePath());
    QString ret;
    if(!nr)		//altes löschen
        dir0.removeRecursively();
    QFileInfo fiN(dir0,QString::number(nr++));
    QDir dirH;
    dirH.mkpath(fiN.absoluteFilePath());
    return ret;
}
void CDoKmz::changeEvent(QEvent *e)
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
