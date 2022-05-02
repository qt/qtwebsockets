/****************************************************************************
**
** Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QWEBSOCKETHANDSHAKERESPONSE_P_H
#define QWEBSOCKETHANDSHAKERESPONSE_P_H
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
#include <QtCore/QList>
#include "qwebsocketprotocol.h"

QT_BEGIN_NAMESPACE

class QWebSocketHandshakeRequest;
class QString;
class QTextStream;

class Q_AUTOTEST_EXPORT QWebSocketHandshakeResponse : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QWebSocketHandshakeResponse)

public:
    QWebSocketHandshakeResponse(const QWebSocketHandshakeRequest &request,
                      const QString &serverName,
                      bool isOriginAllowed,
                      const QList<QWebSocketProtocol::Version> &supportedVersions,
                      const QList<QString> &supportedProtocols,
                      const QList<QString> &supportedExtensions);

    ~QWebSocketHandshakeResponse() override;

    bool isValid() const;
    bool canUpgrade() const;
    QString acceptedProtocol() const;
    QString acceptedExtension() const;
    QWebSocketProtocol::Version acceptedVersion() const;

    QWebSocketProtocol::CloseCode error() const;
    QString errorString() const;

private:
    bool m_isValid;
    bool m_canUpgrade;
    QString m_response;
    QString m_acceptedProtocol;
    QString m_acceptedExtension;
    QWebSocketProtocol::Version m_acceptedVersion;
    QWebSocketProtocol::CloseCode m_error;
    QString m_errorString;

    QString calculateAcceptKey(const QString &key) const;
    QString getHandshakeResponse(const QWebSocketHandshakeRequest &request,
                                 const QString &serverName,
                                 bool isOriginAllowed,
                                 const QList<QWebSocketProtocol::Version> &supportedVersions,
                                 const QList<QString> &supportedProtocols,
                                 const QList<QString> &supportedExtensions);

    QTextStream &writeToStream(QTextStream &textStream) const;
    Q_AUTOTEST_EXPORT friend QTextStream & operator <<(QTextStream &stream,
                                                       const QWebSocketHandshakeResponse &response);
};

Q_AUTOTEST_EXPORT QTextStream & operator <<(QTextStream &stream,
                                            const QWebSocketHandshakeResponse &response);

QT_END_NAMESPACE

#endif // QWEBSOCKETHANDSHAKERESPONSE_P_H
