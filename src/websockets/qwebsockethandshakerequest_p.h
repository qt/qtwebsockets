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

#ifndef QWEBSOCKETHANDSHAKEREQUEST_P_H
#define QWEBSOCKETHANDSHAKEREQUEST_P_H
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
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QUrl>

#include "qwebsocketprotocol.h"

QT_BEGIN_NAMESPACE

class QTextStream;

class Q_AUTOTEST_EXPORT QWebSocketHandshakeRequest
{
    Q_DISABLE_COPY(QWebSocketHandshakeRequest)

public:
    QWebSocketHandshakeRequest(int port, bool isSecure);
    virtual ~QWebSocketHandshakeRequest();

    void clear();

    int port() const;
    bool isSecure() const;
    bool isValid() const;
    QMap<QString, QString> headers() const;
    QList<QWebSocketProtocol::Version> versions() const;
    QString key() const;
    QString origin() const;
    QList<QString> protocols() const;
    QList<QString> extensions() const;
    QUrl requestUrl() const;
    QString resourceName() const;
    QString host() const;

private:
    QTextStream &readFromStream(QTextStream &textStream);
    Q_AUTOTEST_EXPORT friend QTextStream &operator >>(QTextStream &stream, QWebSocketHandshakeRequest &request);

    int m_port;
    bool m_isSecure;
    bool m_isValid;
    QMap<QString, QString> m_headers;
    QList<QWebSocketProtocol::Version> m_versions;
    QString m_key;
    QString m_origin;
    QList<QString> m_protocols;
    QList<QString> m_extensions;
    QUrl m_requestUrl;
};

Q_AUTOTEST_EXPORT QTextStream & operator >>(QTextStream &stream, QWebSocketHandshakeRequest &request);

QT_END_NAMESPACE

#endif // QWEBSOCKETHANDSHAKEREQUEST_P_H
