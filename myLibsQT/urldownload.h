// Quelle: http://tomtheprogrammer.wordpress.com/2013/05/27/qt-5-download-a-file/

#ifndef URLDOWNLOAD_H
#define URLDOWNLOAD_H
#include <QString>
#include <QLineEdit>
#include <QDir>
//#include <QNetworkReply>
class urlDownload
{
public:
    urlDownload();
    static int downloadFile(const QString &url, const QString &aPathInClient);
    static int downloadFile(const QString &url, const QDir &aPath,const QString &aFile);


};
#endif // URLDOWNLOAD_H
