// Quelle: http://tomtheprogrammer.wordpress.com/2013/05/27/qt-5-download-a-file/

#ifndef URLDOWNLOAD_H
#define URLDOWNLOAD_H

class QString;
class QDir;

class urlDownload
{
public:
    static int downloadFile(const QString &url, const QString &aPathInClient);
    static int downloadFile(const QString &url, const QDir &aPath, const QString &aFile);

    urlDownload() = delete;
};
#endif // URLDOWNLOAD_H
