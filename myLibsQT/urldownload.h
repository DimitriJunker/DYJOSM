// Quelle: http://tomtheprogrammer.wordpress.com/2013/05/27/qt-5-download-a-file/

#ifndef URLDOWNLOAD_H
#define URLDOWNLOAD_H
#include <QString>
#include <QDir>

class QNetworkReply;
class QAuthenticator;

class urlDownload
{
public:
    urlDownload();
    static int downloadFile(const QString &url, const QString &aPathInClient,QString login="", QString pwd="");
    static int downloadFile(const QString &url, const QDir &aPath, const QString &aFile,QString login="", QString pwd="");
    static QStringList getLogin();
    static QStringList m_login;
};
class AuthenticationHandler : public QObject
{
    Q_OBJECT
public:
    explicit AuthenticationHandler(QObject * = nullptr);

    void setLogin(const QString &login);
    void setPassword(const QString &password);
    static QStringList getLogin();

public slots:
    void handleAuthentication(QNetworkReply* rep, QAuthenticator * auth);
    int getErr();

private:
    static QString m_login;
    static QString m_password;
    static int m_err;
    static int m_authTry;
};

#endif // URLDOWNLOAD_H
