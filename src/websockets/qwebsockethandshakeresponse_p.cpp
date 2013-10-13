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

#include "qwebsockethandshakeresponse_p.h"
#include "qwebsockethandshakerequest_p.h"
#include <QString>
#include <QTextStream>
#include <QByteArray>
#include <QStringList>
#include <QDateTime>
#include <QCryptographicHash>
#include <QSet>
#include <QList>
#include <QStringBuilder>   //for more efficient string concatenation

QT_BEGIN_NAMESPACE

/*!
    \internal
 */
QWebSocketHandshakeResponse::QWebSocketHandshakeResponse(const QWebSocketHandshakeRequest &request,
                                                         const QString &serverName,
                                                         bool isOriginAllowed,
                                                         const QList<QWebSocketProtocol::Version> &supportedVersions,
                                                         const QList<QString> &supportedProtocols,
                                                         const QList<QString> &supportedExtensions) :
    m_isValid(false),
    m_canUpgrade(false),
    m_response(),
    m_acceptedProtocol(),
    m_acceptedExtension(),
    m_acceptedVersion(QWebSocketProtocol::V_Unknow)
{
    m_response = getHandshakeResponse(request, serverName, isOriginAllowed, supportedVersions, supportedProtocols, supportedExtensions);
    m_isValid = true;
}

/*!
    \internal
 */
QWebSocketHandshakeResponse::~QWebSocketHandshakeResponse()
{
}

/*!
    \internal
 */
bool QWebSocketHandshakeResponse::isValid() const
{
    return m_isValid;
}

/*!
    \internal
 */
bool QWebSocketHandshakeResponse::canUpgrade() const
{
    return m_isValid && m_canUpgrade;
}

/*!
    \internal
 */
QString QWebSocketHandshakeResponse::acceptedProtocol() const
{
    return m_acceptedProtocol;
}

/*!
    \internal
 */
QString QWebSocketHandshakeResponse::calculateAcceptKey(const QString &key) const
{
    const QString tmpKey = key % QStringLiteral("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");    //the UID comes from RFC6455
    const QByteArray hash = QCryptographicHash::hash(tmpKey.toLatin1(), QCryptographicHash::Sha1);
    return QString::fromLatin1(hash.toBase64());
}

/*!
    \internal
 */
QString QWebSocketHandshakeResponse::getHandshakeResponse(const QWebSocketHandshakeRequest &request,
                                                          const QString &serverName,
                                                          bool isOriginAllowed,
                                                          const QList<QWebSocketProtocol::Version> &supportedVersions,
                                                          const QList<QString> &supportedProtocols,
                                                          const QList<QString> &supportedExtensions)
{
    QStringList response;
    m_canUpgrade = false;

    if (!isOriginAllowed)
    {
        if (!m_canUpgrade)
        {
            response << QStringLiteral("HTTP/1.1 403 Access Forbidden");
        }
    }
    else
    {
        if (request.isValid())
        {
            const QString acceptKey = calculateAcceptKey(request.key());
            const QList<QString> matchingProtocols = supportedProtocols.toSet().intersect(request.protocols().toSet()).toList();
            const QList<QString> matchingExtensions = supportedExtensions.toSet().intersect(request.extensions().toSet()).toList();
            QList<QWebSocketProtocol::Version> matchingVersions = request.versions().toSet().intersect(supportedVersions.toSet()).toList();
            std::sort(matchingVersions.begin(), matchingVersions.end(), qGreater<QWebSocketProtocol::Version>());    //sort in descending order

            if (matchingVersions.isEmpty())
            {
                m_canUpgrade = false;
            }
            else
            {
                response << QStringLiteral("HTTP/1.1 101 Switching Protocols") <<
                            QStringLiteral("Upgrade: websocket") <<
                            QStringLiteral("Connection: Upgrade") <<
                            QStringLiteral("Sec-WebSocket-Accept: ") % acceptKey;
                if (!matchingProtocols.isEmpty())
                {
                    m_acceptedProtocol = matchingProtocols.first();
                    response << QStringLiteral("Sec-WebSocket-Protocol: ") % m_acceptedProtocol;
                }
                if (!matchingExtensions.isEmpty())
                {
                    m_acceptedExtension = matchingExtensions.first();
                    response << QStringLiteral("Sec-WebSocket-Extensions: ") % m_acceptedExtension;
                }
                QString origin = request.origin().trimmed();
                if (origin.isEmpty())
                {
                    origin = QStringLiteral("*");
                }
                response << QStringLiteral("Server: ") % serverName    <<
                            QStringLiteral("Access-Control-Allow-Credentials: false")       <<    //do not allow credentialed request (containing cookies)
                            QStringLiteral("Access-Control-Allow-Methods: GET")             <<    //only GET is allowed during handshaking
                            QStringLiteral("Access-Control-Allow-Headers: content-type")    <<    //this is OK; only the content-type header is allowed, no other headers are accepted
                            QStringLiteral("Access-Control-Allow-Origin: ") % origin    <<
                            QStringLiteral("Date: ") % QDateTime::currentDateTimeUtc().toString(QStringLiteral("ddd, dd MMM yyyy hh:mm:ss 'GMT'"));

                m_acceptedVersion = QWebSocketProtocol::currentVersion();
                m_canUpgrade = true;
            }
        }
        else
        {
            m_canUpgrade = false;
        }
        if (!m_canUpgrade)
        {
            response << QStringLiteral("HTTP/1.1 400 Bad Request");
            QStringList versions;
            Q_FOREACH(QWebSocketProtocol::Version version, supportedVersions)
            {
                versions << QString::number(static_cast<int>(version));
            }
            response << QStringLiteral("Sec-WebSocket-Version: ") % versions.join(QStringLiteral(", "));
        }
    }
    response << QStringLiteral("\r\n");    //append empty line at end of header
    return response.join(QStringLiteral("\r\n"));
}

/*!
    \internal
 */
QTextStream &QWebSocketHandshakeResponse::writeToStream(QTextStream &textStream) const
{
    if (!m_response.isEmpty())
    {
        textStream << m_response.toLatin1().constData();
    }
    else
    {
        textStream.setStatus(QTextStream::WriteFailed);
    }
    return textStream;
}

/*!
    \internal
 */
QTextStream &operator <<(QTextStream &stream, const QWebSocketHandshakeResponse &response)
{
    return response.writeToStream(stream);
}

/*!
    \internal
 */
QWebSocketProtocol::Version QWebSocketHandshakeResponse::acceptedVersion() const
{
    return m_acceptedVersion;
}

/*!
    \internal
 */
QString QWebSocketHandshakeResponse::acceptedExtension() const
{
    return m_acceptedExtension;
}

QT_END_NAMESPACE
