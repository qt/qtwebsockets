/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebsocketserver.h"
#include "qwebsocketserver_p.h"
#ifndef QT_NO_SSL
#include "qsslserver_p.h"
#endif
#include "qwebsocketprotocol.h"
#include "qwebsockethandshakerequest_p.h"
#include "qwebsockethandshakeresponse_p.h"
#include "qwebsocket.h"
#include "qwebsocket_p.h"
#include "qwebsocketcorsauthenticator.h"

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QNetworkProxy>

QT_BEGIN_NAMESPACE

/*!
    \internal
 */
QWebSocketServerPrivate::QWebSocketServerPrivate(const QString &serverName, QWebSocketServerPrivate::SecureMode secureMode,
                                                 QWebSocketServer * const pWebSocketServer, QObject *parent) :
    QObject(parent),
    q_ptr(pWebSocketServer),
    m_pTcpServer(Q_NULLPTR),
    m_serverName(serverName),
    m_secureMode(secureMode),
    m_pendingConnections(),
    m_error(QWebSocketProtocol::CC_NORMAL),
    m_errorString()
{
    Q_ASSERT(pWebSocketServer);
    if (m_secureMode == NON_SECURE_MODE) {
        m_pTcpServer = new QTcpServer(this);
        connect(m_pTcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    } else
    {
#ifndef QT_NO_SSL
        QSslServer *pSslServer = new QSslServer(this);
        m_pTcpServer = pSslServer;
        connect(pSslServer, SIGNAL(newEncryptedConnection()), this, SLOT(onNewConnection()));
        connect(pSslServer, SIGNAL(peerVerifyError(QSslError)), q_ptr, SIGNAL(peerVerifyError(QSslError)));
        connect(pSslServer, SIGNAL(sslErrors(QList<QSslError>)), q_ptr, SIGNAL(sslErrors(QList<QSslError>)));
#else
        qFatal("SSL not supported on this platform.");
#endif
    }
    connect(m_pTcpServer, SIGNAL(acceptError(QAbstractSocket::SocketError)), q_ptr, SIGNAL(acceptError(QAbstractSocket::SocketError)));
}

/*!
    \internal
 */
QWebSocketServerPrivate::~QWebSocketServerPrivate()
{
    close();
    m_pTcpServer->deleteLater();
}

/*!
    \internal
 */
void QWebSocketServerPrivate::close()
{
    Q_Q(QWebSocketServer);
    m_pTcpServer->close();
    while (!m_pendingConnections.isEmpty()) {
        QWebSocket *pWebSocket = m_pendingConnections.dequeue();
        pWebSocket->close(QWebSocketProtocol::CC_GOING_AWAY, tr("Server closed."));
        pWebSocket->deleteLater();
    }
    //emit signal via the event queue, so the server gets time
    //to process any hanging events, like flushing buffers aso
    QMetaObject::invokeMethod(q, "closed", Qt::QueuedConnection);
}

/*!
    \internal
 */
QString QWebSocketServerPrivate::errorString() const
{
    if (m_errorString.isEmpty()) {
        return m_pTcpServer->errorString();
    } else {
        return m_errorString;
    }
}

/*!
    \internal
 */
bool QWebSocketServerPrivate::hasPendingConnections() const
{
    return !m_pendingConnections.isEmpty();
}

/*!
    \internal
 */
bool QWebSocketServerPrivate::isListening() const
{
    return m_pTcpServer->isListening();
}

/*!
    \internal
 */
bool QWebSocketServerPrivate::listen(const QHostAddress &address, quint16 port)
{
    return m_pTcpServer->listen(address, port);
}

/*!
    \internal
 */
int QWebSocketServerPrivate::maxPendingConnections() const
{
    return m_pTcpServer->maxPendingConnections();
}

/*!
    \internal
 */
void QWebSocketServerPrivate::addPendingConnection(QWebSocket *pWebSocket)
{
    if (m_pendingConnections.size() < maxPendingConnections()) {
        m_pendingConnections.enqueue(pWebSocket);
    }
}

/*!
    \internal
 */
QWebSocket *QWebSocketServerPrivate::nextPendingConnection()
{
    QWebSocket *pWebSocket = Q_NULLPTR;
    if (!m_pendingConnections.isEmpty()) {
        pWebSocket = m_pendingConnections.dequeue();
    }
    return pWebSocket;
}

/*!
    \internal
 */
void QWebSocketServerPrivate::pauseAccepting()
{
    m_pTcpServer->pauseAccepting();
}

#ifndef QT_NO_NETWORKPROXY
/*!
    \internal
 */
QNetworkProxy QWebSocketServerPrivate::proxy() const
{
    return m_pTcpServer->proxy();
}

/*!
    \internal
 */
void QWebSocketServerPrivate::setProxy(const QNetworkProxy &networkProxy)
{
    m_pTcpServer->setProxy(networkProxy);
}
#endif
/*!
    \internal
 */
void QWebSocketServerPrivate::resumeAccepting()
{
    m_pTcpServer->resumeAccepting();
}

/*!
    \internal
 */
QHostAddress QWebSocketServerPrivate::serverAddress() const
{
    return m_pTcpServer->serverAddress();
}

/*!
    \internal
 */
QWebSocketProtocol::CloseCode QWebSocketServerPrivate::serverError() const
{
    return m_error;
}

/*!
    \internal
 */
quint16 QWebSocketServerPrivate::serverPort() const
{
    return m_pTcpServer->serverPort();
}

/*!
    \internal
 */
void QWebSocketServerPrivate::setMaxPendingConnections(int numConnections)
{
    m_pTcpServer->setMaxPendingConnections(numConnections);
}

/*!
    \internal
 */
bool QWebSocketServerPrivate::setSocketDescriptor(qintptr socketDescriptor)
{
    return m_pTcpServer->setSocketDescriptor(socketDescriptor);
}

/*!
    \internal
 */
qintptr QWebSocketServerPrivate::socketDescriptor() const
{
    return m_pTcpServer->socketDescriptor();
}

/*!
    \internal
 */
bool QWebSocketServerPrivate::waitForNewConnection(int msec, bool *timedOut)
{
    return m_pTcpServer->waitForNewConnection(msec, timedOut);
}

/*!
    \internal
 */
QList<QWebSocketProtocol::Version> QWebSocketServerPrivate::supportedVersions() const
{
    QList<QWebSocketProtocol::Version> supportedVersions;
    supportedVersions << QWebSocketProtocol::currentVersion();	//we only support V13
    return supportedVersions;
}

/*!
    \internal
 */
QStringList QWebSocketServerPrivate::supportedProtocols() const
{
    QList<QString> supportedProtocols;
    return supportedProtocols;	//no protocols are currently supported
}

/*!
    \internal
 */
QStringList QWebSocketServerPrivate::supportedExtensions() const
{
    QList<QString> supportedExtensions;
    return supportedExtensions;	//no extensions are currently supported
}

/*!
  \internal
 */
void QWebSocketServerPrivate::setServerName(const QString &serverName)
{
    if (m_serverName != serverName) {
        m_serverName = serverName;
    }
}

/*!
  \internal
 */
QString QWebSocketServerPrivate::serverName() const
{
    return m_serverName;
}

/*!
  \internal
 */
QWebSocketServerPrivate::SecureMode QWebSocketServerPrivate::secureMode() const
{
    return m_secureMode;
}

#ifndef QT_NO_SSL
void QWebSocketServerPrivate::setSslConfiguration(const QSslConfiguration &sslConfiguration)
{
    if (m_secureMode == SECURE_MODE) {
        qobject_cast<QSslServer *>(m_pTcpServer)->setSslConfiguration(sslConfiguration);
    } else {
        qWarning() << tr("Cannot set SSL configuration for non-secure server.");
    }
}

QSslConfiguration QWebSocketServerPrivate::sslConfiguration() const
{
    if (m_secureMode == SECURE_MODE) {
        return qobject_cast<QSslServer *>(m_pTcpServer)->sslConfiguration();
    } else {
        return QSslConfiguration::defaultConfiguration();
    }
}
#endif

void QWebSocketServerPrivate::setError(QWebSocketProtocol::CloseCode code, QString errorString)
{
    if ((m_error != code) || (m_errorString != errorString)) {
        Q_Q(QWebSocketServer);
        m_error = code;
        m_errorString = errorString;
        Q_EMIT q->serverError(code);
    }
}

/*!
    \internal
 */
void QWebSocketServerPrivate::onNewConnection()
{
    QTcpSocket *pTcpSocket = m_pTcpServer->nextPendingConnection();
    connect(pTcpSocket, SIGNAL(readyRead()), this, SLOT(handshakeReceived()));
}

/*!
    \internal
 */
void QWebSocketServerPrivate::onCloseConnection()
{
    QTcpSocket *pTcpSocket = qobject_cast<QTcpSocket*>(sender());
    if (pTcpSocket) {
        pTcpSocket->close();
    }
}

/*!
    \internal
 */
void QWebSocketServerPrivate::handshakeReceived()
{
    Q_Q(QWebSocketServer);
    QTcpSocket *pTcpSocket = qobject_cast<QTcpSocket*>(sender());
    if (pTcpSocket) {
        bool success = false;
        bool isSecure = false;

        disconnect(pTcpSocket, SIGNAL(readyRead()), this, SLOT(handshakeReceived()));

        QWebSocketHandshakeRequest request(pTcpSocket->peerPort(), isSecure);
        QTextStream textStream(pTcpSocket);
        textStream >> request;

        if (request.isValid()) {
            QWebSocketCorsAuthenticator corsAuthenticator(request.origin());
            Q_EMIT q->originAuthenticationRequired(&corsAuthenticator);

            QWebSocketHandshakeResponse response(request,
                                                 m_serverName,
                                                 corsAuthenticator.allowed(),
                                                 supportedVersions(),
                                                 supportedProtocols(),
                                                 supportedExtensions());

            if (response.isValid()) {
                QTextStream httpStream(pTcpSocket);
                httpStream << response;
                httpStream.flush();

                if (response.canUpgrade()) {
                    QWebSocket *pWebSocket = QWebSocketPrivate::upgradeFrom(pTcpSocket, request, response);
                    if (pWebSocket) {
                        pWebSocket->setParent(this);
                        addPendingConnection(pWebSocket);
                        Q_EMIT q->newConnection();
                        success = true;
                    } else {
                        setError(QWebSocketProtocol::CC_ABNORMAL_DISCONNECTION, tr("Upgrading to websocket failed."));
                    }
                }
                else {
                    setError(response.error(), response.errorString());
                }
            } else {
                setError(QWebSocketProtocol::CC_PROTOCOL_ERROR, tr("Invalid response received."));
            }
        }
        if (!success) {
            qWarning() << tr("Closing socket because of invalid or unsupported request.");
            pTcpSocket->close();
        }
    } else {
        qWarning() << tr("Sender socket is NULL. This should not happen, otherwise it is a Qt bug!!!");
    }
}

QT_END_NAMESPACE
