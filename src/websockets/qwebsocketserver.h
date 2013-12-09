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

#ifndef QWEBSOCKETSERVER_H
#define QWEBSOCKETSERVER_H

#include "QtWebSockets/qwebsockets_global.h"
#include "QtWebSockets/qwebsocketprotocol.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtNetwork/QHostAddress>

#ifndef QT_NO_SSL
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslError>
#endif

QT_BEGIN_NAMESPACE

class QWebSocketServerPrivate;
class QWebSocket;
class QWebSocketCorsAuthenticator;

class Q_WEBSOCKETS_EXPORT QWebSocketServer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QWebSocketServer)
    Q_DECLARE_PRIVATE(QWebSocketServer)

    Q_ENUMS(SecureMode)

public:
    enum SecureMode {
#ifndef QT_NO_SSL
        SECURE_MODE,
#endif
        NON_SECURE_MODE
    };

    explicit QWebSocketServer(const QString &serverName, SecureMode secureMode, QObject *parent = Q_NULLPTR);
    virtual ~QWebSocketServer();

    bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
    void close();

    bool isListening() const;

    void setMaxPendingConnections(int numConnections);
    int maxPendingConnections() const;

    quint16 serverPort() const;
    QHostAddress serverAddress() const;

    SecureMode secureMode() const;

    bool setSocketDescriptor(int socketDescriptor);
    int socketDescriptor() const;

    bool waitForNewConnection(int msec = 0, bool *timedOut = 0);
    bool hasPendingConnections() const;
    virtual QWebSocket *nextPendingConnection();

    QWebSocketProtocol::CloseCode error() const;
    QString errorString() const;

    void pauseAccepting();
    void resumeAccepting();

    void setServerName(const QString &serverName);
    QString serverName() const;

#ifndef QT_NO_NETWORKPROXY
    void setProxy(const QNetworkProxy &networkProxy);
    QNetworkProxy proxy() const;
#endif
#ifndef QT_NO_SSL
    void setSslConfiguration(const QSslConfiguration &sslConfiguration);
    QSslConfiguration sslConfiguration() const;
#endif

    QList<QWebSocketProtocol::Version> supportedVersions() const;
    QStringList supportedProtocols() const;
    QStringList supportedExtensions() const;

Q_SIGNALS:
    void acceptError(QAbstractSocket::SocketError socketError);
    void serverError(QWebSocketProtocol::CloseCode closeCode);
    //TODO: should use a delegate iso of a synchronous signal
    void originAuthenticationRequired(QWebSocketCorsAuthenticator *pAuthenticator);
    void newConnection();
#ifndef QT_NO_SSL
    void peerVerifyError(const QSslError &error);
    void sslErrors(const QList<QSslError> &errors);
#endif
    void closed();

private:
    QWebSocketServerPrivate * const d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBSOCKETSERVER_H
