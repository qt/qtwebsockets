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
#include "echoserver.h"
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QtCore/QDebug>

QT_USE_NAMESPACE

//! [constructor]
EchoServer::EchoServer(quint16 port, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(new QWebSocketServer(QStringLiteral("Echo Server"),
                                            QWebSocketServer::SecureModeNonSecure, this)),
    m_clients()
{
    if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
        qDebug() << "Echoserver listening on port" << port;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                this, &EchoServer::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &EchoServer::closed);
    }
}
//! [constructor]

EchoServer::~EchoServer()
{
    m_pWebSocketServer->close();
    while (!m_clients.isEmpty()) {
        QWebSocket *pWebSocket = m_clients.takeFirst();
        delete pWebSocket;
    }
}

//! [onNewConnection]
void EchoServer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &EchoServer::processMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &EchoServer::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &EchoServer::socketDisconnected);

    m_clients << pSocket;
}
//! [onNewConnection]

//! [processMessage]
void EchoServer::processMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        qint64 bytesWritten = pClient->sendTextMessage(message);
        Q_UNUSED(bytesWritten);
    }
    m_pWebSocketServer->close();
}
//! [processMessage]

//! [processBinaryMessage]
void EchoServer::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        qint64 bytesWritten = pClient->sendBinaryMessage(message);
        Q_UNUSED(bytesWritten);
    }
}
//! [processBinaryMessage]

//! [socketDisconnected]
void EchoServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}
//! [socketDisconnected]
