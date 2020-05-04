#include "cxmlfile.h"
#include <qfile.h>
#include <qtextstream.h>
#include <qglobal.h>
#include <float.h>
#include <qdir.h>
#include "cosm.h"
#include <qprocess.h>
#include <QDebug>
#include <cosm.h>
#ifdef QT_GUI_LIB
#include <qmessagebox.h>
#endif
//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CXmlFile::CXmlFile()
{

}

CXmlFile::~CXmlFile()
{

}

/*** ------------------------------------------------------------------------ ***/
/*	bool CXmlFile::read2Buf(QString pfad, const char *codec)					*/
/*	Liest ein File in einen Buffer, dabei kann ein Codec  z.B. UTF-8 angegeben werden	*/
/*	Parameter:	pfad: Pfad des Files											*/
//              codec: Name des Codec oder nullptr
/*	Ergebnis:	FALSE falls File nicht existiert    							*/
/*	12.5.18 von Dimitri Junker	DYJ 1.03a               						*/
/*** ------------------------------------------------------------------------ ***/
bool CXmlFile::read2Buf(QString pfad)
{
    QFile gpxFile(pfad);
    if(gpxFile.open(QIODevice::ReadOnly|QIODevice::Text)==0)
        return false;
    m_fileLen=(unsigned long)gpxFile.size();
    QTextStream in(&gpxFile);
    m_sBuf = in.readLine();
    QString codec=readAttrValS(m_sBuf,"encoding");

    if(!codec.isEmpty())
        in.setCodec(codec.toLatin1());

    m_sBuf = in.readAll();



    gpxFile.close();
    return true;
}
//<table:table-cell table:number-columns-repeated="3" />
bool CXmlFile::readValB(QString &val,QString &quell,QString tag, int *pPos,QString *attr,int opt)
{
    Q_ASSERT((&val)!=(&quell));
    int startPos=0;
    if(pPos)
        startPos=*pPos;
    QString start;
    bool retB=true;
    start=QString("<%1").arg(tag);
    val="";
    int pos=-2;
    while(pos==-2)
    {
        pos=quell.indexOf(start,startPos);
        if(pos==-1)
            retB=false;
        else
        {
            int posAttr=pos+start.size();
            switch(quell.at(posAttr).toLatin1())
            {
            case '>':	//einfacher Tag <tag>
                {
                    QString end=QString("</%1>").arg(tag);
                    int posE=quell.indexOf(end,pos);
                    if(posE>-1)
                    {
                        int posVal=quell.indexOf(">",pos)+1;
                        val=quell.mid(posVal,posE-posVal);
                        if(pPos)
                            *pPos=posE+end.size();
                        if(attr)
                            *attr="";
                    }
                }
                break;
            case '/':	//Empty ohne Attribut
                if(pPos)
                    *pPos=posAttr+2;
                if(attr)
                    *attr="";
                break;
            case ' ':	// Empty oder mit Atrribut oder beides
            case '\t':
                posAttr++;
                if(quell[posAttr]=='/' && quell[posAttr+1]=='>')
                {	//Empty ohne Attribut aber mit Leerstelle
                    if(pPos)
                        *pPos=posAttr+2;
                    if(attr)
                        *attr="";
                }
                else
                {	//mit Attribut
                    int posVal=quell.indexOf(">",pos)+1;
                    if(quell[posVal-2]=='/')
                    {	//Empty mit Attribut
                        if(pPos)
                            *pPos=posVal;
                        if(attr)
                            *attr=quell.mid(posAttr,posVal-posAttr-2);
                    }
                    else
                    {	//Normal mit Attribut
                        QString end=QString("</%1>").arg(tag);
                        int posE=quell.indexOf(end,pos);
                        if(posE>-1)
                        {
                            val=quell.mid(posVal,posE-posVal);
                            if(pPos)
                                *pPos=posE+end.size();
                            if(attr)
                                *attr=quell.mid(posAttr,posVal-posAttr-1);
                        }
                    }
                }
                break;
            default:	//Zufall (der gefundene Tagname beginnt wie der gesuchte)
                startPos=quell.indexOf(">",pos)+1;
                pos=-2;			//weitersuchen
            }
        }
    }
    if(retB)
    {
        if(!val.left(1).compare("\"") && !val.right(1).compare("\""))
        {
            val.remove(0,1);
            val.remove(val.size()-1);
        }
        if((opt&ELIM_TAGS)>0)
            elimAllTags(val);
        if((opt&TRIM_LEFT)>0)
            trimLeft(val,"\t \r\n");
        if((opt&TRIM_RIGHT)>0)
            trimRight(val,"\t \r\n");

    }
    return retB;
}


int CXmlFile::readVal_intDef(QString &quell, QString tag, int def,int *start)
{
    QString val;
    if(!readValB(val,quell,tag,start,nullptr,TRIM|ELIM_TAGS))
        return def;
    else
    {
        bool ok;
        return val.toInt(&ok,0);
    }

}

bool CXmlFile::readValB(QString &val,QString tag, int *start,QString *attr,int opt)
{
    return readValB(val,m_sBuf,tag,start,attr,opt);
}


double CXmlFile::readAttrValD(QString quell, QString name)
{
    int pos=quell.indexOf(name);
    double ret=	DBL_MAX;

    if(pos>-1)
    {
        pos+=name.size();
        QRegExp rx("([-+]?[0-9]*\\.[0-9]+|[-+]?[0-9]+)");
        int posDbl=rx.indexIn(quell.mid(pos));
        if(posDbl>-1)
            ret=rx.cap(1).toDouble();
      }
    return ret;

}
QString CXmlFile::readAttrValS(QString quell, QString name)
{
    int start=quell.indexOf(name);
    QString ret;

    if(start>-1)
    {
        start=quell.indexOf('\"',start);
        if(start>-1)
        {
            start++;
            int ende=quell.indexOf('\"',start);
            if(ende>-1)
                ret=quell.mid(start,ende-start);
        }
    }
    return ret;

}

QString CXmlFile::readAttr(QString tag, int *start)
{
    return readAttr(m_sBuf,tag,start);
}

QString CXmlFile::readAttr(QString &quell, QString tag, int *pPos)
{
    int startPos=0;
    if(pPos)
        startPos=*pPos;
    QString ret="",start,endTag,endAttr=">";
    start=QString("<%1").arg(tag);
    int pos=quell.indexOf(start,startPos),posAttr=-1;
    if(pos>-1)
    {
        int posE=quell.indexOf(">",pos),posN;
        posN=posE+1;
        if(posE>-1)
        {
            if(quell[posE-1]=='/')
                posE--;
            posAttr=pos+start.size();
            ret=quell.mid(posAttr,posE-posAttr);
            if(pPos)
                *pPos=posN;
        }
    }
    return ret;

}

double CXmlFile::readVal_dblDef(QString &quell, QString tag, double def)
{
    QString val;
    if(!readValB(val,quell,tag))
        return def;
    else
        return val.toDouble();

}

void CXmlFile::elimAllTags(QString &str)
{
    int idxS,idxE;
    while((idxS=str.lastIndexOf('<'))>-1)
    {
        idxE=str.indexOf('>',idxS);
        if(idxE==-1)
            break;
        str.remove(idxS,idxE-idxS+1);
    }
}


/*** ------------------------------------------------------------------------ ***/
/*	bool CXmlFile::read2BufFromZip(QString zipFile, QString filename)	*/
/*	Liest ein File in einen Buffer, dabei kann ein Codec  z.B. UTF-8 angegeben werden	*/
/*	Parameter:	zipFile: Pfad des Zip-Files											*/
//              filename: File im ZIP
/*	Ergebnis:	FALSE falls File nicht existiert    							*/
/*	12.5.18 von Dimitri Junker	DYJ 1.03a               						*/
/*** ------------------------------------------------------------------------ ***/
bool CXmlFile::read2BufFromZip(QString zipFile, QString filename)
{
    //	param.Format("e -tzip \"%s\" \"%s\"",kmzPfad,pfad);
        QString tmpF=QDir::tempPath();
        QString param;
        QString fldr=tmpF+tmpnam(nullptr);
        if(fldr.endsWith("."))
            fldr.chop(1);
        if(!fldr.endsWith("/"))
            fldr+="/";
        QDir tmp;
        tmp.mkpath(fldr);
        QProcess gzip;
        QStringList sl;
        sl<<"e";
        sl<<zipFile;
        param=QString("-o%1").arg(fldr);
        sl<<param;
        sl<<filename;
        int zipRet=gzip.execute("C:/Program Files (x86)/7-Zip/7z.exe",sl);

        qDebug()<<zipRet;

     //   QByteArray result = gzip.readAll();

      /*
        STARTUPINFO sinfo;
        PROCESS_INFORMATION pinfo;

        ZeroMemory( &sinfo, sizeof(sinfo) );
        sinfo.cb = sizeof(sinfo);
        ZeroMemory( &pinfo, sizeof(pinfo) );
        bool bRet = CreateProcess(nullptr,param.GetBuffer(1000),nullptr,nullptr,TRUE,0,nullptr,nullptr,&sinfo,&pinfo);

        if (!bRet) {
            std::cout << "Fehler: " << GetLastError();
        }

        WaitForSingleObject(pinfo.hProcess,INFINITE);
    */
        QString pfad=QString("%1%2").arg(fldr).arg(filename);

        bool ret=read2Buf(pfad);
        QFile::remove(pfad);
        QDir dir;
        dir.rmdir(fldr);

        return ret;
}


//mode=0: nur den Text
int CXmlFile::readOdsZeile(QStringList *zellen,int *start,int mode)
{
    zellen->clear();
    int ret=0;
    QString attr,zeile;
    int aktPosZ=0;
    if(readValB(zeile,"table:table-row", start,&attr))
    {
        QString ws=readAttrValS(attr,"table:number-rows-repeated");
        if(!ws.isEmpty())
            ret=ws.toInt();
        else
            ret=1;
        QString zelle,zelltxt;
        while(readValB(zelle,zeile,"table:table-cell", &aktPosZ,&attr))
        {
            readValB(zelltxt,zelle,"text:p");
            zelltxt.replace("<text:s/>","");
            if(mode==0 && zelltxt.left(1).compare("<")==0)
            {
                QString tmp;
                if(!(readValB(tmp,zelltxt,"text:a")))
                {
                    QString text1=QObject::tr("Fehler bei Zelle\n"),text2=QObject::tr("Falscher Zellinhalt");
                    text1+=zelltxt;
#ifdef QT_GUI_LIB
                    QMessageBox msgBox;
                    msgBox.setText(text1);
                    msgBox.setWindowTitle(text2);
                    msgBox.exec();
#else
                    qDebug()<< text1 <<text2;
#endif


               }
                zelltxt=tmp;
            }

            ws=readAttrValS(attr,"table:number-columns-repeated");
            int wi=1;
            if(!ws.isEmpty())
                wi=ws.toInt();
            for(int i=0;i<wi;i++)
                zellen->append(zelltxt);
          }
    }
    return ret;

}
void CXmlFile::trimLeft(QString &quell,QString wspaces)
{
    while(wspaces.indexOf(quell[0])>-1)
        quell.remove(0,1);
}

void CXmlFile::trimRight(QString &quell,QString wspaces)
{
    while(wspaces.indexOf(quell.right(1))>-1)
        quell.chop(1);
}
