// Logfile.cpp: Implementierung der Klasse CLogfile.
//
//////////////////////////////////////////////////////////////////////
/*
 * 13.07.15 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.01b'
 * Logfileausgabe überarbeitet
*/

#include "Logfile.h"
#include <QApplication>
#include <QFileInfo>
#include <cpath.h>
#include <QTextStream>
#include <cxmlfile.h>
#include <QStandardPaths>


//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
QString CLogfile::m_path="";
CLogfile::CLogfile()
{
    if(!m_path.isEmpty())
		return;
    QString exePath=QApplication::applicationFilePath();
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    if(!paths.size())
        return;
    CPath path,pathE(exePath);
    path.setComp(paths[0],PATH_PATH);
    QString logname=pathE.makePath(PATH_FN)+"_log";
    path.setComp(logname,PATH_FN);
    path.setComp("html",PATH_EXT);
    m_path=path.makePath();

    QFile log(m_path);
 //Altes Log löschen, incl. Temporäre Files
    CXmlFile xLog;
    if(xLog.read2Buf(m_path))
    {
        QString path;
        int aktPos=0;
        while(xLog.readValB(path,"tmpfile",&aktPos))
        {
            QFile fi(path);
            fi.remove();
        }
    }

    if(log.open( QIODevice::WriteOnly)==0)
        return;

    QTextStream out(&log);
    out<<"<html>"<<"\r\n";
    out<<"<head>"<<"\r\n";
    out<< "<title>"<< (QObject::tr("Logfile für: ")) << exePath << "</title>"<<"\r\n";
    out<<"</head>"<<"\r\n";
    out<<"<body>"<<"\r\n";
    log.close();
}

CLogfile::~CLogfile()
{

}

void CLogfile::write(QString file,int line,QString str)
{
    QFile log(m_path);
    if(log.open( QIODevice::Append)==0)
        return;
    QTextStream out(&log);
    out << file << ": " << line << " " << str;
    log.close();

}
void CLogfile::closeHtml()
{
    QFile log(m_path);
    if(log.open( QIODevice::Append)==0)
        return;
    QTextStream out(&log);
    out << "</body>\r\n</html>\r\n";
    log.close();
}
