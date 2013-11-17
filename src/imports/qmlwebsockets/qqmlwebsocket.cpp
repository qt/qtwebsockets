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

#include "qqmlwebsocket.h"
#include <QWebSocket>

QQmlWebSocket::QQmlWebSocket(QObject *parent) :
    QObject(parent),
    m_pWebSocket(Q_NULLPTR),
    m_readyState(CONNECTING),
    m_url(),
    m_protocols()
{
}

QQmlWebSocket::~QQmlWebSocket()
{
    delete m_pWebSocket;
    m_pWebSocket = Q_NULLPTR;
}

void QQmlWebSocket::sendTextMessage(const QString &message)
{
    m_pWebSocket->write(message);
}

void QQmlWebSocket::sendBinaryMessage(const QByteArray &message)
{
    m_pWebSocket->write(message);
}

void QQmlWebSocket::close(quint16 code, const QString &reason)
{
    //see http://www.w3.org/TR/websockets
    if ((code == 1000) && (code >= 3000) && (code <= 4999))
    {
        m_pWebSocket->close(static_cast<QWebSocketProtocol::CloseCode>(code), reason);
    }
    else
    {
        Q_EMIT exception(InvalidAccessError);
    }
}

QUrl QQmlWebSocket::url() const
{
    return m_url;
}

void QQmlWebSocket::setUrl(const QUrl &url)
{
    if (m_url != url)
    {
        if (m_pWebSocket && (m_pWebSocket->state() == QAbstractSocket::ConnectedState))
        {
            m_pWebSocket->close();
        }
        m_url = url;
        Q_EMIT urlChanged();
        if (m_pWebSocket)
        {
            m_pWebSocket->open(m_url);
        }
    }
}

QQmlWebSocket::ReadyState QQmlWebSocket::readyState() const
{
    return m_readyState;
}

QString QQmlWebSocket::extensions() const
{
    return m_pWebSocket->extension();
}

QString QQmlWebSocket::protocol() const
{
    return m_pWebSocket->protocol();
}

void QQmlWebSocket::classBegin()
{
}

void QQmlWebSocket::componentComplete()
{
    m_pWebSocket = new QWebSocket();
    connect(m_pWebSocket, SIGNAL(connected()), this, SIGNAL(connected()));
    connect(m_pWebSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SIGNAL(binaryMessage(QByteArray)));
    connect(m_pWebSocket, SIGNAL(textMessageReceived(QString)), this, SIGNAL(textMessage(QString)));
    connect(m_pWebSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(m_pWebSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));
    connect(m_pWebSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    m_pWebSocket->open(m_url);
}

void QQmlWebSocket::onError(QAbstractSocket::SocketError error)
{
    Q_EMIT errorOccurred(static_cast<quint16>(error), m_pWebSocket->errorString());
}

void QQmlWebSocket::onStateChanged(QAbstractSocket::SocketState state)
{
    switch (state)
    {
        case QAbstractSocket::ConnectingState:
        case QAbstractSocket::BoundState:
        case QAbstractSocket::HostLookupState:
        {
            m_readyState = CONNECTING;
            break;
        }
        case QAbstractSocket::UnconnectedState:
        {
            m_readyState = CLOSED;
            break;
        }
        case QAbstractSocket::ConnectedState:
        {
            m_readyState = OPEN;
            break;
        }
        case QAbstractSocket::ClosingState:
        {
            m_readyState = CLOSING;
            break;
        }
        default:
        {
            m_readyState = CONNECTING;
            break;
        }
    }
    Q_EMIT stateChanged(m_readyState);
}

void QQmlWebSocket::onDisconnected()
{
    Q_EMIT closed(m_pWebSocket->closeCode() == QWebSocketProtocol::CC_NORMAL,
                  static_cast<quint16>(m_pWebSocket->closeCode()),
                  m_pWebSocket->closeReason());
}
