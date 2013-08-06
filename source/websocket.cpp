#include "websocket.h"
#include "handshakerequest.h"
#include "handshakeresponse.h"
#include <QUrl>
#include <QTcpSocket>
#include <QByteArray>
#include <QtEndian>
#include <QCryptographicHash>
#include <QRegExp>
#include <QStringList>
#include <QHostAddress>
#include <QNetworkProxy>

#include <QDebug>

#include <limits>

/** @class WebSocket
	@brief The class WebSocket implements a tcp socket that talks the websocket protocol.
*/

const quint64 FRAME_SIZE_IN_BYTES = 512 * 512 * 2;	//maximum size of a frame when sending a message

/**
 * @brief WebSocket::WebSocket
 * @param version The version of the protocol to use (currently only V13 is supported)
 * @param parent The parent object
 */
WebSocket::WebSocket(WebSocketProtocol::Version version, QObject *parent) :
	QObject(parent),
	m_pSocket(new QTcpSocket(this)),
	m_version(version),
	m_resourceName(),
	m_requestUrl(),
	m_origin("imagine.barco.com"),
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
	makeConnections(m_pSocket);
	qsrand(static_cast<uint>(QDateTime::currentMSecsSinceEpoch()));
}

//only called by upgradeFrom
/**
 * @brief Constructor used for the server implementation. Should never be called directly.
 * @param pTcpSocket The tcp socket to use for this websocket
 * @param version The version of the protocol to speak (currently only V13 is supported)
 * @param parent The parent object of the WebSocket object
 */
WebSocket::WebSocket(QTcpSocket *pTcpSocket, WebSocketProtocol::Version version, QObject *parent) :
	QObject(parent),
	m_pSocket(pTcpSocket),
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
	makeConnections(m_pSocket);
}

/**
 * @brief WebSocket::~WebSocket
 */
WebSocket::~WebSocket()
{
	if (state() == QAbstractSocket::ConnectedState)
	{
		//qDebug() << "GOING_AWAY, connection closed.";
		close(WebSocketProtocol::CC_GOING_AWAY, "Connection closed");
		releaseConnections(m_pSocket);
	}
}

/**
 * @brief Aborts the current socket and resets the socket. Unlike disconnectFromHost(), this function immediately closes the socket, discarding any pending data in the write buffer.
 */
void WebSocket::abort()
{
	m_pSocket->abort();
}

/**
 * @brief Returns the type of error that last occurred
 * @return QAbstractSocket::SocketError
 */
QAbstractSocket::SocketError WebSocket::error() const
{
	return m_pSocket->error();
}

/**
 * @brief Returns a human-readable description of the last error that occurred
 * @return QString
 */
QString WebSocket::errorString() const
{
	return m_pSocket->errorString();
}

/**
 * @brief Sends the given string over the socket as a text message
 * @param message '\0' terminated string to be sent
 * @return The number of bytes actually sent
 * @see WebSocket::send(const QString &message)
 */
qint64 WebSocket::send(const char *message)
{
	return send(QString::fromUtf8(message));
}

/**
 * @brief Sends the given string over the socket as a text message
 * @param message The message to be sent
 * @return The number of bytes actually sent
 */
qint64 WebSocket::send(const QString &message)
{
	return doWriteData(message.toUtf8(), false);
}

/**
 * @brief Sends the given bytearray over the socket as a binary message
 * @param data The binary data to be sent
 * @return The number of bytes actually sent
 */
qint64 WebSocket::send(const QByteArray &data)
{
	return doWriteData(data, true);
}

WebSocket *WebSocket::upgradeFrom(QTcpSocket *pTcpSocket,
								  const HandshakeRequest &request,
								  const HandshakeResponse &response,
								  QObject *parent)
{
	WebSocket *pWebSocket = new WebSocket(pTcpSocket, response.getAcceptedVersion(), parent);
	pWebSocket->setExtension(response.getAcceptedExtension());
	pWebSocket->setOrigin(request.getOrigin());
	pWebSocket->setRequestUrl(request.getRequestUrl());
	pWebSocket->setProtocol(response.getAcceptedProtocol());
	pWebSocket->setResourceName(request.getRequestUrl().toString(QUrl::RemoveUserInfo));
	pWebSocket->enableMasking(false);	//a server should not send masked frames

	return pWebSocket;
}

/**
 * @brief Gracefully loses the socket with the given close code and reason. Any data in the write buffer is flushed before the socket is closed.
 * @param closeCode
 * @param reason
 */
void WebSocket::close(WebSocketProtocol::CloseCode closeCode, QString reason)
{
	if (!m_isClosingHandshakeSent)
	{
		m_pSocket->flush();

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
			WebSocketProtocol::mask(payload.data(), payload.size(), maskingKey);
		}
		QByteArray frame = getFrameHeader(WebSocketProtocol::OC_CLOSE, payload.size(), maskingKey, true);
		frame.append(payload);
		m_pSocket->write(frame);

		m_isClosingHandshakeSent = true;

		//setSocketState(QAbstractSocket::ClosingState);
		Q_EMIT aboutToClose();
	}
	//if (m_isClosingHandshakeSent && m_isClosingHandshakeReceived)
	{
		m_pSocket->close();
		setSocketState(QAbstractSocket::UnconnectedState);
		Q_EMIT disconnected();
	}
}

/**
 * @brief Opens a websocket connection using the given URL.
 * @param url
 * @param mask If true, all frames will be masked; this is only necessary for client side sockets; servers should never mask
 */
void WebSocket::open(const QUrl &url, bool mask)
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

/**
 * @brief Pings the server to indicate that the connection is still alive.
 */
void WebSocket::ping()
{
	m_pingTimer.restart();
	QByteArray pingFrame = getFrameHeader(WebSocketProtocol::OC_PING, 0, 0, true);
	writeFrame(pingFrame);
}

/**
 * @brief Sets the version to use for the websocket protocol; this must be set before the socket is opened.
 * @param version Currently only V13 is supported
 */
void WebSocket::setVersion(WebSocketProtocol::Version version)
{
	m_version = version;
}

/**
 * @brief Sets the resource name of the connection; must be set before the socket is openend
 * @param resourceName
 */
void WebSocket::setResourceName(QString resourceName)
{
	m_resourceName = resourceName;
}

/**
 * @brief WebSocket::setRequestUrl
 * @param requestUrl
 */
void WebSocket::setRequestUrl(QUrl requestUrl)
{
	m_requestUrl = requestUrl;
}

/**
 * @brief WebSocket::setOrigin
 * @param origin
 */
void WebSocket::setOrigin(QString origin)
{
	m_origin = origin;
}

/**
 * @brief WebSocket::setProtocol
 * @param protocol
 */
void WebSocket::setProtocol(QString protocol)
{
	m_protocol = protocol;
}

/**
 * @brief WebSocket::setExtension
 * @param extension
 */
void WebSocket::setExtension(QString extension)
{
	m_extension = extension;
}

/**
 * @brief WebSocket::enableMasking
 * @param enable
 */
void WebSocket::enableMasking(bool enable)
{
	m_mustMask = enable;
}

qint64 WebSocket::doWriteData(const QByteArray &data, bool isBinary)
{
	return doWriteFrames(data, isBinary);
}

void WebSocket::makeConnections(const QTcpSocket *pTcpSocket)
{
	//pass through signals
	connect(pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(error(QAbstractSocket::SocketError)));
	connect(pTcpSocket, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
	connect(pTcpSocket, SIGNAL(readChannelFinished()), this, SIGNAL(readChannelFinished()));
	connect(pTcpSocket, SIGNAL(aboutToClose()), this, SIGNAL(aboutToClose()));
	//connect(pTcpSocket, SIGNAL(bytesWritten(qint64)), this, SIGNAL(bytesWritten(qint64)));

	//catch signals
	connect(pTcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(processStateChanged(QAbstractSocket::SocketState)));
	connect(pTcpSocket, SIGNAL(readyRead()), this, SLOT(processData()));

	connect(&m_dataProcessor, SIGNAL(frameReceived(WebSocketProtocol::OpCode, QByteArray, bool)), this, SLOT(processFrame(WebSocketProtocol::OpCode, QByteArray, bool)));
	connect(&m_dataProcessor, SIGNAL(binaryMessageReceived(QByteArray)), this, SIGNAL(binaryMessageReceived(QByteArray)));
	connect(&m_dataProcessor, SIGNAL(textMessageReceived(QString)), this, SIGNAL(textMessageReceived(QString)));
	connect(&m_dataProcessor, SIGNAL(errorEncountered(WebSocketProtocol::CloseCode,QString)), this, SLOT(close(WebSocketProtocol::CloseCode,QString)));
}

void WebSocket::releaseConnections(const QTcpSocket *pTcpSocket)
{
	if (pTcpSocket)
	{
		//pass through signals
		disconnect(pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SIGNAL(error(QAbstractSocket::SocketError)));
		disconnect(pTcpSocket, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
		disconnect(pTcpSocket, SIGNAL(readChannelFinished()), this, SIGNAL(readChannelFinished()));
		disconnect(pTcpSocket, SIGNAL(aboutToClose()), this, SIGNAL(aboutToClose()));
		//disconnect(pTcpSocket, SIGNAL(bytesWritten(qint64)), this, SIGNAL(bytesWritten(qint64)));

		//catched signals
		disconnect(pTcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(processStateChanged(QAbstractSocket::SocketState)));
		disconnect(pTcpSocket, SIGNAL(readyRead()), this, SLOT(processData()));
	}
	disconnect(&m_dataProcessor, SIGNAL(frameReceived(WebSocketProtocol::OpCode,QByteArray,bool)), this, SLOT(processFrame(WebSocketProtocol::OpCode,QByteArray,bool)));
	disconnect(&m_dataProcessor, SIGNAL(binaryMessageReceived(QByteArray)), this, SIGNAL(binaryMessageReceived(QByteArray)));
	disconnect(&m_dataProcessor, SIGNAL(textMessageReceived(QString)), this, SIGNAL(textMessageReceived(QString)));
	disconnect(&m_dataProcessor, SIGNAL(errorEncountered(WebSocketProtocol::CloseCode,QString)), this, SLOT(close(WebSocketProtocol::CloseCode,QString)));
}

/**
 * @brief WebSocket::getVersion
 * @return
 */
WebSocketProtocol::Version WebSocket::getVersion()
{
	return m_version;
}

/**
 * @brief WebSocket::getRequestUrl
 * @return
 */
QUrl WebSocket::getRequestUrl()
{
	return m_requestUrl;
}

/**
 * @brief WebSocket::getOrigin
 * @return
 */
QString WebSocket::getOrigin()
{
	return m_origin;
}

/**
 * @brief WebSocket::getProtocol
 * @return
 */
QString WebSocket::getProtocol()
{
	return m_protocol;
}

/**
 * @brief WebSocket::getExtension
 * @return
 */
QString WebSocket::getExtension()
{
	return m_extension;
}

QByteArray WebSocket::getFrameHeader(WebSocketProtocol::OpCode opCode, quint64 payloadLength, quint32 maskingKey, bool lastFrame) const
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
		qDebug() << "WebSocket::getHeader: payload too big!";
	}

	return header;
}

qint64 WebSocket::doWriteFrames(const QByteArray &data, bool isBinary)
{
	const WebSocketProtocol::OpCode firstOpCode = isBinary ? WebSocketProtocol::OC_BINARY : WebSocketProtocol::OC_TEXT;

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
		WebSocketProtocol::OpCode opcode = isFirstFrame ? firstOpCode : WebSocketProtocol::OC_CONTINUE;

		//write header
		bytesWritten += m_pSocket->write(getFrameHeader(opcode, size, maskingKey, isLastFrame));

		//write payload
		if (size > 0)
		{
			char *currentData = payload + currentPosition;
			if (m_mustMask)
			{
				//WARNING: currentData is written over
				WebSocketProtocol::mask(currentData, size, maskingKey);
			}
			qint64 written = m_pSocket->write(currentData, static_cast<qint64>(size));
			if (written > 0)
			{
				bytesWritten += written;
				payloadWritten += written;
			}
			else
			{
				qDebug() << "WebSocket::doWriteFrames: Error writing bytes to socket:" << m_pSocket->errorString();
				m_pSocket->flush();
				break;
			}
		}
		currentPosition += size;
		bytesLeft -= size;
	}
	if (payloadWritten != data.size())
	{
		qDebug() << "Bytes written" << payloadWritten << "!=" << "data size:" << data.size();
	}
	return payloadWritten;
}

quint32 WebSocket::generateRandomNumber() const
{
	return static_cast<quint32>((static_cast<double>(qrand()) / RAND_MAX) * std::numeric_limits<quint32>::max());
}

quint32 WebSocket::generateMaskingKey() const
{
	return generateRandomNumber();
}

QByteArray WebSocket::generateKey() const
{
	QByteArray key;

	for (int i = 0; i < 4; ++i)
	{
		quint32 tmp = generateRandomNumber();
		key.append(static_cast<const char *>(static_cast<const void *>(&tmp)), sizeof(quint32));
	}

	return key.toBase64();
}

QString WebSocket::calculateAcceptKey(const QString &key) const
{
	QString tmpKey = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	QByteArray hash = QCryptographicHash::hash(tmpKey.toLatin1(), QCryptographicHash::Sha1);
	return QString(hash.toBase64());
}

qint64 WebSocket::writeFrames(const QList<QByteArray> &frames)
{
	qint64 written = 0;
	for (int i = 0; i < frames.size(); ++i)
	{
		written += writeFrame(frames[i]);
	}
	return written;
}

qint64 WebSocket::writeFrame(const QByteArray &frame)
{
	return m_pSocket->write(frame);
}

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

//called for a server handshake response
void WebSocket::processHandshake(QTcpSocket *pSocket)
{
	if (pSocket == 0)
	{
		return;
	}

	bool ok = false;
	const QString regExpStatusLine("^(HTTP/1.1)\\s([0-9]+)\\s(.*)");
	const QRegExp regExp(regExpStatusLine);
	QString statusLine = readLine(pSocket);
	QString httpProtocol;
	int httpStatusCode;
	QString httpStatusMessage;
	if (regExp.indexIn(statusLine) != -1)
	{
		QStringList tokens = regExp.capturedTexts();
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
		qDebug() << "WebSocket::processHandshake: Invalid statusline in response:" << statusLine;
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
		QString extensions = headers.value("Sec-WebSocket-Extensions", "");
		QString protocol = headers.value("Sec-WebSocket-Protocol", "");
		QString version = headers.value("Sec-WebSocket-Version", "");

		if (httpStatusCode == 101)	//HTTP/1.1 101 Switching Protocols
		{
			//do not check the httpStatusText right now
			ok = !(acceptKey.isEmpty() ||
				   (httpProtocol.toLower() != "http/1.1") ||
				   (upgrade.toLower() != "websocket") ||
				   (connection.toLower() != "upgrade"));
			if (ok)
			{
				QString accept = calculateAcceptKey(m_key);
				ok = (accept == acceptKey);
				if (!ok)
				{
					qDebug() << "WebSocket::processHandshake: Accept-Key received from server" << qPrintable(acceptKey) << "does not match the client key" << qPrintable(accept);
				}
			}
			else
			{
				qDebug() << "WebSocket::processHandshake: Invalid statusline in response:" << statusLine;
			}
		}
		else if (httpStatusCode == 400)	//HTTP/1.1 400 Bad Request
		{
			if (!version.isEmpty())
			{
				QStringList versions = version.split(", ", QString::SkipEmptyParts);
				if (!versions.contains("13"))
				{
					//if needed to switch protocol version, then we are finished here
					//because we cannot handle other protocols than the RFC one (v13)
					qDebug() << "WebSocket::processHandshake: Server requests a version that we don't support:" << versions;
					ok = false;
				}
				else
				{
					//we tried v13, but something different went wrong
					qDebug() << "WebSocket::processHandshake: Unknown error condition encountered. Aborting connection.";
					ok = false;
				}
			}
		}
		else
		{
			qDebug() << "WebSocket::processHandshake: Unhandled http status code" << httpStatusCode;
			ok = false;
		}

		if (!ok)
		{
			Q_EMIT error(QAbstractSocket::ConnectionRefusedError);
		}
		else
		{
			//handshake succeeded
			setSocketState(QAbstractSocket::ConnectedState);
			Q_EMIT connected();
		}
	}
}

void WebSocket::processStateChanged(QAbstractSocket::SocketState socketState)
{
	QAbstractSocket::SocketState webSocketState = this->state();
	switch (socketState)
	{
		case QAbstractSocket::ConnectedState:
		{
			if (webSocketState == QAbstractSocket::ConnectingState)
			{
				m_key = generateKey();
				QString handshake = createHandShakeRequest(m_resourceName, m_requestUrl.host() + ":" + QString::number(m_requestUrl.port(80)), getOrigin(), "", "", m_key);
				m_pSocket->write(handshake.toLatin1());
			}
			break;
		}
		case QAbstractSocket::ClosingState:
		{
			if (webSocketState == QAbstractSocket::ConnectedState)
			{
				setSocketState(QAbstractSocket::ClosingState);
				close(WebSocketProtocol::CC_GOING_AWAY);
				//Q_EMIT aboutToClose();
			}
			break;
		}
		case QAbstractSocket::UnconnectedState:
		{
			if (webSocketState != QAbstractSocket::UnconnectedState)
			{
				setSocketState(QAbstractSocket::UnconnectedState);
				Q_EMIT disconnected();
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
void WebSocket::processData()
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

//TODO: implement separate signals for textframereceived and binaryframereceivd
//in that way the UTF8 can be sent as is from within the dataprocessor
void WebSocket::processFrame(WebSocketProtocol::OpCode opCode, QByteArray frame, bool isLastFrame)
{
	switch (opCode)
	{
		case WebSocketProtocol::OC_BINARY:
		{
			Q_EMIT binaryFrameReceived(frame, isLastFrame);
			break;
		}
		case WebSocketProtocol::OC_TEXT:
		{
			Q_EMIT textFrameReceived(QString::fromUtf8(frame.constData(), frame.length()), isLastFrame);
			break;
		}
		case WebSocketProtocol::OC_PING:
		{
			quint32 maskingKey = 0;
			if (m_mustMask)
			{
				maskingKey = generateMaskingKey();
			}
			m_pSocket->write(getFrameHeader(WebSocketProtocol::OC_PONG, frame.size(), maskingKey, true));
			if (frame.size() > 0)
			{
				if (m_mustMask)
				{
					WebSocketProtocol::mask(&frame, maskingKey);
				}
				m_pSocket->write(frame);
			}
			break;
		}
		case WebSocketProtocol::OC_PONG:
		{
			Q_EMIT pong(static_cast<quint64>(m_pingTimer.elapsed()));
			break;
		}
		case WebSocketProtocol::OC_CLOSE:
		{
			quint16 closeCode = WebSocketProtocol::CC_NORMAL;
			QString closeReason;
			if (frame.size() > 0)   //close frame can have a close code and reason
			{
				closeCode = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(frame.constData()));
				frame.remove(0, 2);
				//TODO: check for invalid UTF-8 sequence (see testcase 7.5.1)
				closeReason = QString::fromUtf8(frame.constData(), frame.length());
				if (!WebSocketProtocol::isCloseCodeValid(closeCode))
				{
					closeCode = WebSocketProtocol::CC_PROTOCOL_ERROR;
				}
				m_isClosingHandshakeReceived = true;
			}
			close(static_cast<WebSocketProtocol::CloseCode>(closeCode), closeReason);
			break;
		}
		case WebSocketProtocol::OC_CONTINUE:
		case WebSocketProtocol::OC_RESERVED_3:
		case WebSocketProtocol::OC_RESERVED_4:
		case WebSocketProtocol::OC_RESERVED_5:
		case WebSocketProtocol::OC_RESERVED_6:
		case WebSocketProtocol::OC_RESERVED_7:
		case WebSocketProtocol::OC_RESERVED_B:
		case WebSocketProtocol::OC_RESERVED_D:
		case WebSocketProtocol::OC_RESERVED_E:
		case WebSocketProtocol::OC_RESERVED_F:
		case WebSocketProtocol::OC_RESERVED_V:
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

QString WebSocket::createHandShakeRequest(QString resourceName,
										  QString host,
										  QString origin,
										  QString extensions,
										  QString protocols,
										  QByteArray key)
{
	QStringList handshakeRequest;
	handshakeRequest << "GET " + resourceName + " HTTP/1.1" <<
						"Host: " + host <<
						"Upgrade: websocket" <<
						"Connection: Upgrade" <<
						"Sec-WebSocket-Key: " + QString(key) <<
						"Origin: " + origin <<
						"Sec-WebSocket-Version: 13";
	if (extensions.length() > 0)
	{
		handshakeRequest << "Sec-WebSocket-Extensions: " + extensions;
	}
	if (protocols.length() > 0)
	{
		handshakeRequest << "Sec-WebSocket-Protocol: " + protocols;
	}
	handshakeRequest << "\r\n";

	return handshakeRequest.join("\r\n");
}

/**
 * @brief WebSocket::state
 * @return
 */
QAbstractSocket::SocketState WebSocket::state() const
{
	return m_socketState;
}

/**
 * @brief WebSocket::waitForConnected
 * @param msecs
 * @return
 */
bool WebSocket::waitForConnected(int msecs)
{
	bool retVal = false;
	if (m_pSocket)
	{
		retVal = m_pSocket->waitForConnected(msecs);
	}
	return retVal;
}

/**
 * @brief WebSocket::waitForDisconnected
 * @param msecs
 * @return
 */
bool WebSocket::waitForDisconnected(int msecs)
{
	bool retVal = true;
	if (m_pSocket)
	{
		retVal = m_pSocket->waitForDisconnected(msecs);
	}
	return retVal;
}

void WebSocket::setSocketState(QAbstractSocket::SocketState state)
{
	if (m_socketState != state)
	{
		m_socketState = state;
		Q_EMIT stateChanged(m_socketState);
	}
}

/**
 * @brief WebSocket::localAddress
 * @return
 */
QHostAddress WebSocket::localAddress() const
{
	QHostAddress address;
	if (m_pSocket)
	{
		address = m_pSocket->localAddress();
	}
	return address;
}

/**
 * @brief WebSocket::localPort
 * @return
 */
quint16 WebSocket::localPort() const
{
	quint16 port = 0;
	if (m_pSocket)
	{
		port = m_pSocket->localPort();
	}
	return port;
}

/**
 * @brief WebSocket::peerAddress
 * @return
 */
QHostAddress WebSocket::peerAddress() const
{
	QHostAddress peer;
	if (m_pSocket)
	{
		peer = m_pSocket->peerAddress();
	}
	return peer;
}

/**
 * @brief WebSocket::peerName
 * @return
 */
QString WebSocket::peerName() const
{
	QString name;
	if (m_pSocket)
	{
		name = m_pSocket->peerName();
	}
	return name;
}

/**
 * @brief WebSocket::peerPort
 * @return
 */
quint16 WebSocket::peerPort() const
{
	quint16 port = 0;
	if (m_pSocket)
	{
		port = m_pSocket->peerPort();
	}
	return port;
}

/**
 * @brief WebSocket::proxy
 * @return
 */
QNetworkProxy WebSocket::proxy() const
{
	QNetworkProxy proxy;
	if (m_pSocket)
	{
		proxy = m_pSocket->proxy();
	}
	return proxy;
}

/**
 * @brief WebSocket::readBufferSize
 * @return
 */
qint64 WebSocket::readBufferSize() const
{
	qint64 readBuffer = 0;
	if (m_pSocket)
	{
		readBuffer = m_pSocket->readBufferSize();
	}
	return readBuffer;
}

/**
 * @brief WebSocket::setProxy
 * @param networkProxy
 */
void WebSocket::setProxy(const QNetworkProxy &networkProxy)
{
	if (m_pSocket)
	{
		m_pSocket->setProxy(networkProxy);
	}
}

/**
 * @brief WebSocket::isValid
 * @return
 */
bool WebSocket::isValid()
{
	bool valid = false;
	if (m_pSocket)
	{
		valid = m_pSocket->isValid();
	}
	return valid;
}
