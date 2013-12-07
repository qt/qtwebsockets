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

#include "qwebsockethandshakerequest_p.h"
#include "qwebsocketprotocol.h"

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QTextStream>
#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <functional>   //for std::greater

QT_BEGIN_NAMESPACE

/*!
    \brief Constructs a new QWebSocketHandshakeRequest given \a port and \a isSecure
    \internal
 */
QWebSocketHandshakeRequest::QWebSocketHandshakeRequest(int port, bool isSecure) :
    m_port(port),
    m_isSecure(isSecure),
    m_isValid(false),
    m_headers(),
    m_versions(),
    m_key(),
    m_origin(),
    m_protocols(),
    m_extensions(),
    m_requestUrl()
{
}

/*!
    \internal
 */
QWebSocketHandshakeRequest::~QWebSocketHandshakeRequest()
{
}

/*!
    \brief Clears the request
    \internal
 */
void QWebSocketHandshakeRequest::clear()
{
    m_isValid = false;
    m_headers.clear();
    m_versions.clear();
    m_key.clear();
    m_origin.clear();
    m_protocols.clear();
    m_extensions.clear();
    m_requestUrl.clear();
}

/*!
    \internal
 */
int QWebSocketHandshakeRequest::port() const
{
    return m_requestUrl.port(m_port);
}

/*!
    \internal
 */
bool QWebSocketHandshakeRequest::isSecure() const
{
    return m_isSecure;
}

/*!
    \internal
 */
bool QWebSocketHandshakeRequest::isValid() const
{
    return m_isValid;
}

/*!
    \internal
 */
QMap<QString, QString> QWebSocketHandshakeRequest::headers() const
{
    return m_headers;
}

/*!
    \internal
 */
QList<QWebSocketProtocol::Version> QWebSocketHandshakeRequest::versions() const
{
    return m_versions;
}

/*!
    \internal
 */
QString QWebSocketHandshakeRequest::resourceName() const
{
    return m_requestUrl.path();
}

/*!
    \internal
 */
QString QWebSocketHandshakeRequest::key() const
{
    return m_key;
}

/*!
    \internal
 */
QString QWebSocketHandshakeRequest::host() const
{
    return m_requestUrl.host();
}

/*!
    \internal
 */
QString QWebSocketHandshakeRequest::origin() const
{
    return m_origin;
}

/*!
    \internal
 */
QList<QString> QWebSocketHandshakeRequest::protocols() const
{
    return m_protocols;
}

/*!
    \internal
 */
QList<QString> QWebSocketHandshakeRequest::extensions() const
{
    return m_extensions;
}

/*!
    \internal
 */
QUrl QWebSocketHandshakeRequest::requestUrl() const
{
    return m_requestUrl;
}

/*!
    \internal
 */
QTextStream &QWebSocketHandshakeRequest::readFromStream(QTextStream &textStream)
{
    m_isValid = false;
    clear();
    if (textStream.status() != QTextStream::Ok) {
        return textStream;
    }
    const QString requestLine = textStream.readLine();
    const QStringList tokens = requestLine.split(' ', QString::SkipEmptyParts);
    if (tokens.length() < 3) {
        m_isValid = false;
        clear();
        return textStream;
    }
    const QString verb(tokens.at(0));
    const QString resourceName(tokens.at(1));
    const QString httpProtocol(tokens.at(2));
    bool conversionOk = false;
    const float httpVersion = httpProtocol.midRef(5).toFloat(&conversionOk);

    if (!conversionOk) {
        clear();
        m_isValid = false;
        return textStream;
    }
    QString headerLine = textStream.readLine();
    m_headers.clear();
    while (!headerLine.isEmpty()) {
        const QStringList headerField = headerLine.split(QStringLiteral(": "), QString::SkipEmptyParts);
        if (headerField.length() < 2) {
            clear();
            return textStream;
        }
        m_headers.insertMulti(headerField.at(0), headerField.at(1));
        headerLine = textStream.readLine();
    }

    const QString host = m_headers.value(QStringLiteral("Host"), QStringLiteral(""));
    m_requestUrl = QUrl::fromEncoded(resourceName.toLatin1());
    if (m_requestUrl.isRelative()) {
        m_requestUrl.setHost(host);
    }
    if (m_requestUrl.scheme().isEmpty()) {
        const QString scheme =  isSecure() ? QStringLiteral("wss") : QStringLiteral("ws");
        m_requestUrl.setScheme(scheme);
    }

    const QStringList versionLines = m_headers.values(QStringLiteral("Sec-WebSocket-Version"));
    for (QStringList::const_iterator v = versionLines.begin(); v != versionLines.end(); ++v) {
        const QStringList versions = (*v).split(QStringLiteral(","), QString::SkipEmptyParts);
        for (QStringList::const_iterator i = versions.begin(); i != versions.end(); ++i) {
            bool ok = false;
            (void)(*i).toUInt(&ok);
            if (!ok) {
                clear();
                return textStream;
            }
            const QWebSocketProtocol::Version ver = QWebSocketProtocol::versionFromString((*i).trimmed());
            m_versions << ver;
        }
    }
    //sort in descending order
    std::sort(m_versions.begin(), m_versions.end(), std::greater<QWebSocketProtocol::Version>());
    m_key = m_headers.value(QStringLiteral("Sec-WebSocket-Key"), QStringLiteral(""));
    //must contain "Upgrade", case-insensitive
    const QString upgrade = m_headers.value(QStringLiteral("Upgrade"), QStringLiteral(""));
    //must be equal to "websocket", case-insensitive
    const QString connection = m_headers.value(QStringLiteral("Connection"), QStringLiteral(""));
    const QStringList connectionLine = connection.split(QStringLiteral(","), QString::SkipEmptyParts);
    QStringList connectionValues;
    for (QStringList::const_iterator c = connectionLine.begin(); c != connectionLine.end(); ++c) {
        connectionValues << (*c).trimmed();
    }

    //optional headers
    m_origin = m_headers.value(QStringLiteral("Sec-WebSocket-Origin"), QStringLiteral(""));
    const QStringList protocolLines = m_headers.values(QStringLiteral("Sec-WebSocket-Protocol"));
    for (QStringList::const_iterator pl = protocolLines.begin(); pl != protocolLines.end(); ++pl) {
        QStringList protocols = (*pl).split(QStringLiteral(","), QString::SkipEmptyParts);
        for (QStringList::const_iterator p = protocols.begin(); p != protocols.end(); ++p) {
            m_protocols << (*p).trimmed();
        }
    }
    const QStringList extensionLines = m_headers.values(QStringLiteral("Sec-WebSocket-Extensions"));
    for (QStringList::const_iterator el = extensionLines.begin(); el != extensionLines.end(); ++el) {
        QStringList extensions = (*el).split(QStringLiteral(","), QString::SkipEmptyParts);
        for (QStringList::const_iterator e = extensions.begin(); e != extensions.end(); ++e) {
            m_extensions << (*e).trimmed();
        }
    }

    //TODO: authentication field

    m_isValid = !(host.isEmpty() ||
                  resourceName.isEmpty() ||
                  m_versions.isEmpty() ||
                  m_key.isEmpty() ||
                  (verb != QStringLiteral("GET")) ||
                  (!conversionOk || (httpVersion < 1.1f)) ||
                  (upgrade.toLower() != QStringLiteral("websocket")) ||
                  (!connectionValues.contains(QStringLiteral("upgrade"), Qt::CaseInsensitive)));
    if (!m_isValid) {
        clear();
    }
    return textStream;
}

/*!
    \internal
 */
QTextStream &operator >>(QTextStream &stream, QWebSocketHandshakeRequest &request)
{
    return request.readFromStream(stream);
}

QT_END_NAMESPACE
