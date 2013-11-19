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
#include <QScopedPointer>
#include <QWebSocket>

class QQmlWebSocket : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_DISABLE_COPY(QQmlWebSocket)
    Q_INTERFACES(QQmlParserStatus)

    Q_ENUMS(Status)
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)

public:
    explicit QQmlWebSocket(QObject *parent = Q_NULLPTR);
    virtual ~QQmlWebSocket();

    enum Status
    {
        Connecting  = 0,
        Open        = 1,
        Closing     = 2,
        Closed      = 3,
        Error       = 4
    };

    QUrl url() const;
    void setUrl(const QUrl &url);
    Status status() const;
    QString errorString() const;

    void setActive(bool active);
    bool isActive() const;

public Q_SLOTS:
    void sendTextMessage(const QString &message);


Q_SIGNALS:
    void textMessageReceived(QString message);
    void statusChanged(Status status);
    void activeChanged(bool isActive);
    void errorStringChanged(QString errorString);
    void urlChanged();

public:
    void classBegin() Q_DECL_OVERRIDE;
    void componentComplete() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void onError(QAbstractSocket::SocketError error);
    void onStateChanged(QAbstractSocket::SocketState state);

private:
    QScopedPointer<QWebSocket> m_webSocket;
    Status m_status;
    QUrl m_url;
    bool m_isActive;
    bool m_componentCompleted;
    QString m_errorString;

    void setStatus(Status status);
    void open();
    void close();
    void setErrorString(QString errorString = QString());
};

#endif // QQMLWEBSOCKET_H
