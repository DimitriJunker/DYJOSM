#include "urldownload.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QEventLoop>
#include <QFileInfo>
#include <QFile>
#include <QAuthenticator>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <windows.h>


QStringList urlDownload::m_login;
QString AuthenticationHandler::m_login;
QString AuthenticationHandler::m_password;
int AuthenticationHandler::m_err;
int AuthenticationHandler::m_authTry;


urlDownload::urlDownload()
{
}

int urlDownload::downloadFile(const QString &url, const QDir &aPath,const QString &aFile,QString login, QString pwd)
{
    QFileInfo fi(aPath,aFile);

    return downloadFile(url,fi.filePath(),login,pwd);
}

int urlDownload::downloadFile(const QString &url, const QString &aPathInClient,QString login, QString pwd)
{
    int err;
    QNetworkAccessManager m_NetworkMngr;
    AuthenticationHandler authHandler;
    // introduce new parameters for the credentials or you need to transfer them here somehow
    authHandler.setLogin(login);
    authHandler.setPassword(pwd);
    QObject::connect(&m_NetworkMngr, &QNetworkAccessManager::authenticationRequired,
            &authHandler, &AuthenticationHandler::handleAuthentication);

    QNetworkReply * reply= m_NetworkMngr.get(QNetworkRequest(url));
//    QString url1="http://topomap.uzice.net/9/284/187.png";
//    QNetworkReply * reply= m_NetworkMngr.get(QNetworkRequest(url1));
    const QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> replyDestroyer(reply);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    err=authHandler.getErr();
    if(err==QNetworkReply::NoError)
        err=reply->error();
    if(err<QNetworkReply::InternalServerError)
    {
        m_login=authHandler.getLogin();
        QFileInfo fi(aPathInClient);
        QDir dir(fi.dir());
        if(!dir.exists())
            dir.mkpath(fi.path());
        QFile file(aPathInClient);
        file.open(QIODevice::WriteOnly);
        const auto ba=reply->readAll();
        file.write(ba);
        file.close();
    }


    return err;
}
QStringList urlDownload::getLogin()
{
    return m_login;
}

//---------------------------------------------------------------------
AuthenticationHandler::AuthenticationHandler(QObject * p) : QObject(p)
{
    m_err=QNetworkReply::NoError;

}

void AuthenticationHandler::setLogin(const QString &login)
{
    m_login = login;
    m_authTry=0;
}

void AuthenticationHandler::setPassword(const QString &password)
{
    m_password = password;
}

void AuthenticationHandler::handleAuthentication(QNetworkReply *, QAuthenticator * auth)
{
    m_authTry++;
    qDebug() << "Authenticating as " << m_login << "/" << m_password << "try:" << m_authTry;
    QString loginStr="";
    bool ok=true,ask=false;

    if(m_login.isEmpty()||m_password.isEmpty())
    {
        loginStr=QInputDialog::getText(nullptr,QObject::tr("Bitte Login Info für Tileserver eingeben"),
                                       QObject::tr("Eingabe durch ':' getrennt, also: username:password"),QLineEdit::Normal,"",&ok);
    }
    else if(m_authTry>1)
    {
        loginStr=QString("%1:%2").arg(m_login).arg(m_password);
        ask=true;
    }
    while(ok && !loginStr.isEmpty())
    {
        if(!ask)
        {
            QStringList loginStrL=loginStr.split(':');
            if(loginStrL.count()==2)
            {
                m_login=loginStrL[0];
                m_password=loginStrL[1];
                m_err=2;
                m_authTry=1;
                loginStr="";
            }
            else {
                ask=true;
            }
        }
        if(ask)
        {
            loginStr=QInputDialog::getText(nullptr,QObject::tr("Login Info für Tileserver war falsch"),
                                               QObject::tr("Eingabe durch ':' getrennt, also: username:password"),QLineEdit::Normal,loginStr,&ok);
            ask=false;
        }
    }

    if(!ok)
    {
        QMessageBox::warning(nullptr, "", QObject::tr("Bitte andere Quelle wählen oder sich auf der Homepage anmelden"),
                             QMessageBox::Ok);
        m_err=QNetworkReply::AuthenticationRequiredError;
    }
    else
    {
        auth->setUser(m_login);
        auth->setPassword(m_password);
    }
}
int AuthenticationHandler::getErr()
{
    return m_err;
}

QStringList AuthenticationHandler::getLogin()
{
    QStringList lo={m_login,m_password};
    return lo;

}
