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

#ifndef QWEBSOCKETSERVER_P_H
#define QWEBSOCKETSERVER_P_H
//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtCore/QString>
#include <QtNetwork/QHostAddress>
#include "qwebsocket.h"

#ifndef QT_NO_SSL
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslError>
#endif

QT_BEGIN_NAMESPACE

class QTcpServer;
class QWebSocketServer;

class QWebSocketServerPrivate : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QWebSocketServerPrivate)
    Q_DECLARE_PUBLIC(QWebSocketServer)

public:
    enum SecureMode
    {
        SECURE_MODE = true,
        NON_SECURE_MODE
    };

    explicit QWebSocketServerPrivate(const QString &serverName, SecureMode secureMode, QWebSocketServer * const pWebSocketServer, QObject *parent = Q_NULLPTR);
    virtual ~QWebSocketServerPrivate();

    void close();
    QString errorString() const;
    bool hasPendingConnections() const;
    bool isListening() const;
    bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
    int maxPendingConnections() const;
    virtual QWebSocket *nextPendingConnection();
    void pauseAccepting();
#ifndef QT_NO_NETWORKPROXY
    QNetworkProxy proxy() const;
    void setProxy(const QNetworkProxy &networkProxy);
#endif
    void resumeAccepting();
    QHostAddress serverAddress() const;
    QWebSocketProtocol::CloseCode serverError() const;
    quint16 serverPort() const;
    void setMaxPendingConnections(int numConnections);
    bool setSocketDescriptor(qintptr socketDescriptor);
    qintptr socketDescriptor() const;
    bool waitForNewConnection(int msec = 0, bool *timedOut = Q_NULLPTR);

    QList<QWebSocketProtocol::Version> supportedVersions() const;
    QStringList supportedProtocols() const;
    QStringList supportedExtensions() const;

    void setServerName(const QString &serverName);
    QString serverName() const;

    SecureMode secureMode() const;

#ifndef QT_NO_SSL
    void setSslConfiguration(const QSslConfiguration &sslConfiguration);
    QSslConfiguration sslConfiguration() const;
#endif

    void setError(QWebSocketProtocol::CloseCode code, QString errorString);

private Q_SLOTS:
    void onNewConnection();
    void onCloseConnection();
    void handshakeReceived();

private:
    QWebSocketServer * const q_ptr;

    QTcpServer *m_pTcpServer;
    QString m_serverName;
    SecureMode m_secureMode;
    QQueue<QWebSocket *> m_pendingConnections;
    QWebSocketProtocol::CloseCode m_error;
    QString m_errorString;

    void addPendingConnection(QWebSocket *pWebSocket);
};

QT_END_NAMESPACE

#endif // QWEBSOCKETSERVER_P_H
