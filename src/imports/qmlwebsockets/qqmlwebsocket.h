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

#ifndef QQMLWEBSOCKET_H
#define QQMLWEBSOCKET_H

#include <QObject>
#include <QQmlParserStatus>
#include <QtQml>
#include <QWebSocket>

class QQmlWebSocket : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_DISABLE_COPY(QQmlWebSocket)
    Q_INTERFACES(QQmlParserStatus)

    Q_ENUMS(ReadyState)
    Q_ENUMS(Exception)
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(ReadyState readyState READ readyState)
    Q_PROPERTY(QString extensions READ extensions)
    Q_PROPERTY(QString protocol READ protocol)

public:
    explicit QQmlWebSocket(QObject *parent = Q_NULLPTR);
    virtual ~QQmlWebSocket();

    enum ReadyState
    {
        CONNECTING  = 0,
        OPEN        = 1,
        CLOSING     = 2,
        CLOSED      = 3
    };
    enum Exception
    {
        SyntaxError,
        SecurityError,
        InvalidAccessError,
        InvalidStateError
    };

    Q_INVOKABLE void sendTextMessage(const QString &message);
    Q_INVOKABLE void sendBinaryMessage(const QByteArray &message);
    Q_INVOKABLE void close(quint16 code = 1000, const QString &reason = QString());

    QUrl url() const;
    void setUrl(const QUrl &url);
    ReadyState readyState() const;
    QString extensions() const;
    QString protocol() const;

Q_SIGNALS:
    void connected();
    void errorOccurred(quint16 errno, QString message);
    void closed(bool wasClean, quint16 code, QString reason);
    void textMessage(QString message);
    void binaryMessage(QByteArray message);
    void exception(Exception exception);
    void stateChanged(ReadyState readyState);
    void urlChanged();

public:
    void classBegin() Q_DECL_OVERRIDE;
    void componentComplete() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void onError(QAbstractSocket::SocketError error);
    void onStateChanged(QAbstractSocket::SocketState state);
    void onDisconnected();

private:
    QWebSocket *m_pWebSocket;
    ReadyState m_readyState;
    QUrl m_url;
    QStringList m_protocols;
};

#endif // QQMLWEBSOCKET_H
