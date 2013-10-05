/*
QWebSockets implements the WebSocket protocol as defined in RFC 6455.
Copyright (C) 2013 Kurt Pattyn (pattyn.kurt@gmail.com)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "qwebsockethandshakerequest_p.h"
#include <QString>
#include <QMap>
#include <QTextStream>
#include <QUrl>
#include <QList>
#include <QStringList>
#include "qwebsocketprotocol.h"

QT_BEGIN_NAMESPACE

/*!
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
    \internal
 */
void QWebSocketHandshakeRequest::clear()
{
    m_port = -1;
    m_isSecure = false;
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
int QWebSocketHandshakeRequest::getPort() const
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
QMap<QString, QString> QWebSocketHandshakeRequest::getHeaders() const
{
    return m_headers;
}

/*!
    \internal
 */
QList<QWebSocketProtocol::Version> QWebSocketHandshakeRequest::getVersions() const
{
    return m_versions;
}

/*!
    \internal
 */
QString QWebSocketHandshakeRequest::getResourceName() const
{
    return m_requestUrl.path();
}

/*!
    \internal
 */
QString QWebSocketHandshakeRequest::getKey() const
{
    return m_key;
}

/*!
    \internal
 */
QString QWebSocketHandshakeRequest::getHost() const
{
    return m_requestUrl.host();
}

/*!
    \internal
 */
QString QWebSocketHandshakeRequest::getOrigin() const
{
    return m_origin;
}

/*!
    \internal
 */
QList<QString> QWebSocketHandshakeRequest::getProtocols() const
{
    return m_protocols;
}

/*!
    \internal
 */
QList<QString> QWebSocketHandshakeRequest::getExtensions() const
{
    return m_extensions;
}

/*!
    \internal
 */
QUrl QWebSocketHandshakeRequest::getRequestUrl() const
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
    if (textStream.status() == QTextStream::Ok)
    {
        const QString requestLine = textStream.readLine();
        const QStringList tokens = requestLine.split(' ', QString::SkipEmptyParts);
        const QString verb = tokens[0];
        const QString resourceName = tokens[1];
        const QString httpProtocol = tokens[2];
        bool conversionOk = false;
        const float httpVersion = httpProtocol.midRef(5).toFloat(&conversionOk);

        QString headerLine = textStream.readLine();
        m_headers.clear();
        while (!headerLine.isEmpty())
        {
            const QStringList headerField = headerLine.split(QStringLiteral(": "), QString::SkipEmptyParts);
            m_headers.insertMulti(headerField[0], headerField[1]);
            headerLine = textStream.readLine();
        }

        const QString host = m_headers.value(QStringLiteral("Host"), QStringLiteral(""));
        m_requestUrl = QUrl::fromEncoded(resourceName.toLatin1());
        if (m_requestUrl.isRelative())
        {
            m_requestUrl.setHost(host);
        }
        if (m_requestUrl.scheme().isEmpty())
        {
            const QString scheme =  isSecure() ? QStringLiteral("wss://") : QStringLiteral("ws://");
            m_requestUrl.setScheme(scheme);
        }

        const QStringList versionLines = m_headers.values(QStringLiteral("Sec-WebSocket-Version"));
        for (QStringList::const_iterator v = versionLines.begin(); v != versionLines.end(); ++v)
        {
            const QStringList versions = (*v).split(QStringLiteral(","), QString::SkipEmptyParts);
            for (QStringList::const_iterator i = versions.begin(); i != versions.end(); ++i)
            {
               const QWebSocketProtocol::Version ver = QWebSocketProtocol::versionFromString((*i).trimmed());
               m_versions << ver;
            }
        }
        std::sort(m_versions.begin(), m_versions.end(), qGreater<QWebSocketProtocol::Version>()); //sort in descending order
        m_key = m_headers.value(QStringLiteral("Sec-WebSocket-Key"), QStringLiteral(""));
        const QString upgrade = m_headers.value(QStringLiteral("Upgrade"), QStringLiteral(""));           //must be equal to "websocket", case-insensitive
        const QString connection = m_headers.value(QStringLiteral("Connection"), QStringLiteral(""));     //must contain "Upgrade", case-insensitive
        const QStringList connectionLine = connection.split(QStringLiteral(","), QString::SkipEmptyParts);
        QStringList connectionValues;
        for (QStringList::const_iterator c = connectionLine.begin(); c != connectionLine.end(); ++c)
        {
            connectionValues << (*c).trimmed();
        }

        //optional headers
        m_origin = m_headers.value(QStringLiteral("Sec-WebSocket-Origin"), QStringLiteral(""));
        const QStringList protocolLines = m_headers.values(QStringLiteral("Sec-WebSocket-Protocol"));
        for (QStringList::const_iterator pl = protocolLines.begin(); pl != protocolLines.end(); ++pl)
        {
            QStringList protocols = (*pl).split(QStringLiteral(","), QString::SkipEmptyParts);
            for (QStringList::const_iterator p = protocols.begin(); p != protocols.end(); ++p)
            {
                m_protocols << (*p).trimmed();
            }
        }
        const QStringList extensionLines = m_headers.values(QStringLiteral("Sec-WebSocket-Extensions"));
        for (QStringList::const_iterator el = extensionLines.begin(); el != extensionLines.end(); ++el)
        {
            QStringList extensions = (*el).split(QStringLiteral(","), QString::SkipEmptyParts);
            for (QStringList::const_iterator e = extensions.begin(); e != extensions.end(); ++e)
            {
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
