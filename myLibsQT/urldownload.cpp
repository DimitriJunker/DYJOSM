#include "urldownload.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QMessageBox>
#include <QStandardPaths>
#include <qeventloop.h>
#include <qfileinfo.h>
#include <qfile.h>

urlDownload::urlDownload()
{
}

int urlDownload::downloadFile(const QString &url, const QDir &aPath,const QString &aFile)
{
    QFileInfo fi(aPath,aFile);
    return downloadFile(url,fi.filePath ());

}

int urlDownload::downloadFile(const QString &url, const QString &aPathInClient)
{
        QNetworkAccessManager m_NetworkMngr;
        QNetworkReply *reply= m_NetworkMngr.get(QNetworkRequest(url));
        QEventLoop loop;
        QObject::connect(reply, SIGNAL(finished()),&loop, SLOT(quit()));
        loop.exec();
        QUrl aUrl(url);
        QFileInfo fileInfo=aUrl.path();

//        QFile file(aPathInClient+"/"+fileInfo.fileName());

        QFileInfo fi(aPathInClient);
        QDir dir(fi.dir());
        if(!dir.exists())
            dir.mkpath(fi.path());

        QFile file(aPathInClient);
        file.open(QIODevice::WriteOnly);
        QByteArray ba=reply->readAll();
        file.write(ba);
        file.close();

        delete reply;
        return 0;
}
