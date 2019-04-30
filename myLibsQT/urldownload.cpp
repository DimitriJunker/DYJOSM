#include "urldownload.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QMessageBox>
#include <QScopedPointer>
#include <QEventLoop>
#include <QFileInfo>
#include <QFile>
#include <QDir>

int urlDownload::downloadFile(const QString &url, const QDir &aPath, const QString &aFile)
{
    const QFileInfo fi(aPath, aFile);
    return downloadFile(url, fi.filePath());
}

int urlDownload::downloadFile(const QString &url, const QString &aPathInClient)
{
    QNetworkAccessManager networkManager;
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> reply(
        networkManager.get(QNetworkRequest(url)));
    QEventLoop loop;
    QObject::connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const QFileInfo fileInfo = QUrl(url).path();

//        QFile file(aPathInClient+"/"+fileInfo.fileName());

    {
        const QFileInfo clientPathFi(aPathInClient);
        QDir dir(clientPathFi.dir());
        if(!dir.exists())
            dir.mkpath(clientPathFi.path());
    }

    QFile file(aPathInClient);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(reply->readAll());
        return 0;
    }

    return -1;
}
