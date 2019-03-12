#include "cpath.h"
#include <qfileinfo.h>
#include <qdir.h>

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////
/*
 * 13.07.15 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.01a'
 * Fehler bei CPath behoben
 *
 * 13.07.15 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ Taho 4.01e'
 * Übergabeparameter von char auf QString geändert
*/
CPath::CPath()
{
   m_fInfo=QFileInfo();
   hasDrive=false;
}

CPath::~CPath()
{

}

CPath::CPath(QString path)
{
    m_fInfo.setFile(path);
    hasDrive=path.indexOf(":")==1;
}
// macht aus dem Pfad einen Pfad relativ zu hPath

QString CPath::relPath(QString hPath)
{
    QString rPath="",aPath=m_fInfo.path();
    QDir hDir(hPath);
    hPath=hDir.path();      //normalisieren ( / oder\ )
    if(hasDrive && aPath.left(1).compare(hPath.left(1))) // wenn auf unterschiedlichen Partitionen ist alles zwecklos
        return m_fInfo.filePath();
    if(aPath.indexOf(hPath)==0) // dann ist pfad in einem Subdir von hPath
        rPath="./";
    else
    {
        do{
            rPath+="../";
            hDir=QFileInfo(hDir.path()).path();
        }while(aPath.indexOf(hDir.path())!=0);	// solange bis der Anfang von m_dir gleich pHome.m_dir ist
    }
    if(hasDrive&& hDir.path().size()<4)	//dann nur gleiches Laufwerk
        rPath=aPath.mid(2);
    else
        rPath+=(aPath.mid(hDir.path().size()+1));
    QFileInfo tmp(rPath,m_fInfo.fileName());   // normieren (/ oder \)
    return tmp.filePath();
}

QString CPath::makePath(int elem)
{
    QString csPath="";
    switch(elem)
    {
    case 0:
        break;
    case PATH_PATH:
        csPath=m_fInfo.path();
        break;
    case PATH_FN:
        csPath=m_fInfo.baseName();
        break;
    case PATH_FN+PATH_PATH:
        csPath=m_fInfo.path()+m_fInfo.baseName();
        break;
    case PATH_EXT:
        csPath=m_fInfo.completeSuffix();
        break;
    case PATH_EXT+PATH_FN:
        csPath=m_fInfo.fileName();
        break;
    case PATH_EXT+PATH_FN+PATH_PATH:
        csPath=m_fInfo.filePath();

    }

     return csPath;
}

CPath& CPath::operator =(const QString &quell)
{
    *this=CPath(quell);
    return *this;
}

void CPath::setFile(QString path)
{
    QFileInfo fi(path);
    m_fInfo.setFile(path);
}

QString CPath::absPath(QString rPath, QString hPath)
{
    QString ret;
    QFileInfo fi(rPath);
    if(fi.isAbsolute())
        ret=fi.filePath();
    else
    {
        QString curr=QDir::currentPath();
        QDir::setCurrent(hPath);
        ret=fi.absoluteFilePath();
        QDir::setCurrent(curr);
    }
    return ret;
}

QString CPath::relPath(QString aPath, QString hPath)
{
    CPath aP(aPath);
    return aP.relPath(hPath);
}



void CPath::setComp(QString quell, int elem)
{

    switch(elem)
    {
    case 0:
        break;
    case PATH_PATH:
    {
        QDir dir(quell);
        QString fname=m_fInfo.fileName();
        if(fname.isEmpty())
            fname=".";
        m_fInfo.setFile(dir,fname);
        break;
    }
    case PATH_FN:
    {
        QDir dir=m_fInfo.path();
        QString fname=quell+"."+m_fInfo.completeSuffix();
        m_fInfo.setFile(dir,fname);
        break;
    }
    case PATH_FN+PATH_PATH: //Unsinn
        break;
    case PATH_EXT:
    {
        QDir dir=m_fInfo.path();
        QString fname=m_fInfo.baseName()+"."+quell;
        m_fInfo.setFile(dir,fname);
        break;
    }
    case PATH_EXT+PATH_FN:
    {
        QDir dir=m_fInfo.path();
        m_fInfo.setFile(dir,quell);
        break;
    }
    case PATH_EXT+PATH_FN+PATH_PATH:
        m_fInfo.setFile(quell);
    }
}

bool CPath::isFldr()
{
    return(m_fInfo.isDir());
}
bool CPath::isEmpty()
{
    return(m_fInfo.filePath().isEmpty());
}
