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

#include "qwebsocket.h"
#include "qwebsocket_p.h"
#include "qwebsockethandshakerequest_p.h"
#include "qwebsockethandshakeresponse_p.h"
#include <QUrl>
#include <QTcpSocket>
#include <QByteArray>
#include <QtEndian>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QStringList>
#include <QHostAddress>
#include <QStringBuilder>   //for more efficient string concatenation
#ifndef QT_NONETWORKPROXY
#include <QNetworkProxy>
#endif

#include <QDebug>

#include <limits>

QT_BEGIN_NAMESPACE

const quint64 FRAME_SIZE_IN_BYTES = 512 * 512 * 2;	//maximum size of a frame when sending a message

/*!
    \internal
*/
QWebSocketPrivate::QWebSocketPrivate(const QString &origin, QWebSocketProtocol::Version version, QWebSocket *pWebSocket, QObject *parent) :
    QObject(parent),
    q_ptr(pWebSocket),
    m_pSocket(Q_NULLPTR),
    m_errorString(),
    m_version(version),
    m_resourceName(),
    m_requestUrl(),
    m_origin(origin),
    m_protocol(),
    m_extension(),
    m_socketState(QAbstractSocket::UnconnectedState),
    m_key(),
    m_mustMask(true),
    m_isClosingHandshakeSent(false),
    m_isClosingHandshakeReceived(false),
    m_pingTimer(),
    m_dataProcessor()
{
    Q_ASSERT(pWebSocket);
    //makeConnections(m_pSocket);
    qsrand(static_cast<uint>(QDateTime::currentMSecsSinceEpoch()));
}

/*!
    \internal
*/
QWebSocketPrivate::QWebSocketPrivate(QTcpSocket *pTcpSocket, QWebSocketProtocol::Version version, QWebSocket *pWebSocket, QObject *parent) :
    QObject(parent),
    q_ptr(pWebSocket),
    m_pSocket(pTcpSocket),
    m_errorString(pTcpSocket->errorString()),
    m_version(version),
    m_resourceName(),
    m_requestUrl(),
    m_origin(),
    m_protocol(),
    m_extension(),
    m_socketState(pTcpSocket->state()),
    m_key(),
    m_mustMask(true),
    m_isClosingHandshakeSent(false),
    m_isClosingHandshakeReceived(false),
    m_pingTimer(),
    m_dataProcessor()
{
    Q_ASSERT(pWebSocket);
    makeConnections(m_pSocket);
}

/*!
    \internal
*/
QWebSocketPrivate::~QWebSocketPrivate()
{
    if (state() == QAbstractSocket::ConnectedState)
    {
        close(QWebSocketProtocol::CC_GOING_AWAY, tr("Connection closed"));
    }
    releaseConnections(m_pSocket);
    m_pSocket->deleteLater();
    m_pSocket = Q_NULLPTR;
}

/*!
    \internal
 */
void QWebSocketPrivate::abort()
{
    m_pSocket->abort();
}

/*!
    \internal
 */
QAbstractSocket::SocketError QWebSocketPrivate::error() const
{
    return m_pSocket->error();
}

/*!
    \internal
 */
QString QWebSocketPrivate::errorString() const
{
    if (!m_errorString.isEmpty())
    {
        return m_errorString;
    }
    else
    {
        return m_pSocket->errorString();
    }
}

/*!
    \internal
 */
bool QWebSocketPrivate::flush()
{
    return m_pSocket->flush();
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::write(const char *message)
{
    //TODO: create a QByteArray from message, and directly call doWriteData
    //now the data is converted to a string, and then converted back to a bytearray
    return write(QString::fromUtf8(message));
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::write(const char *message, qint64 maxSize)
{
    //TODO: create a QByteArray from message, and directly call doWriteData
    //now the data is converted to a string, and then converted back to a bytearray
    return write(QString::fromUtf8(message, static_cast<int>(maxSize)));
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::write(const QString &message)
{
    return doWriteData(message.toUtf8(), false);
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::write(const QByteArray &data)
{
    return doWriteData(data, true);
}

#ifndef QT_NO_SSL
/*!
    \internal
 */
void QWebSocketPrivate::setSslConfiguration(const QSslConfiguration &sslConfiguration)
{
    m_sslConfiguration = sslConfiguration;
}

/*!
    \internal
 */
QSslConfiguration QWebSocketPrivate::sslConfiguration() const
{
    return m_sslConfiguration;
}

/*!
    \internal
 */
void QWebSocketPrivate::ignoreSslErrors(const QList<QSslError> &errors)
{
    m_ignoredSslErrors = errors;
}

#endif

/*!
  Called from QWebSocketServer
  \internal
 */
QWebSocket *QWebSocketPrivate::upgradeFrom(QTcpSocket *pTcpSocket,
                                           const QWebSocketHandshakeRequest &request,
                                           const QWebSocketHandshakeResponse &response,
                                           QObject *parent)
{
    QWebSocket *pWebSocket = new QWebSocket(pTcpSocket, response.acceptedVersion(), parent);
    pWebSocket->d_func()->setExtension(response.acceptedExtension());
    pWebSocket->d_func()->setOrigin(request.origin());
    pWebSocket->d_func()->setRequestUrl(request.requestUrl());
    pWebSocket->d_func()->setProtocol(response.acceptedProtocol());
    pWebSocket->d_func()->setResourceName(request.requestUrl().toString(QUrl::RemoveUserInfo));
    //a server should not send masked frames
    pWebSocket->d_func()->enableMasking(false);

    return pWebSocket;
}

/*!
    \internal
 */
void QWebSocketPrivate::close(QWebSocketProtocol::CloseCode closeCode, QString reason)
{
    Q_Q(QWebSocket);
    if (!m_isClosingHandshakeSent)
    {
        quint32 maskingKey = 0;
        if (m_mustMask)
        {
            maskingKey = generateMaskingKey();
        }
        const quint16 code = qToBigEndian<quint16>(closeCode);
        QByteArray payload;
        payload.append(static_cast<const char *>(static_cast<const void *>(&code)), 2);
        if (!reason.isEmpty())
        {
            payload.append(reason.toUtf8());
        }
        if (m_mustMask)
        {
            QWebSocketProtocol::mask(payload.data(), payload.size(), maskingKey);
        }
        QByteArray frame = getFrameHeader(QWebSocketProtocol::OC_CLOSE, payload.size(), maskingKey, true);
        frame.append(payload);
        m_pSocket->write(frame);
        m_pSocket->flush();

        m_isClosingHandshakeSent = true;

        Q_EMIT q->aboutToClose();
    }
    m_pSocket->close();
}

/*!
    \internal
 */
void QWebSocketPrivate::open(const QUrl &url, bool mask)
{
    Q_Q(QWebSocket);

    m_dataProcessor.clear();
    m_isClosingHandshakeReceived = false;
    m_isClosingHandshakeSent = false;

    setRequestUrl(url);
    QString resourceName = url.path();
    if (!url.query().isEmpty())
    {
        if (!resourceName.endsWith(QChar::fromLatin1('?')))
        {
            resourceName.append(QChar::fromLatin1('?'));
        }
        resourceName.append(url.query());
    }
    if (resourceName.isEmpty())
    {
        resourceName = QStringLiteral("/");
    }
    setResourceName(resourceName);
    enableMasking(mask);

#ifndef QT_NO_SSL
    if (url.scheme() == QStringLiteral("wss"))
    {
        if (!QSslSocket::supportsSsl())
        {
            qWarning() << tr("SSL Sockets are not supported on this platform.");
            setErrorString(tr("SSL Sockets are not supported on this platform."));
            emit q->error(QAbstractSocket::UnsupportedSocketOperationError);
            return;
        }
        else
        {
            QSslSocket *sslSocket = new QSslSocket(this);
            m_pSocket = sslSocket;

            makeConnections(m_pSocket);
            connect(sslSocket, SIGNAL(encryptedBytesWritten(qint64)), q, SIGNAL(bytesWritten(qint64)));
            setSocketState(QAbstractSocket::ConnectingState);

            sslSocket->setSslConfiguration(m_sslConfiguration);
            sslSocket->ignoreSslErrors(m_ignoredSslErrors);
            sslSocket->connectToHostEncrypted(url.host(), url.port(443));
        }
    }
    else
#endif
    {
        m_pSocket = new QTcpSocket(this);

        makeConnections(m_pSocket);
        connect(m_pSocket, SIGNAL(bytesWritten(qint64)), q, SIGNAL(bytesWritten(qint64)));
        setSocketState(QAbstractSocket::ConnectingState);
        m_pSocket->connectToHost(url.host(), url.port(80));
    }
}

/*!
    \internal
 */
void QWebSocketPrivate::ping(const QByteArray &payload)
{
    Q_ASSERT(payload.length() < 126);
    m_pingTimer.restart();
    QByteArray pingFrame = getFrameHeader(QWebSocketProtocol::OC_PING, payload.size(), 0 /*do not mask*/, true);
    pingFrame.append(payload);
    writeFrame(pingFrame);
}

/*!
  \internal
    Sets the version to use for the websocket protocol; this must be set before the socket is opened.
*/
void QWebSocketPrivate::setVersion(QWebSocketProtocol::Version version)
{
    m_version = version;
}

/*!
    \internal
    Sets the resource name of the connection; must be set before the socket is openend
*/
void QWebSocketPrivate::setResourceName(const QString &resourceName)
{
    m_resourceName = resourceName;
}

/*!
  \internal
 */
void QWebSocketPrivate::setRequestUrl(const QUrl &requestUrl)
{
    m_requestUrl = requestUrl;
}

/*!
  \internal
 */
void QWebSocketPrivate::setOrigin(const QString &origin)
{
    m_origin = origin;
}

/*!
  \internal
 */
void QWebSocketPrivate::setProtocol(const QString &protocol)
{
    m_protocol = protocol;
}

/*!
  \internal
 */
void QWebSocketPrivate::setExtension(const QString &extension)
{
    m_extension = extension;
}

/*!
  \internal
 */
void QWebSocketPrivate::enableMasking(bool enable)
{
    m_mustMask = enable;
}

/*!
 * \internal
 */
qint64 QWebSocketPrivate::doWriteData(const QByteArray &data, bool isBinary)
{
    return doWriteFrames(data, isBinary);
}

/*!
 * \internal
 */
void QWebSocketPrivate::makeConnections(const QTcpSocket *pTcpSocket)
{
    Q_Q(QWebSocket);

    //pass through signals
    connect(pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), q, SIGNAL(error(QAbstractSocket::SocketError)));
    connect(pTcpSocket, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), q, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
    connect(pTcpSocket, SIGNAL(readChannelFinished()), q, SIGNAL(readChannelFinished()));
    connect(pTcpSocket, SIGNAL(aboutToClose()), q, SIGNAL(aboutToClose()));
    //connect(pTcpSocket, SIGNAL(bytesWritten(qint64)), q, SIGNAL(bytesWritten(qint64)));

    //catch signals
    connect(pTcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(processStateChanged(QAbstractSocket::SocketState)));
    connect(pTcpSocket, SIGNAL(readyRead()), this, SLOT(processData()));

    connect(&m_dataProcessor, SIGNAL(textFrameReceived(QString,bool)), q, SIGNAL(textFrameReceived(QString,bool)));
    connect(&m_dataProcessor, SIGNAL(binaryFrameReceived(QByteArray,bool)), q, SIGNAL(binaryFrameReceived(QByteArray,bool)));
    connect(&m_dataProcessor, SIGNAL(binaryMessageReceived(QByteArray)), q, SIGNAL(binaryMessageReceived(QByteArray)));
    connect(&m_dataProcessor, SIGNAL(textMessageReceived(QString)), q, SIGNAL(textMessageReceived(QString)));
    connect(&m_dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)), this, SLOT(close(QWebSocketProtocol::CloseCode,QString)));
    connect(&m_dataProcessor, SIGNAL(pingReceived(QByteArray)), this, SLOT(processPing(QByteArray)));
    connect(&m_dataProcessor, SIGNAL(pongReceived(QByteArray)), this, SLOT(processPong(QByteArray)));
    connect(&m_dataProcessor, SIGNAL(closeReceived(QWebSocketProtocol::CloseCode,QString)), this, SLOT(processClose(QWebSocketProtocol::CloseCode,QString)));
}

/*!
 * \internal
 */
void QWebSocketPrivate::releaseConnections(const QTcpSocket *pTcpSocket)
{
    Q_Q(QWebSocket);
    if (pTcpSocket)
    {
        //pass through signals
        disconnect(pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), q, SIGNAL(error(QAbstractSocket::SocketError)));
        disconnect(pTcpSocket, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), q, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
        disconnect(pTcpSocket, SIGNAL(readChannelFinished()), q, SIGNAL(readChannelFinished()));
        disconnect(pTcpSocket, SIGNAL(aboutToClose()), q, SIGNAL(aboutToClose()));
        //disconnect(pTcpSocket, SIGNAL(bytesWritten(qint64)), q, SIGNAL(bytesWritten(qint64)));

        //catched signals
        disconnect(pTcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(processStateChanged(QAbstractSocket::SocketState)));
        disconnect(pTcpSocket, SIGNAL(readyRead()), this, SLOT(processData()));
    }
    disconnect(&m_dataProcessor, SIGNAL(pingReceived(QByteArray)), this, SLOT(processPing(QByteArray)));
    disconnect(&m_dataProcessor, SIGNAL(pongReceived(QByteArray)), this, SLOT(processPong(QByteArray)));
    disconnect(&m_dataProcessor, SIGNAL(closeReceived(QWebSocketProtocol::CloseCode,QString)), this, SLOT(processClose(QWebSocketProtocol::CloseCode,QString)));
    disconnect(&m_dataProcessor, SIGNAL(textFrameReceived(QString,bool)), q, SIGNAL(textFrameReceived(QString,bool)));
    disconnect(&m_dataProcessor, SIGNAL(binaryFrameReceived(QByteArray,bool)), q, SIGNAL(binaryFrameReceived(QByteArray,bool)));
    disconnect(&m_dataProcessor, SIGNAL(binaryMessageReceived(QByteArray)), q, SIGNAL(binaryMessageReceived(QByteArray)));
    disconnect(&m_dataProcessor, SIGNAL(textMessageReceived(QString)), q, SIGNAL(textMessageReceived(QString)));
    disconnect(&m_dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)), this, SLOT(close(QWebSocketProtocol::CloseCode,QString)));
}

/*!
    \internal
 */
QWebSocketProtocol::Version QWebSocketPrivate::version() const
{
    return m_version;
}

/*!
    \internal
 */
QString QWebSocketPrivate::resourceName() const
{
    return m_resourceName;
}

/*!
    \internal
 */
QUrl QWebSocketPrivate::requestUrl() const
{
    return m_requestUrl;
}

/*!
    \internal
 */
QString QWebSocketPrivate::origin() const
{
    return m_origin;
}

/*!
    \internal
 */
QString QWebSocketPrivate::protocol() const
{
    return m_protocol;
}

/*!
    \internal
 */
QString QWebSocketPrivate::extension() const
{
    return m_extension;
}

/*!
 * \internal
 */
QByteArray QWebSocketPrivate::getFrameHeader(QWebSocketProtocol::OpCode opCode, quint64 payloadLength, quint32 maskingKey, bool lastFrame) const
{
    QByteArray header;
    quint8 byte = 0x00;
    bool ok = payloadLength <= 0x7FFFFFFFFFFFFFFFULL;

    if (ok)
    {
        //FIN, RSV1-3, opcode
        byte = static_cast<quint8>((opCode & 0x0F) | (lastFrame ? 0x80 : 0x00));	//FIN, opcode
        //RSV-1, RSV-2 and RSV-3 are zero
        header.append(static_cast<char>(byte));

        //Now write the masking bit and the payload length byte
        byte = 0x00;
        if (maskingKey != 0)
        {
            byte |= 0x80;
        }
        if (payloadLength <= 125)
        {
            byte |= static_cast<quint8>(payloadLength);
            header.append(static_cast<char>(byte));
        }
        else if (payloadLength <= 0xFFFFU)
        {
            byte |= 126;
            header.append(static_cast<char>(byte));
            quint16 swapped = qToBigEndian<quint16>(static_cast<quint16>(payloadLength));
            header.append(static_cast<const char *>(static_cast<const void *>(&swapped)), 2);
        }
        else if (payloadLength <= 0x7FFFFFFFFFFFFFFFULL)
        {
            byte |= 127;
            header.append(static_cast<char>(byte));
            quint64 swapped = qToBigEndian<quint64>(payloadLength);
            header.append(static_cast<const char *>(static_cast<const void *>(&swapped)), 8);
        }

        //Write mask
        if (maskingKey != 0)
        {
            header.append(static_cast<const char *>(static_cast<const void *>(&maskingKey)), sizeof(quint32));
        }
    }
    else
    {
        //setErrorString("WebSocket::getHeader: payload too big!");
        //Q_EMIT q_ptr->error(QAbstractSocket::DatagramTooLargeError);
        qDebug() << "WebSocket::getHeader: payload too big!";
    }

    return header;
}

/*!
 * \internal
 */
qint64 QWebSocketPrivate::doWriteFrames(const QByteArray &data, bool isBinary)
{
    Q_Q(QWebSocket);
    const QWebSocketProtocol::OpCode firstOpCode = isBinary ? QWebSocketProtocol::OC_BINARY : QWebSocketProtocol::OC_TEXT;

    int numFrames = data.size() / FRAME_SIZE_IN_BYTES;
    QByteArray tmpData(data);
    tmpData.detach();
    char *payload = tmpData.data();
    quint64 sizeLeft = static_cast<quint64>(data.size()) % FRAME_SIZE_IN_BYTES;
    if (sizeLeft)
    {
        ++numFrames;
    }
    if (numFrames == 0)     //catch the case where the payload is zero bytes; in that case, we still need to send a frame
    {
        numFrames = 1;
    }
    quint64 currentPosition = 0;
    qint64 bytesWritten = 0;
    qint64 payloadWritten = 0;
    quint64 bytesLeft = data.size();

    for (int i = 0; i < numFrames; ++i)
    {
        quint32 maskingKey = 0;
        if (m_mustMask)
        {
            maskingKey = generateMaskingKey();
        }

        const bool isLastFrame = (i == (numFrames - 1));
        const bool isFirstFrame = (i == 0);

        const quint64 size = qMin(bytesLeft, FRAME_SIZE_IN_BYTES);
        const QWebSocketProtocol::OpCode opcode = isFirstFrame ? firstOpCode : QWebSocketProtocol::OC_CONTINUE;

        //write header
        bytesWritten += m_pSocket->write(getFrameHeader(opcode, size, maskingKey, isLastFrame));

        //write payload
        if (size > 0)
        {
            char *currentData = payload + currentPosition;
            if (m_mustMask)
            {
                QWebSocketProtocol::mask(currentData, size, maskingKey);
            }
            qint64 written = m_pSocket->write(currentData, static_cast<qint64>(size));
            if (written > 0)
            {
                bytesWritten += written;
                payloadWritten += written;
            }
            else
            {
                setErrorString(tr("Error writing bytes to socket: %1.").arg(m_pSocket->errorString()));
                qDebug() << errorString();
                m_pSocket->flush();
                Q_EMIT q->error(QAbstractSocket::NetworkError);
                break;
            }
        }
        currentPosition += size;
        bytesLeft -= size;
    }
    if (payloadWritten != data.size())
    {
        setErrorString(tr("Bytes written %1 != %2.").arg(payloadWritten).arg(data.size()));
        qDebug() << errorString();
        Q_EMIT q->error(QAbstractSocket::NetworkError);
    }
    return payloadWritten;
}

/*!
 * \internal
 */
quint32 QWebSocketPrivate::generateRandomNumber() const
{
    return static_cast<quint32>((static_cast<double>(qrand()) / RAND_MAX) * std::numeric_limits<quint32>::max());
}

/*!
    \internal
 */
quint32 QWebSocketPrivate::generateMaskingKey() const
{
    return generateRandomNumber();
}

/*!
    \internal
 */
QByteArray QWebSocketPrivate::generateKey() const
{
    QByteArray key;

    for (int i = 0; i < 4; ++i)
    {
        quint32 tmp = generateRandomNumber();
        key.append(static_cast<const char *>(static_cast<const void *>(&tmp)), sizeof(quint32));
    }

    return key.toBase64();
}


/*!
    \internal
 */
QString QWebSocketPrivate::calculateAcceptKey(const QString &key) const
{
    const QString tmpKey = key % QStringLiteral("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    const QByteArray hash = QCryptographicHash::hash(tmpKey.toLatin1(), QCryptographicHash::Sha1);
    return QString::fromLatin1(hash.toBase64());
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::writeFrames(const QList<QByteArray> &frames)
{
    qint64 written = 0;
    for (int i = 0; i < frames.size(); ++i)
    {
        written += writeFrame(frames[i]);
    }
    return written;
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::writeFrame(const QByteArray &frame)
{
    return m_pSocket->write(frame);
}

/*!
    \internal
 */
QString readLine(QTcpSocket *pSocket)
{
    QString line;
    char c;
    while (pSocket->getChar(&c))
    {
        if (c == '\r')
        {
            pSocket->getChar(&c);
            break;
        }
        else
        {
            line.append(QChar::fromLatin1(c));
        }
    }
    return line;
}

//called on the client for a server handshake response
/*!
    \internal
 */
void QWebSocketPrivate::processHandshake(QTcpSocket *pSocket)
{
    Q_Q(QWebSocket);
    if (!pSocket)
    {
        return;
    }

    bool ok = false;
    QString errorDescription;

    const QString regExpStatusLine(QStringLiteral("^(HTTP/[0-9]+\\.[0-9]+)\\s([0-9]+)\\s(.*)"));
    const QRegularExpression regExp(regExpStatusLine);
    const QString statusLine = readLine(pSocket);
    QString httpProtocol;
    int httpStatusCode;
    QString httpStatusMessage;
    const QRegularExpressionMatch match = regExp.match(statusLine);
    if (match.hasMatch())
    {
        QStringList tokens = match.capturedTexts();
        tokens.removeFirst();	//remove the search string
        if (tokens.length() == 3)
        {
            httpProtocol = tokens[0];
            httpStatusCode = tokens[1].toInt();
            httpStatusMessage = tokens[2].trimmed();
            ok = true;
        }
    }
    if (!ok)
    {
        errorDescription = tr("Invalid statusline in response: %1.").arg(statusLine);
    }
    else
    {
        QString headerLine = readLine(pSocket);
        QMap<QString, QString> headers;
        while (!headerLine.isEmpty())
        {
            const QStringList headerField = headerLine.split(QStringLiteral(": "), QString::SkipEmptyParts);
            headers.insertMulti(headerField[0], headerField[1]);
            headerLine = readLine(pSocket);
        }

        const QString acceptKey = headers.value(QStringLiteral("Sec-WebSocket-Accept"), QStringLiteral(""));
        const QString upgrade = headers.value(QStringLiteral("Upgrade"), QStringLiteral(""));
        const QString connection = headers.value(QStringLiteral("Connection"), QStringLiteral(""));
        //unused for the moment
        //const QString extensions = headers.value(QStringLiteral("Sec-WebSocket-Extensions"), QStringLiteral(""));
        //const QString protocol = headers.value(QStringLiteral("Sec-WebSocket-Protocol"), QStringLiteral(""));
        const QString version = headers.value(QStringLiteral("Sec-WebSocket-Version"), QStringLiteral(""));

        if (httpStatusCode == 101)	//HTTP/x.y 101 Switching Protocols
        {
            bool conversionOk = false;
            const float version = httpProtocol.midRef(5).toFloat(&conversionOk);
            //TODO: do not check the httpStatusText right now
            ok = !(acceptKey.isEmpty() ||
                   (!conversionOk || (version < 1.1f)) ||
                   (upgrade.toLower() != QStringLiteral("websocket")) ||
                   (connection.toLower() != QStringLiteral("upgrade")));
            if (ok)
            {
                const QString accept = calculateAcceptKey(QString::fromLatin1(m_key));
                ok = (accept == acceptKey);
                if (!ok)
                {
                    errorDescription = tr("Accept-Key received from server %1 does not match the client key %2.").arg(acceptKey).arg(accept);
                }
            }
            else
            {
                errorDescription = tr("QWebSocketPrivate::processHandshake: Invalid statusline in response: %1.").arg(statusLine);
            }
        }
        else if (httpStatusCode == 400)	//HTTP/1.1 400 Bad Request
        {
            if (!version.isEmpty())
            {
                const QStringList versions = version.split(QStringLiteral(", "), QString::SkipEmptyParts);
                if (!versions.contains(QString::number(QWebSocketProtocol::currentVersion())))
                {
                    //if needed to switch protocol version, then we are finished here
                    //because we cannot handle other protocols than the RFC one (v13)
                    errorDescription = tr("Handshake: Server requests a version that we don't support: %1.").arg(versions.join(QStringLiteral(", ")));
                    ok = false;
                }
                else
                {
                    //we tried v13, but something different went wrong
                    errorDescription = tr("QWebSocketPrivate::processHandshake: Unknown error condition encountered. Aborting connection.");
                    ok = false;
                }
            }
        }
        else
        {
            errorDescription = tr("QWebSocketPrivate::processHandshake: Unhandled http status code: %1 (%2).").arg(httpStatusCode).arg(httpStatusMessage);
            ok = false;
        }

        if (!ok)
        {
            qDebug() << errorDescription;
            setErrorString(errorDescription);
            Q_EMIT q->error(QAbstractSocket::ConnectionRefusedError);
        }
        else
        {
            //handshake succeeded
            setSocketState(QAbstractSocket::ConnectedState);
            Q_EMIT q->connected();
        }
    }
}

/*!
    \internal
 */
void QWebSocketPrivate::processStateChanged(QAbstractSocket::SocketState socketState)
{
    Q_Q(QWebSocket);
    QAbstractSocket::SocketState webSocketState = this->state();
    switch (socketState)
    {
    case QAbstractSocket::ConnectedState:
    {
        if (webSocketState == QAbstractSocket::ConnectingState)
        {
            m_key = generateKey();
            QString handshake = createHandShakeRequest(m_resourceName, m_requestUrl.host() % QStringLiteral(":") % QString::number(m_requestUrl.port(80)), origin(), QStringLiteral(""), QStringLiteral(""), m_key);
            m_pSocket->write(handshake.toLatin1());
        }
        break;
    }
    case QAbstractSocket::ClosingState:
    {
        if (webSocketState == QAbstractSocket::ConnectedState)
        {
            setSocketState(QAbstractSocket::ClosingState);
        }
        break;
    }
    case QAbstractSocket::UnconnectedState:
    {
        if (webSocketState != QAbstractSocket::UnconnectedState)
        {
            setSocketState(QAbstractSocket::UnconnectedState);
            Q_EMIT q->disconnected();
        }
        break;
    }
    case QAbstractSocket::HostLookupState:
    case QAbstractSocket::ConnectingState:
    case QAbstractSocket::BoundState:
    case QAbstractSocket::ListeningState:
    {
        //do nothing
        //to make C++ compiler happy;
        break;
    }
    default:
    {
        break;
    }
    }
}

//order of events:
//connectToHost is called
//our socket state is set to "connecting", and tcpSocket->connectToHost is called
//the tcpsocket is opened, a handshake message is sent; a readyRead signal is thrown
//this signal is catched by processData
//when OUR socket state is in the "connecting state", this means that
//we have received data from the server (response to handshake), and that we
//should "upgrade" our socket to a websocket (connected state)
//if our socket was already upgraded, then we need to process websocket data
/*!
 \internal
 */
void QWebSocketPrivate::processData()
{
    while (m_pSocket->bytesAvailable())
    {
        if (state() == QAbstractSocket::ConnectingState)
        {
            processHandshake(m_pSocket);
        }
        else
        {
            m_dataProcessor.process(m_pSocket);
        }
    }
}

/*!
 \internal
 */
void QWebSocketPrivate::processPing(QByteArray data)
{
    quint32 maskingKey = 0;
    if (m_mustMask)
    {
        maskingKey = generateMaskingKey();
    }
    m_pSocket->write(getFrameHeader(QWebSocketProtocol::OC_PONG, data.size(), maskingKey, true));
    if (data.size() > 0)
    {
        if (m_mustMask)
        {
            QWebSocketProtocol::mask(&data, maskingKey);
        }
        m_pSocket->write(data);
    }
}

/*!
 \internal
 */
void QWebSocketPrivate::processPong(QByteArray data)
{
    Q_Q(QWebSocket);
    Q_EMIT q->pong(static_cast<quint64>(m_pingTimer.elapsed()), data);
}

/*!
 \internal
 */
void QWebSocketPrivate::processClose(QWebSocketProtocol::CloseCode closeCode, QString closeReason)
{
    m_isClosingHandshakeReceived = true;
    close(closeCode, closeReason);
}

/*!
    \internal
 */
QString QWebSocketPrivate::createHandShakeRequest(QString resourceName,
                                                  QString host,
                                                  QString origin,
                                                  QString extensions,
                                                  QString protocols,
                                                  QByteArray key)
{
    QStringList handshakeRequest;

    handshakeRequest << QStringLiteral("GET ") % resourceName % QStringLiteral(" HTTP/1.1") <<
                        QStringLiteral("Host: ") % host <<
                        QStringLiteral("Upgrade: websocket") <<
                        QStringLiteral("Connection: Upgrade") <<
                        QStringLiteral("Sec-WebSocket-Key: ") % QString::fromLatin1(key);
    if (!origin.isEmpty())
    {
        handshakeRequest << QStringLiteral("Origin: ") % origin;
    }
    handshakeRequest << QStringLiteral("Sec-WebSocket-Version: ") % QString::number(QWebSocketProtocol::currentVersion());
    if (extensions.length() > 0)
    {
        handshakeRequest << QStringLiteral("Sec-WebSocket-Extensions: ") % extensions;
    }
    if (protocols.length() > 0)
    {
        handshakeRequest << QStringLiteral("Sec-WebSocket-Protocol: ") % protocols;
    }
    handshakeRequest << QStringLiteral("\r\n");

    return handshakeRequest.join(QStringLiteral("\r\n"));
}

/*!
    \internal
 */
QAbstractSocket::SocketState QWebSocketPrivate::state() const
{
    return m_socketState;
}

/*!
    \internal
 */
bool QWebSocketPrivate::waitForConnected(int msecs)
{
    return m_pSocket->waitForConnected(msecs);
}

/*!
    \internal
 */
bool QWebSocketPrivate::waitForDisconnected(int msecs)
{
    return m_pSocket->waitForDisconnected(msecs);
}

/*!
    \internal
 */
void QWebSocketPrivate::setSocketState(QAbstractSocket::SocketState state)
{
    Q_Q(QWebSocket);
    if (m_socketState != state)
    {
        m_socketState = state;
        Q_EMIT q->stateChanged(m_socketState);
    }
}

/*!
    \internal
 */
void QWebSocketPrivate::setErrorString(const QString &errorString)
{
    m_errorString = errorString;
}

/*!
    \internal
 */
QHostAddress QWebSocketPrivate::localAddress() const
{
    return m_pSocket->localAddress();
}

/*!
    \internal
 */
quint16 QWebSocketPrivate::localPort() const
{
    return m_pSocket->localPort();
}

/*!
    \internal
 */
QAbstractSocket::PauseModes QWebSocketPrivate::pauseMode() const
{
    return m_pSocket->pauseMode();
}

/*!
    \internal
 */
QHostAddress QWebSocketPrivate::peerAddress() const
{
    return m_pSocket->peerAddress();
}

/*!
    \internal
 */
QString QWebSocketPrivate::peerName() const
{
    return m_pSocket->peerName();
}

/*!
    \internal
 */
quint16 QWebSocketPrivate::peerPort() const
{
    return m_pSocket->peerPort();
}

/*!
    \internal
 */
QNetworkProxy QWebSocketPrivate::proxy() const
{
    return m_pSocket->proxy();
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::readBufferSize() const
{
    return m_pSocket->readBufferSize();
}

/*!
    \internal
 */
void QWebSocketPrivate::resume()
{
    m_pSocket->resume();
}

/*!
  \internal
 */
void QWebSocketPrivate::setPauseMode(QAbstractSocket::PauseModes pauseMode)
{
    m_pSocket->setPauseMode(pauseMode);
}

/*!
    \internal
 */
void QWebSocketPrivate::setProxy(const QNetworkProxy &networkProxy)
{
    m_pSocket->setProxy(networkProxy);
}

/*!
    \internal
 */
void QWebSocketPrivate::setReadBufferSize(qint64 size)
{
    m_pSocket->setReadBufferSize(size);
}

/*!
    \internal
 */
void QWebSocketPrivate::setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value)
{
    m_pSocket->setSocketOption(option, value);
}

/*!
    \internal
 */
QVariant QWebSocketPrivate::socketOption(QAbstractSocket::SocketOption option)
{
    return m_pSocket->socketOption(option);
}

/*!
    \internal
 */
bool QWebSocketPrivate::isValid() const
{
    return m_pSocket->isValid();
}

QT_END_NAMESPACE
