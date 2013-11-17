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
    m_webSocket(),
    m_status(Closed),
    m_url(),
    m_isActive(false),
    m_componentCompleted(false)
{
}

QQmlWebSocket::~QQmlWebSocket()
{
}

void QQmlWebSocket::sendTextMessage(const QString &message)
{
    m_webSocket->write(message);
}

void QQmlWebSocket::sendBinaryMessage(const QByteArray &message)
{
    m_webSocket->write(message);
}

QUrl QQmlWebSocket::url() const
{
    return m_url;
}

void QQmlWebSocket::setUrl(const QUrl &url)
{
    if (m_url != url) {
        if (m_webSocket && (m_webSocket->state() == QAbstractSocket::ConnectedState)) {
            m_webSocket->close();
        }
        m_url = url;
        Q_EMIT urlChanged();
        if (m_webSocket) {
            m_webSocket->open(m_url);
        }
    }
}

QQmlWebSocket::Status QQmlWebSocket::status() const
{
    return m_status;
}

QString QQmlWebSocket::errorString() const
{
    QString error;
    if (m_componentCompleted)
    {
        error = m_webSocket->errorString();
    }
    return error;
}

void QQmlWebSocket::classBegin()
{
    m_componentCompleted = false;
}

void QQmlWebSocket::componentComplete()
{
    m_webSocket.reset(new QWebSocket());
    connect(m_webSocket.data(), SIGNAL(connected()), this, SLOT(onConnected()));
    connect(m_webSocket.data(), SIGNAL(binaryMessageReceived(QByteArray)), this, SIGNAL(binaryMessageReceived(QByteArray)));
    connect(m_webSocket.data(), SIGNAL(textMessageReceived(QString)), this, SIGNAL(textMessageReceived(QString)));
    connect(m_webSocket.data(), SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(m_webSocket.data(), SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));
    connect(m_webSocket.data(), SIGNAL(disconnected()), this, SLOT(onDisconnected()));

    m_componentCompleted = true;

    open();
}

void QQmlWebSocket::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    setStatus(Error);
}

void QQmlWebSocket::onStateChanged(QAbstractSocket::SocketState state)
{
    switch (state)
    {
        case QAbstractSocket::ConnectingState:
        case QAbstractSocket::BoundState:
        case QAbstractSocket::HostLookupState:
        {
            setStatus(Connecting);
            break;
        }
        case QAbstractSocket::UnconnectedState:
        {
            setStatus(Closed);
            break;
        }
        case QAbstractSocket::ConnectedState:
        {
            setStatus(Open);
            break;
        }
        case QAbstractSocket::ClosingState:
        {
            setStatus(Closing);
            break;
        }
        default:
        {
            setStatus(Connecting);
            break;
        }
    }
}

void QQmlWebSocket::onConnected()
{
    Q_EMIT activeChanged(true);
}

void QQmlWebSocket::onDisconnected()
{
    Q_EMIT activeChanged(false);
}

void QQmlWebSocket::setStatus(QQmlWebSocket::Status status)
{
    if (m_status != status) {
        m_status = status;
        Q_EMIT statusChanged(m_status);
    }
}

void QQmlWebSocket::setActive(bool active)
{
    if (m_isActive != active) {
        m_isActive = active;
        if (m_componentCompleted) {
            if (m_isActive) {
                open();
            }
            else {
                close();
            }
        }
    }
}

bool QQmlWebSocket::isActive() const
{
    return m_isActive;
}

void QQmlWebSocket::open()
{
    if (m_componentCompleted && m_isActive && m_url.isValid() && m_webSocket) {
        m_webSocket->open(m_url);
    }
}

void QQmlWebSocket::close()
{
    if (m_componentCompleted && m_webSocket) {
        m_webSocket->close();
    }
}
