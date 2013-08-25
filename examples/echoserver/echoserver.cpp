#include "echoserver.h"
#include "qwebsocketserver.h"
#include "qwebsocket.h"
#include <QDebug>

//! [constructor]
EchoServer::EchoServer(quint16 port, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(0),
    m_clients()
{
    m_pWebSocketServer = new QWebSocketServer("Echo Server", this);
    if (m_pWebSocketServer->listen(QHostAddress::Any, port))
    {
        qDebug() << "Echoserver listening on port" << port;
        connect(m_pWebSocketServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    }
}
//! [constructor]

//! [onNewConnection]
void EchoServer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(processMessage(QString)));
    connect(pSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(processBinaryMessage(QByteArray)));
    connect(pSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    //connect(pSocket, SIGNAL(pong(quint64)), this, SLOT(processPong(quint64)));

    m_clients << pSocket;
}
//! [onNewConnection]

//! [processMessage]
void EchoServer::processMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient != 0)
    {
        pClient->write(message);
    }
}
//! [processMessage]

//! [processBinaryMessage]
void EchoServer::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient != 0)
    {
        pClient->write(message);
    }
}
//! [processBinaryMessage]

//! [socketDisconnected]
void EchoServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient != 0)
    {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}
//! [socketDisconnected]
