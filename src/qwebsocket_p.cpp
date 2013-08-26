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
#include "handshakerequest_p.h"
#include "handshakeresponse_p.h"
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
QWebSocketPrivate::QWebSocketPrivate(QString origin, QWebSocketProtocol::Version version, QWebSocket *pWebSocket, QObject *parent) :
    QObject(parent),
    q_ptr(pWebSocket),
    m_pSocket(new QTcpSocket(this)),
    m_errorString(),
    m_version(version),
    m_resourceName(),
    m_requestUrl(),
    m_origin(origin),
    m_protocol(""),
    m_extension(""),
    m_socketState(QAbstractSocket::UnconnectedState),
    m_key(),
    m_mustMask(true),
    m_isClosingHandshakeSent(false),
    m_isClosingHandshakeReceived(false),
    m_pingTimer(),
    m_dataProcessor()
{
    Q_ASSERT(pWebSocket != 0);
    makeConnections(m_pSocket);
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
    Q_ASSERT(pWebSocket != 0);
    makeConnections(m_pSocket);
}

/*!
    \internal
*/
QWebSocketPrivate::~QWebSocketPrivate()
{
    if (state() == QAbstractSocket::ConnectedState)
    {
        close(QWebSocketProtocol::CC_GOING_AWAY, QWebSocket::tr("Connection closed"));
    }
    releaseConnections(m_pSocket);
    m_pSocket->deleteLater();
    m_pSocket = 0;
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
    return write(QString::fromUtf8(message));
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::write(const char *message, qint64 maxSize)
{
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

/*!
  \internal
 */
QWebSocket *QWebSocketPrivate::upgradeFrom(QTcpSocket *pTcpSocket,
                                           const HandshakeRequest &request,
                                           const HandshakeResponse &response,
                                           QObject *parent)
{
    QWebSocket *pWebSocket = new QWebSocket(pTcpSocket, response.getAcceptedVersion(), parent);
    pWebSocket->d_ptr->setExtension(response.getAcceptedExtension());
    pWebSocket->d_ptr->setOrigin(request.getOrigin());
    pWebSocket->d_ptr->setRequestUrl(request.getRequestUrl());
    pWebSocket->d_ptr->setProtocol(response.getAcceptedProtocol());
    pWebSocket->d_ptr->setResourceName(request.getRequestUrl().toString(QUrl::RemoveUserInfo));
    pWebSocket->d_ptr->enableMasking(false);	//a server should not send masked frames

    return pWebSocket;
}

/*!
    \internal
 */
void QWebSocketPrivate::close(QWebSocketProtocol::CloseCode closeCode, QString reason)
{
    if (!m_isClosingHandshakeSent)
    {
        quint32 maskingKey = 0;
        if (m_mustMask)
        {
            maskingKey = generateMaskingKey();
        }
        quint16 code = qToBigEndian<quint16>(closeCode);
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

        Q_EMIT q_ptr->aboutToClose();
    }
    m_pSocket->close();
}

/*!
    \internal
 */
void QWebSocketPrivate::open(const QUrl &url, bool mask)
{
    m_dataProcessor.clear();
    m_isClosingHandshakeReceived = false;
    m_isClosingHandshakeSent = false;

    setRequestUrl(url);
    QString resourceName = url.path() + url.query();
    if (resourceName.isEmpty())
    {
        resourceName = "/";
    }
    setResourceName(resourceName);
    enableMasking(mask);

    setSocketState(QAbstractSocket::ConnectingState);

    m_pSocket->connectToHost(url.host(), url.port(80));
}

/*!
    \internal
 */
void QWebSocketPrivate::ping()
{
    m_pingTimer.restart();
    QByteArray pingFrame = getFrameHeader(QWebSocketProtocol::OC_PING, 0, 0, true);
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
void QWebSocketPrivate::setResourceName(QString resourceName)
{
    m_resourceName = resourceName;
}

/*!
  \internal
 */
void QWebSocketPrivate::setRequestUrl(QUrl requestUrl)
{
    m_requestUrl = requestUrl;
}

/*!
  \internal
 */
void QWebSocketPrivate::setOrigin(QString origin)
{
    m_origin = origin;
}

/*!
  \internal
 */
void QWebSocketPrivate::setProtocol(QString protocol)
{
    m_protocol = protocol;
}

/*!
  \internal
 */
void QWebSocketPrivate::setExtension(QString extension)
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
    //pass through signals
    connect(pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), q_ptr, SIGNAL(error(QAbstractSocket::SocketError)));
    connect(pTcpSocket, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), q_ptr, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
    connect(pTcpSocket, SIGNAL(readChannelFinished()), q_ptr, SIGNAL(readChannelFinished()));
    connect(pTcpSocket, SIGNAL(aboutToClose()), q_ptr, SIGNAL(aboutToClose()));
    //connect(pTcpSocket, SIGNAL(bytesWritten(qint64)), q_ptr, SIGNAL(bytesWritten(qint64)));

    //catch signals
    connect(pTcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(processStateChanged(QAbstractSocket::SocketState)));
    connect(pTcpSocket, SIGNAL(readyRead()), this, SLOT(processData()));

    connect(&m_dataProcessor, SIGNAL(controlFrameReceived(QWebSocketProtocol::OpCode, QByteArray)), this, SLOT(processControlFrame(QWebSocketProtocol::OpCode, QByteArray)));
    connect(&m_dataProcessor, SIGNAL(textFrameReceived(QString,bool)), q_ptr, SIGNAL(textFrameReceived(QString,bool)));
    connect(&m_dataProcessor, SIGNAL(binaryFrameReceived(QByteArray,bool)), q_ptr, SIGNAL(binaryFrameReceived(QByteArray,bool)));
    connect(&m_dataProcessor, SIGNAL(binaryMessageReceived(QByteArray)), q_ptr, SIGNAL(binaryMessageReceived(QByteArray)));
    connect(&m_dataProcessor, SIGNAL(textMessageReceived(QString)), q_ptr, SIGNAL(textMessageReceived(QString)));
    connect(&m_dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)), this, SLOT(close(QWebSocketProtocol::CloseCode,QString)));
}

/*!
 * \internal
 */
void QWebSocketPrivate::releaseConnections(const QTcpSocket *pTcpSocket)
{
    if (pTcpSocket)
    {
        //pass through signals
        disconnect(pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), q_ptr, SIGNAL(error(QAbstractSocket::SocketError)));
        disconnect(pTcpSocket, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), q_ptr, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
        disconnect(pTcpSocket, SIGNAL(readChannelFinished()), q_ptr, SIGNAL(readChannelFinished()));
        //disconnect(pTcpSocket, SIGNAL(aboutToClose()), q_ptr, SIGNAL(aboutToClose()));
        //disconnect(pTcpSocket, SIGNAL(bytesWritten(qint64)), q_ptr, SIGNAL(bytesWritten(qint64)));

        //catched signals
        disconnect(pTcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(processStateChanged(QAbstractSocket::SocketState)));
        disconnect(pTcpSocket, SIGNAL(readyRead()), this, SLOT(processData()));
    }
    disconnect(&m_dataProcessor, SIGNAL(controlFrameReceived(QWebSocketProtocol::OpCode,QByteArray)), this, SLOT(processControlFrame(QWebSocketProtocol::OpCode,QByteArray)));
    disconnect(&m_dataProcessor, SIGNAL(textFrameReceived(QString,bool)), q_ptr, SIGNAL(textFrameReceived(QString,bool)));
    disconnect(&m_dataProcessor, SIGNAL(binaryFrameReceived(QByteArray,bool)), q_ptr, SIGNAL(binaryFrameReceived(QByteArray,bool)));
    disconnect(&m_dataProcessor, SIGNAL(binaryMessageReceived(QByteArray)), q_ptr, SIGNAL(binaryMessageReceived(QByteArray)));
    disconnect(&m_dataProcessor, SIGNAL(textMessageReceived(QString)), q_ptr, SIGNAL(textMessageReceived(QString)));
    disconnect(&m_dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)), this, SLOT(close(QWebSocketProtocol::CloseCode,QString)));
}

/*!
    \internal
 */
QWebSocketProtocol::Version QWebSocketPrivate::version()
{
    return m_version;
}

/*!
    \internal
 */
QString QWebSocketPrivate::resourceName()
{
    return m_resourceName;
}

/*!
    \internal
 */
QUrl QWebSocketPrivate::requestUrl()
{
    return m_requestUrl;
}

/*!
    \internal
 */
QString QWebSocketPrivate::origin()
{
    return m_origin;
}

/*!
    \internal
 */
QString QWebSocketPrivate::protocol()
{
    return m_protocol;
}

/*!
    \internal
 */
QString QWebSocketPrivate::extension()
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

        bool isLastFrame = (i == (numFrames - 1));
        bool isFirstFrame = (i == 0);

        quint64 size = qMin(bytesLeft, FRAME_SIZE_IN_BYTES);
        QWebSocketProtocol::OpCode opcode = isFirstFrame ? firstOpCode : QWebSocketProtocol::OC_CONTINUE;

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
                setErrorString(QWebSocket::tr("Error writing bytes to socket: %1.").arg(m_pSocket->errorString()));
                qDebug() << errorString();
                m_pSocket->flush();
                Q_EMIT q_ptr->error(QAbstractSocket::NetworkError);
                break;
            }
        }
        currentPosition += size;
        bytesLeft -= size;
    }
    if (payloadWritten != data.size())
    {
        setErrorString(QWebSocket::tr("Bytes written %1 != %2.").arg(payloadWritten).arg(data.size()));
        qDebug() << errorString();
        Q_EMIT q_ptr->error(QAbstractSocket::NetworkError);
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
    QString tmpKey = key % "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    QByteArray hash = QCryptographicHash::hash(tmpKey.toLatin1(), QCryptographicHash::Sha1);
    return QString(hash.toBase64());
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
            line.append(QChar(c));
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
    if (pSocket == 0)
    {
        return;
    }

    bool ok = false;
    QString errorDescription;

    const QString regExpStatusLine("^(HTTP/[0-9]+\\.[0-9]+)\\s([0-9]+)\\s(.*)");
    const QRegularExpression regExp(regExpStatusLine);
    QString statusLine = readLine(pSocket);
    QString httpProtocol;
    int httpStatusCode;
    QString httpStatusMessage;
    QRegularExpressionMatch match = regExp.match(statusLine);
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
        errorDescription = QWebSocket::tr("Invalid statusline in response: %1.").arg(statusLine);
    }
    else
    {
        QString headerLine = readLine(pSocket);
        QMap<QString, QString> headers;
        while (!headerLine.isEmpty())
        {
            QStringList headerField = headerLine.split(QString(": "), QString::SkipEmptyParts);
            headers.insertMulti(headerField[0], headerField[1]);
            headerLine = readLine(pSocket);
        }

        QString acceptKey = headers.value("Sec-WebSocket-Accept", "");
        QString upgrade = headers.value("Upgrade", "");
        QString connection = headers.value("Connection", "");
        //unused for the moment
        //QString extensions = headers.value("Sec-WebSocket-Extensions", "");
        //QString protocol = headers.value("Sec-WebSocket-Protocol", "");
        QString version = headers.value("Sec-WebSocket-Version", "");

        if (httpStatusCode == 101)	//HTTP/x.y 101 Switching Protocols
        {
            bool conversionOk = false;
            float version = httpProtocol.midRef(5).toFloat(&conversionOk);
            //TODO: do not check the httpStatusText right now
            ok = !(acceptKey.isEmpty() ||
                   (!conversionOk || (version < 1.1f)) ||
                   (upgrade.toLower() != "websocket") ||
                   (connection.toLower() != "upgrade"));
            if (ok)
            {
                QString accept = calculateAcceptKey(m_key);
                ok = (accept == acceptKey);
                if (!ok)
                {
                    errorDescription = QWebSocket::tr("Accept-Key received from server %1 does not match the client key %2.").arg(acceptKey).arg(accept);
                }
            }
            else
            {
                errorDescription = QWebSocket::tr("Invalid statusline in response: %1.").arg(statusLine);
            }
        }
        else if (httpStatusCode == 400)	//HTTP/1.1 400 Bad Request
        {
            if (!version.isEmpty())
            {
                QStringList versions = version.split(", ", QString::SkipEmptyParts);
                if (!versions.contains(QString::number(QWebSocketProtocol::currentVersion())))
                {
                    //if needed to switch protocol version, then we are finished here
                    //because we cannot handle other protocols than the RFC one (v13)
                    errorDescription = QWebSocket::tr("Handshake: Server requests a version that we don't support: %1.").arg(versions.join(", "));
                    ok = false;
                }
                else
                {
                    //we tried v13, but something different went wrong
                    errorDescription = QWebSocket::tr("Unknown error condition encountered. Aborting connection.");
                    ok = false;
                }
            }
        }
        else
        {
            errorDescription = QWebSocket::tr("Unhandled http status code: %1.").arg(httpStatusCode);
            ok = false;
        }

        if (!ok)
        {
            qDebug() << errorDescription;
            setErrorString(errorDescription);
            Q_EMIT q_ptr->error(QAbstractSocket::ConnectionRefusedError);
        }
        else
        {
            //handshake succeeded
            setSocketState(QAbstractSocket::ConnectedState);
            Q_EMIT q_ptr->connected();
        }
    }
}

/*!
    \internal
 */
void QWebSocketPrivate::processStateChanged(QAbstractSocket::SocketState socketState)
{
    QAbstractSocket::SocketState webSocketState = this->state();
    switch (socketState)
    {
        case QAbstractSocket::ConnectedState:
        {
            if (webSocketState == QAbstractSocket::ConnectingState)
            {
                m_key = generateKey();
                QString handshake = createHandShakeRequest(m_resourceName, m_requestUrl.host() % ":" % QString::number(m_requestUrl.port(80)), origin(), "", "", m_key);
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
                Q_EMIT q_ptr->disconnected();
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
void QWebSocketPrivate::processControlFrame(QWebSocketProtocol::OpCode opCode, QByteArray frame)
{
    switch (opCode)
    {
        case QWebSocketProtocol::OC_PING:
        {
            quint32 maskingKey = 0;
            if (m_mustMask)
            {
                maskingKey = generateMaskingKey();
            }
            m_pSocket->write(getFrameHeader(QWebSocketProtocol::OC_PONG, frame.size(), maskingKey, true));
            if (frame.size() > 0)
            {
                if (m_mustMask)
                {
                    QWebSocketProtocol::mask(&frame, maskingKey);
                }
                m_pSocket->write(frame);
            }
            break;
        }
        case QWebSocketProtocol::OC_PONG:
        {
            Q_EMIT q_ptr->pong(static_cast<quint64>(m_pingTimer.elapsed()));
            break;
        }
        case QWebSocketProtocol::OC_CLOSE:
        {
            quint16 closeCode = QWebSocketProtocol::CC_NORMAL;
            QString closeReason;
            if (frame.size() > 0)   //close frame can have a close code and reason
            {
                closeCode = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(frame.constData()));
                if (!QWebSocketProtocol::isCloseCodeValid(closeCode))
                {
                    closeCode = QWebSocketProtocol::CC_PROTOCOL_ERROR;
                    closeReason = QWebSocket::tr("Invalid close code %1 detected.").arg(closeCode);
                }
                else
                {
                    if (frame.size() > 2)
                    {
                        QTextCodec *tc = QTextCodec::codecForName("UTF-8");
                        QTextCodec::ConverterState state(QTextCodec::ConvertInvalidToNull);
                        closeReason = tc->toUnicode(frame.constData() + 2, frame.size() - 2, &state);
                        bool failed = (state.invalidChars != 0) || (state.remainingChars != 0);
                        if (failed)
                        {
                            closeCode = QWebSocketProtocol::CC_WRONG_DATATYPE;
                            closeReason = QWebSocket::tr("Invalid UTF-8 code encountered.");
                        }
                    }
                }
            }
            m_isClosingHandshakeReceived = true;
            close(static_cast<QWebSocketProtocol::CloseCode>(closeCode), closeReason);
            break;
        }
        case QWebSocketProtocol::OC_CONTINUE:
        case QWebSocketProtocol::OC_BINARY:
        case QWebSocketProtocol::OC_TEXT:
        case QWebSocketProtocol::OC_RESERVED_3:
        case QWebSocketProtocol::OC_RESERVED_4:
        case QWebSocketProtocol::OC_RESERVED_5:
        case QWebSocketProtocol::OC_RESERVED_6:
        case QWebSocketProtocol::OC_RESERVED_7:
        case QWebSocketProtocol::OC_RESERVED_B:
        case QWebSocketProtocol::OC_RESERVED_D:
        case QWebSocketProtocol::OC_RESERVED_E:
        case QWebSocketProtocol::OC_RESERVED_F:
        case QWebSocketProtocol::OC_RESERVED_V:
        {
            //do nothing
            //case added to make C++ compiler happy
            break;
        }
        default:
        {
            qDebug() << "WebSocket::processData: Invalid opcode detected:" << static_cast<int>(opCode);
            //Do nothing
            break;
        }
    }
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

    handshakeRequest << "GET " % resourceName % " HTTP/1.1" <<
                        "Host: " % host <<
                        "Upgrade: websocket" <<
                        "Connection: Upgrade" <<
                        "Sec-WebSocket-Key: " % QString(key);
    if (!origin.isEmpty())
    {
        handshakeRequest << "Origin: " % origin;
    }
    handshakeRequest << "Sec-WebSocket-Version: " % QString::number(QWebSocketProtocol::currentVersion());
    if (extensions.length() > 0)
    {
        handshakeRequest << "Sec-WebSocket-Extensions: " % extensions;
    }
    if (protocols.length() > 0)
    {
        handshakeRequest << "Sec-WebSocket-Protocol: " % protocols;
    }
    handshakeRequest << "\r\n";

    return handshakeRequest.join("\r\n");
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
    bool retVal = false;
    if (m_pSocket)
    {
        retVal = m_pSocket->waitForConnected(msecs);
    }
    return retVal;
}

/*!
    \internal
 */
bool QWebSocketPrivate::waitForDisconnected(int msecs)
{
    bool retVal = true;
    if (m_pSocket)
    {
        retVal = m_pSocket->waitForDisconnected(msecs);
    }
    return retVal;
}

/*!
    \internal
 */
void QWebSocketPrivate::setSocketState(QAbstractSocket::SocketState state)
{
    if (m_socketState != state)
    {
        m_socketState = state;
        Q_EMIT q_ptr->stateChanged(m_socketState);
    }
}

/*!
    \internal
 */
void QWebSocketPrivate::setErrorString(QString errorString)
{
    m_errorString = errorString;
}

/*!
    \internal
 */
QHostAddress QWebSocketPrivate::localAddress() const
{
    QHostAddress address;
    if (m_pSocket)
    {
        address = m_pSocket->localAddress();
    }
    return address;
}

/*!
    \internal
 */
quint16 QWebSocketPrivate::localPort() const
{
    quint16 port = 0;
    if (m_pSocket)
    {
        port = m_pSocket->localPort();
    }
    return port;
}

/*!
    \internal
 */
QAbstractSocket::PauseModes QWebSocketPrivate::pauseMode() const
{
    QAbstractSocket::PauseModes mode = QAbstractSocket::PauseNever;
    if (m_pSocket)
    {
        mode = m_pSocket->pauseMode();
    }
    return mode;
}

/*!
    \internal
 */
QHostAddress QWebSocketPrivate::peerAddress() const
{
    QHostAddress peer;
    if (m_pSocket)
    {
        peer = m_pSocket->peerAddress();
    }
    return peer;
}

/*!
    \internal
 */
QString QWebSocketPrivate::peerName() const
{
    QString name;
    if (m_pSocket)
    {
        name = m_pSocket->peerName();
    }
    return name;
}

/*!
    \internal
 */
quint16 QWebSocketPrivate::peerPort() const
{
    quint16 port = 0;
    if (m_pSocket)
    {
        port = m_pSocket->peerPort();
    }
    return port;
}

/*!
    \internal
 */
QNetworkProxy QWebSocketPrivate::proxy() const
{
    QNetworkProxy proxy;
    if (m_pSocket)
    {
        proxy = m_pSocket->proxy();
    }
    return proxy;
}

/*!
    \internal
 */
qint64 QWebSocketPrivate::readBufferSize() const
{
    qint64 readBuffer = 0;
    if (m_pSocket)
    {
        readBuffer = m_pSocket->readBufferSize();
    }
    return readBuffer;
}

/*!
    \internal
 */
void QWebSocketPrivate::resume()
{
    if (m_pSocket)
    {
        m_pSocket->resume();
    }
}

/*!
  \internal
 */
void QWebSocketPrivate::setPauseMode(QAbstractSocket::PauseModes pauseMode)
{
    if (m_pSocket)
    {
        m_pSocket->setPauseMode(pauseMode);
    }
}

/*!
    \internal
 */
void QWebSocketPrivate::setProxy(const QNetworkProxy &networkProxy)
{
    if (m_pSocket)
    {
        m_pSocket->setProxy(networkProxy);
    }
}

/*!
    \internal
 */
void QWebSocketPrivate::setReadBufferSize(qint64 size)
{
    if (m_pSocket)
    {
        m_pSocket->setReadBufferSize(size);
    }
}

/*!
    \internal
 */
void QWebSocketPrivate::setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value)
{
    if (m_pSocket)
    {
        m_pSocket->setSocketOption(option, value);
    }
}

/*!
    \internal
 */
QVariant QWebSocketPrivate::socketOption(QAbstractSocket::SocketOption option)
{
    QVariant result;
    if (m_pSocket)
    {
        result = m_pSocket->socketOption(option);
    }
    return result;
}

/*!
    \internal
 */
bool QWebSocketPrivate::isValid()
{
    bool valid = false;
    if (m_pSocket)
    {
        valid = m_pSocket->isValid();
    }
    return valid;
}

QT_END_NAMESPACE
