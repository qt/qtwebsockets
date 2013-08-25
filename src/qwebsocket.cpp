#include "qwebsocket.h"
#include "qwebsocket_p.h"
#include <QUrl>
#include <QTcpSocket>
#include <QByteArray>
#include <QHostAddress>

#include <QDebug>

#include <limits>

/*!
	\class QWebSocket
	\brief The class QWebSocket implements a TCP socket that talks the websocket protocol.

	WebSockets is a web technology providing full-duplex communications channels over a single TCP connection.
	The WebSocket protocol was standardized by the IETF as RFC 6455 in 2011 (see http://tools.ietf.org/html/rfc6455).
	It can both be used in a client application and server application.

	This class was modeled after QAbstractSocket.

	\ref echoclient

	\author Kurt Pattyn (pattyn.kurt@gmail.com)
*/
/*!
  \page echoclient QWebSocket client example
  \brief A sample websocket client that sends a message and displays the message that it receives back.

  \section Description
  The EchoClient example implements a web socket client that sends a message to a websocket server and dumps the answer that it gets back.
  This example should ideally be used with the EchoServer example.
  \section Code
  We start by connecting to the `connected()` signal.
  \snippet echoclient.cpp constructor
  After the connection, we open the socket to the given \a url.

  \snippet echoclient.cpp onConnected
  When the client is connected successfully, we connect to the `onTextMessageReceived()` signal, and send out "Hello, world!".
  If connected with the EchoServer, we will receive the same message back.

  \snippet echoclient.cpp onTextMessageReceived
  Whenever a message is received, we write it out.
*/

/*!
  \fn void QWebSocket::connected()
  \brief Emitted when a connection is successfully established.
  \sa open(), disconnected()
*/
/*!
  \fn void QWebSocket::disconnected()
  \brief Emitted when the socket is disconnected.
  \sa close(), connected()
*/
/*!
	\fn void QWebSocket::aboutToClose()

	This signal is emitted when the socket is about to close.
	Connect this signal if you have operations that need to be performed before the socket closes
	(e.g., if you have data in a separate buffer that needs to be written to the device).

	\sa close()
 */
/*!
\fn void QWebSocket::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)

This signal can be emitted when a \a proxy that requires
authentication is used. The \a authenticator object can then be
filled in with the required details to allow authentication and
continue the connection.

\note It is not possible to use a QueuedConnection to connect to
this signal, as the connection will fail if the authenticator has
not been filled in with new information when the signal returns.

\sa QAuthenticator, QNetworkProxy
*/
/*!
	\fn void QWebSocket::stateChanged(QAbstractSocket::SocketState state);

	This signal is emitted whenever QWebSocket's state changes.
	The \a socketState parameter is the new state.

	QAbstractSocket::SocketState is not a registered metatype, so for queued
	connections, you will have to register it with Q_REGISTER_METATYPE() and
	qRegisterMetaType().

	\sa state()
*/
/*!
	\fn void QWebSocket::readChannelFinished()

	This signal is emitted when the input (reading) stream is closed in this device. It is emitted as soon as the closing is detected.

	\sa close()
*/

/*!
	\fn void QWebSocket::textFrameReceived(QString frame, bool isLastFrame);

	This signal is emitted whenever a text frame is received. The \a frame contains the data and
	\a isLastFrame indicates whether this is the last frame of the complete message.

	This signal can be used to process large messages frame by frame, instead of waiting for the complete
	message to arrive.

	\sa binaryFrameReceived(QByteArray, bool), textMessageReceived(QString)
*/
/*!
	\fn void QWebSocket::binaryFrameReceived(QByteArray frame, bool isLastFrame);

	This signal is emitted whenever a binary frame is received. The \a frame contains the data and
	\a isLastFrame indicates whether this is the last frame of the complete message.

	This signal can be used to process large messages frame by frame, instead of waiting for the complete
	message to arrive.

	\sa textFrameReceived(QString, bool), binaryMessageReceived(QByteArray)
*/
/*!
	\fn void QWebSocket::textMessageReceived(QString message);

	This signal is emitted whenever a text message is received. The \a message contains the received text.

	\sa textFrameReceived(QString, bool), binaryMessageReceived(QByteArray)
*/
/*!
	\fn void QWebSocket::binaryMessageReceived(QByteArray message);

	This signal is emitted whenever a binary message is received. The \a message contains the received bytes.

	\sa binaryFrameReceived(QByteArray, bool), textMessageReceived(QString)
*/
/*!
	\fn void QWebSocket::error(QAbstractSocket::SocketError error);

	This signal is emitted after an error occurred. The \a socketError
	parameter describes the type of error that occurred.

	QAbstractSocket::SocketError is not a registered metatype, so for queued
	connections, you will have to register it with Q_DECLARE_METATYPE() and
	qRegisterMetaType().

	\sa error(), errorString()
*/
/*!
  \fn void QWebSocket::pong(quint64 elapsedTime)

  Emitted when a pong message is received in reply to a previous ping.

  \sa ping()
  */

const quint64 FRAME_SIZE_IN_BYTES = 512 * 512 * 2;	//maximum size of a frame when sending a message

/*!
 * \brief Creates a new QWebSocket with the given \a origin, the \a version of the protocol to use and \a parent.
 *
 * The \a origin of the client is as specified in http://tools.ietf.org/html/rfc6454.
 * (The \a origin is not required for non-web browser clients (see RFC 6455)).
 * \note Currently only V13 (RFC 6455) is supported
 */
QWebSocket::QWebSocket(QString origin, QWebSocketProtocol::Version version, QObject *parent) :
	QObject(parent),
	d_ptr(new QWebSocketPrivate(origin, version, this, this))
{
}

/*!
 * \brief Destroys the QWebSocket. Closes the socket if it is still open, and releases any used resources.
 */
QWebSocket::~QWebSocket()
{
	delete d_ptr;
	//d_ptr = 0;
}

/*!
 * \brief Aborts the current socket and resets the socket. Unlike close(), this function immediately closes the socket, discarding any pending data in the write buffer.
 */
void QWebSocket::abort()
{
	d_ptr->abort();
}

/*!
 * Returns the type of error that last occurred
 * \sa errorString()
 */
QAbstractSocket::SocketError QWebSocket::error() const
{
	return d_ptr->error();
}

//only called by QWebSocketPrivate::upgradeFrom
/*!
  \internal
 */
QWebSocket::QWebSocket(QTcpSocket *pTcpSocket, QWebSocketProtocol::Version version, QObject *parent) :
	QObject(parent),
	d_ptr(new QWebSocketPrivate(pTcpSocket, version, this, this))
{
}

/*!
 * Returns a human-readable description of the last error that occurred
 *
 * \sa error()
 */
QString QWebSocket::errorString() const
{
	return d_ptr->errorString();
}

/*!
	This function writes as much as possible from the internal write buffer to the underlying network socket, without blocking.
	If any data was written, this function returns true; otherwise false is returned.
	Call this function if you need WebSocket to start sending buffered data immediately.
	The number of bytes successfully written depends on the operating system.
	In most cases, you do not need to call this function, because WebSocket will start sending data automatically once control goes back to the event loop.
	In the absence of an event loop, call waitForBytesWritten() instead.

	\sa send() and waitForBytesWritten().
*/
bool QWebSocket::flush()
{
	return d_ptr->flush();
}

/*!
 * Sends the given \a message over the socket as a text message and returns the number of bytes actually sent.
 * \param message Text message to be sent. Must be '\0' terminated.
 * \return The number of bytes actually sent.
 * \sa write(const QString &message) and write(const char *message, qint64 maxSize)
 */
qint64 QWebSocket::write(const char *message)
{
	return d_ptr->write(message);
}

/*!
 * Sends the most \a maxSize bytes of the given \a message over the socket as a text message and returns the number of bytes actually sent.
 * \param message Text message to be sent.
 * \return The number of bytes actually sent.
 * \sa write(const QString &message) and write(const char *message)
 */
qint64 QWebSocket::write(const char *message, qint64 maxSize)
{
	return d_ptr->write(message, maxSize);
}

/*!
	\brief Sends the given \a message over the socket as a text message and returns the number of bytes actually sent.
	\param message The message to be sent
	\return The number of bytes actually sent.
	\sa write(const char *message) and write(const char *message, qint64 maxSize)
 */
qint64 QWebSocket::write(const QString &message)
{
	return d_ptr->write(message);
}

/**
 * @brief Sends the given \a data over the socket as a binary message and returns the number of bytes actually sent.
 * @param data The binary data to be sent.
 * @return The number of bytes actually sent.
 */
qint64 QWebSocket::write(const QByteArray &data)
{
	return d_ptr->write(data);
}

/*!
 * \brief Gracefully closes the socket with the given \a closeCode and \a reason. Any data in the write buffer is flushed before the socket is closed.
 * \param closeCode The QWebSocketProtocol::CloseCode indicating the reason to close.
 * \param reason A string describing the error more in detail
 */
void QWebSocket::close(QWebSocketProtocol::CloseCode closeCode, QString reason)
{
	d_ptr->close(closeCode, reason);
}

/*!
 * \brief Opens a websocket connection using the given \a url.
 * If \a mask is true, all frames will be masked; this is only necessary for client side sockets; servers should never mask
 * \param url The url to connect to
 * \param mask When true, all frames are masked
 * \note A client socket must *always* mask its frames; servers may *never* mask its frames
 */
void QWebSocket::open(const QUrl &url, bool mask)
{
	d_ptr->open(url, mask);
}

/*!
 * \brief Pings the server to indicate that the connection is still alive.
 *
 * \sa pong()
 */
void QWebSocket::ping()
{
	d_ptr->ping();
}

/*!
 * \brief Returns the version the socket is currently using
 */
QWebSocketProtocol::Version QWebSocket::version()
{
	return d_ptr->version();
}

/**
 * @brief Returns the name of the resource currently accessed.
 */
QString QWebSocket::resourceName()
{
	return d_ptr->resourceName();
}

/*!
 * \brief Returns the url the socket is connected to or will connect to.
 */
QUrl QWebSocket::requestUrl()
{
	return d_ptr->requestUrl();
}

/*!
  Returns the current origin
 */
QString QWebSocket::origin()
{
	return d_ptr->origin();
}

/*!
  Returns the currently used protocol.
 */
QString QWebSocket::protocol()
{
	return d_ptr->protocol();
}

/*!
  Returns the currently used extension.
 */
QString QWebSocket::extension()
{
	return d_ptr->extension();
}

/*!
  Returns the current state of the socket
 */
QAbstractSocket::SocketState QWebSocket::state() const
{
	return d_ptr->state();
}

/**
	@brief Waits until the socket is connected, up to \a msecs milliseconds. If the connection has been established, this function returns true; otherwise it returns false. In the case where it returns false, you can call error() to determine the cause of the error.
	The following example waits up to one second for a connection to be established:

	~~~{.cpp}
	socket->open("ws://localhost:1234", false);
	if (socket->waitForConnected(1000))
	{
		qDebug("Connected!");
	}
	~~~

	If \a msecs is -1, this function will not time out.
	@note This function may wait slightly longer than msecs, depending on the time it takes to complete the host lookup.
	@note Multiple calls to this functions do not accumulate the time. If the function times out, the connecting process will be aborted.

	\param msecs The number of milliseconds to wait before a time out occurs; when -1, this function will block until the socket is connected.

	\sa connected(), open(), state()
 */
bool QWebSocket::waitForConnected(int msecs)
{
	return d_ptr->waitForConnected(msecs);
}

/*!
  Waits \a msecs for the socket to be disconnected.
  If the socket was successfully disconnected within time, this method returns true.
  Otherwise false is returned.

  \param msecs The number of milliseconds to wait before a time out occurs; when -1, this function will block until the socket is disconnected.

  \sa close(), state()
*/
bool QWebSocket::waitForDisconnected(int msecs)
{
	return d_ptr->waitForDisconnected(msecs);
}

/*!
  Returns the local address
 */
QHostAddress QWebSocket::localAddress() const
{
	return d_ptr->localAddress();
}

/*!
  Returns the local port
 */
quint16 QWebSocket::localPort() const
{
	return d_ptr->localPort();
}

/*!
  Returns the peer address
 */
QHostAddress QWebSocket::peerAddress() const
{
	return d_ptr->peerAddress();
}

/*!
  Returns the peerName
 */
QString QWebSocket::peerName() const
{
	return d_ptr->peerName();
}

/*!
  Returns the peerport
 */
quint16 QWebSocket::peerPort() const
{
	return d_ptr->peerPort();
}

/*!
  * Returns the currently configured proxy
 */
QNetworkProxy QWebSocket::proxy() const
{
	return d_ptr->proxy();
}

/*!
 * Returns the size in bytes of the readbuffer that is used by the socket.
 */
qint64 QWebSocket::readBufferSize() const
{
	return d_ptr->readBufferSize();
}

/*!
  Sets the proxy to \a networkProxy
 */
void QWebSocket::setProxy(const QNetworkProxy &networkProxy)
{
	d_ptr->setProxy(networkProxy);
}

/**
	Sets the size of WebSocket's internal read buffer to be size bytes.
	If the buffer size is limited to a certain size, WebSocket won't buffer more than this size of data.
	Exceptionally, a buffer size of 0 means that the read buffer is unlimited and all incoming data is buffered. This is the default.
	This option is useful if you only read the data at certain points in time (e.g., in a real-time streaming application) or if you want to protect your socket against receiving too much data, which may eventually cause your application to run out of memory.
	\sa readBufferSize() and read().
*/
void QWebSocket::setReadBufferSize(qint64 size)
{
	d_ptr->setReadBufferSize(size);
}

/*!
	Sets the given \a option to the value described by \a value.
	\sa socketOption().
*/
void QWebSocket::setSocketOption(QAbstractSocket::SocketOption option, const QVariant &value)
{
	d_ptr->setSocketOption(option, value);
}

/*!
	Returns the value of the option \a option.
	\sa setSocketOption().
*/
QVariant QWebSocket::socketOption(QAbstractSocket::SocketOption option)
{
	return d_ptr->socketOption(option);
}

/*!
  Returns true if the WebSocket is valid.
 */
bool QWebSocket::isValid()
{
	return d_ptr->isValid();
}
