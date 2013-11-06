#include "qsslserver_p.h"

#include <QtNetwork/QSslSocket>
#include <QtNetwork/QSslCipher>

QSslServer::QSslServer(QObject *parent) :
    QTcpServer(parent),
    m_sslConfiguration(QSslConfiguration::defaultConfiguration())
{
}

QSslServer::~QSslServer()
{
}

void QSslServer::setSslConfiguration(const QSslConfiguration &sslConfiguration)
{
    m_sslConfiguration = sslConfiguration;
}

QSslConfiguration QSslServer::sslConfiguration() const
{
    return m_sslConfiguration;
}

void QSslServer::incomingConnection(qintptr socket)
{
    QSslSocket *pSslSocket = new QSslSocket();

    pSslSocket->setSslConfiguration(m_sslConfiguration);

    if (pSslSocket->setSocketDescriptor(socket))
    {
        connect(pSslSocket, SIGNAL(peerVerifyError(QSslError)), this, SIGNAL(peerVerifyError(QSslError)));
        connect(pSslSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SIGNAL(sslErrors(QList<QSslError>)));
        connect(pSslSocket, SIGNAL(encrypted()), this, SIGNAL(newEncryptedConnection()));

        addPendingConnection(pSslSocket);

        pSslSocket->startServerEncryption();
    }
    else
    {
       delete pSslSocket;
    }
}
