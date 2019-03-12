#ifndef CPATH_H
#define CPATH_H

#include <qstringlist.h>
#include <qfileinfo.h>
enum {PATH_PATH=1,PATH_FN=2,PATH_EXT=4,PATH_FILE=6,PATH_FULL=7};
enum {FIND_NOMORE,FIND_MORESUB,FIND_OK};

class CPath
{
public:
    bool isEmpty();
    bool isFldr();
    int findNextSub();
    QStringList m_subDirs;
    void addSubDir(QString dir);
    int findNextFile();
    bool m_findRec;
 //   CFileFind m_find;
    int findFile(bool rec=false);
    void setComp(QString quell, int elem);
    static QString relPath(QString aPath, QString hPath);
    static QString absPath(QString qPath, QString hPath);
    void setFile(QString file);
//    bool up();
//    void delFile();
    QString makePath(int elem=PATH_FULL);
    QString relPath(QString hPath);
    CPath(QString path);
    CPath();
    virtual ~CPath();
    CPath& operator=(const QString &quell);
private:
    QFileInfo m_fInfo;
    bool hasDrive;
    //   QString m_drive,m_dir,m_fname,m_ext;
//    char m_drive[_MAX_DRIVE];
//    char m_dir[_MAX_DIR];
//    char m_fname[_MAX_FNAME];
//    char m_ext[_MAX_EXT];
};

#endif // CPATH_H
