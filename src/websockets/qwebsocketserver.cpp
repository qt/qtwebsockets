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

/*!
    \class QWebSocketServer

    \inmodule QtWebSockets

    \brief Implements a websocket-based server.

    It is modeled after QTcpServer, and behaves the same. So, if you know how to use QTcpServer,
    you know how to use QWebSocketServer.
    This class makes it possible to accept incoming websocket connections.
    You can specify the port or have QWebSocketServer pick one automatically.
    You can listen on a specific address or on all the machine's addresses.
    Call listen() to have the server listen for incoming connections.

    The newConnection() signal is then emitted each time a client connects to the server.
    Call nextPendingConnection() to accept the pending connection as a connected QWebSocket.
    The function returns a pointer to a QWebSocket in QAbstractSocket::ConnectedState that you can
    use for communicating with the client.
    If an error occurs, serverError() returns the type of error, and errorString() can be called
    to get a human readable description of what happened.
    When listening for connections, the address and port on which the server is listening are
    available as serverAddress() and serverPort().
    Calling close() makes QWebSocketServer stop listening for incoming connections.

    \sa echoserver.html

    \sa QWebSocket
*/

/*!
  \page echoserver.html example
  \title WebSocket server example
  \brief A sample websocket server echoing back messages sent to it.

  \section1 Description
  The echoserver example implements a web socket server that echoes back everything that is sent
  to it.
  \section1 Code
  We start by creating a QWebSocketServer (`new QWebSocketServer()`). After the creation, we listen
  on all local network interfaces (`QHostAddress::Any`) on the specified \a port.
  \snippet echoserver/echoserver.cpp constructor
  If listening is successful, we connect the `newConnection()` signal to the slot
  `onNewConnection()`.
  The `newConnection()` signal will be thrown whenever a new web socket client is connected to our
  server.

  \snippet echoserver/echoserver.cpp onNewConnection
  When a new connection is received, the client QWebSocket is retrieved (`nextPendingConnection()`),
  and the signals we are interested in are connected to our slots
  (`textMessageReceived()`, `binaryMessageReceived()` and `disconnected()`).
  The client socket is remembered in a list, in case we would like to use it later
  (in this example, nothing is done with it).

  \snippet echoserver/echoserver.cpp processMessage
  Whenever `processMessage()` is triggered, we retrieve the sender, and if valid, send back the
  original message (`send()`).
  The same is done with binary messages.
  \snippet echoserver/echoserver.cpp processBinaryMessage
  The only difference is that the message now is a QByteArray instead of a QString.

  \snippet echoserver/echoserver.cpp socketDisconnected
  Whenever a socket is disconnected, we remove it from the clients list and delete the socket.
  Note: it is best to use `deleteLater()` to delete the socket.
*/

/*!
    \fn void QWebSocketServer::acceptError(QAbstractSocket::SocketError socketError)
    This signal is emitted when accepting a new connection results in an error.
    The \a socketError parameter describes the type of error that occurred

    \sa pauseAccepting(), resumeAccepting()
*/

/*!
    \fn void QWebSocketServer::serverError(QWebSocketProtocol::CloseCode closeCode)
    This signal is emitted when an error occurs during the setup of a web socket connection.
    The \a closeCode parameter describes the type of error that occurred

    \sa errorString()
*/

/*!
    \fn void QWebSocketServer::newConnection()
    This signal is emitted every time a new connection is available.

    \sa hasPendingConnections(), nextPendingConnection()
*/

/*!
    \fn void QWebSocketServer::closed()
    This signal is emitted when the server closed it's connection.

    \sa close()
*/

/*!
    \fn void QWebSocketServer::originAuthenticationRequired(QWebSocketCorsAuthenticator *authenticator)
    This signal is emitted when a new connection is requested.
    The slot connected to this signal should indicate whether the origin
    (which can be determined by the origin() call) is allowed in the \a authenticator object
    (by issuing \l{QWebSocketCorsAuthenticator::}{setAllowed()})

    If no slot is connected to this signal, all origins will be accepted by default.

    \note It is not possible to use a QueuedConnection to connect to
    this signal, as the connection will always succeed.
*/

/*!
    \fn void QWebSocketServer::peerVerifyError(const QSslError &error)

    QWebSocketServer can emit this signal several times during the SSL handshake,
    before encryption has been established, to indicate that an error has
    occurred while establishing the identity of the peer. The \a error is
    usually an indication that QWebSocketServer is unable to securely identify the
    peer.

    This signal provides you with an early indication when something's wrong.
    By connecting to this signal, you can manually choose to tear down the
    connection from inside the connected slot before the handshake has
    completed. If no action is taken, QWebSocketServer will proceed to emitting
    QWebSocketServer::sslErrors().

    \sa sslErrors()
*/

/*!
    \fn void QWebSocketServer::sslErrors(const QList<QSslError> &errors)

    QWebSocketServer emits this signal after the SSL handshake to indicate that one
    or more errors have occurred while establishing the identity of the
    peer. The errors are usually an indication that QWebSocketServer is unable to
    securely identify the peer. Unless any action is taken, the connection
    will be dropped after this signal has been emitted.

    \a errors contains one or more errors that prevent QSslSocket from
    verifying the identity of the peer.

    \sa peerVerifyError()
*/

/*!
  \enum QWebSocketServer::SecureMode
  Indicates whether the server operates over wss (SECURE_MODE) or ws (NON_SECURE_MODE)

  \value SECURE_MODE The server operates in secure mode (over wss)
  \value NON_SECURE_MODE The server operates in non-secure mode (over ws)
  */

#include "qwebsocketprotocol.h"
#include "qwebsocket.h"
#include "qwebsocketserver.h"
#include "qwebsocketserver_p.h"

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QNetworkProxy>

#ifndef QT_NO_SSL
#include <QtNetwork/QSslConfiguration>
#endif

QT_BEGIN_NAMESPACE

/*!
    Constructs a new WebSocketServer with the given \a serverName.
    The \a serverName will be used in the http handshake phase to identify the server.
    The \a secureMode parameter indicates whether the server operates over wss (\l{SECURE_MODE})
    or over ws (\l{NON_SECURE_MODE}).

    \a parent is passed to the QObject constructor.
 */
QWebSocketServer::QWebSocketServer(const QString &serverName, SecureMode secureMode,
                                   QObject *parent) :
    QObject(parent),
    d_ptr(new QWebSocketServerPrivate(serverName,
                                      #ifndef QT_NO_SSL
                                      (secureMode == SECURE_MODE) ?
                                          QWebSocketServerPrivate::SECURE_MODE :
                                          QWebSocketServerPrivate::NON_SECURE_MODE,
                                      #else
                                      QWebSocketServerPrivate::NON_SECURE_MODE,
                                      #endif
                                      this,
                                      this))
{
#ifdef QT_NO_SSL
    Q_UNUSED(secureMode)
#endif
}

/*!
    Destroys the WebSocketServer object. If the server is listening for connections,
    the socket is automatically closed.
    Any client WebSockets that are still connected are closed and deleted.

    \sa close()
 */
QWebSocketServer::~QWebSocketServer()
{
    delete d_ptr;
}

/*!
  Closes the server. The server will no longer listen for incoming connections.
 */
void QWebSocketServer::close()
{
    Q_D(QWebSocketServer);
    d->close();
}

/*!
    Returns a human readable description of the last error that occurred.

    \sa serverError()
*/
QString QWebSocketServer::errorString() const
{
    Q_D(const QWebSocketServer);
    return d->errorString();
}

/*!
    Returns true if the server has pending connections; otherwise returns false.

    \sa nextPendingConnection(), setMaxPendingConnections()
 */
bool QWebSocketServer::hasPendingConnections() const
{
    Q_D(const QWebSocketServer);
    return d->hasPendingConnections();
}

/*!
    Returns true if the server is currently listening for incoming connections;
    otherwise returns false.

    \sa listen()
 */
bool QWebSocketServer::isListening() const
{
    Q_D(const QWebSocketServer);
    return d->isListening();
}

/*!
    Tells the server to listen for incoming connections on address \a address and port \a port.
    If \a port is 0, a port is chosen automatically.
    If \a address is QHostAddress::Any, the server will listen on all network interfaces.

    Returns true on success; otherwise returns false.

    \sa isListening()
 */
bool QWebSocketServer::listen(const QHostAddress &address, quint16 port)
{
    Q_D(QWebSocketServer);
    return d->listen(address, port);
}

/*!
    Returns the maximum number of pending accepted connections. The default is 30.

    \sa setMaxPendingConnections(), hasPendingConnections()
 */
int QWebSocketServer::maxPendingConnections() const
{
    Q_D(const QWebSocketServer);
    return d->maxPendingConnections();
}

/*!
    Returns the next pending connection as a connected WebSocket object.
    The socket is created as a child of the server, which means that it is automatically
    deleted when the WebSocketServer object is destroyed.
    It is still a good idea to delete the object explicitly when you are done with it,
    to avoid wasting memory.
    Q_NULLPTR is returned if this function is called when there are no pending connections.

    Note: The returned WebSocket object cannot be used from another thread..

    \sa hasPendingConnections()
*/
QWebSocket *QWebSocketServer::nextPendingConnection()
{
    Q_D(QWebSocketServer);
    return d->nextPendingConnection();
}

/*!
    Pauses incoming new connections. Queued connections will remain in queue.
    \sa resumeAccepting()
 */
void QWebSocketServer::pauseAccepting()
{
    Q_D(QWebSocketServer);
    d->pauseAccepting();
}

#ifndef QT_NO_NETWORKPROXY
/*!
    Returns the network proxy for this socket. By default QNetworkProxy::DefaultProxy is used.

    \sa setProxy()
*/
QNetworkProxy QWebSocketServer::proxy() const
{
    Q_D(const QWebSocketServer);
    return d->proxy();
}

/*!
    \brief Sets the explicit network proxy for this socket to \a networkProxy.

    To disable the use of a proxy for this socket, use the QNetworkProxy::NoProxy proxy type:

    \code
        server->setProxy(QNetworkProxy::NoProxy);
    \endcode

    \sa proxy()
*/
void QWebSocketServer::setProxy(const QNetworkProxy &networkProxy)
{
    Q_D(QWebSocketServer);
    d->setProxy(networkProxy);
}
#endif

#ifndef QT_NO_SSL
/*!
    Sets the SSL configuration for the websocket server to \a sslConfiguration.
    This method has no effect if QWebSocketServer runs in non-secure mode
    (QWebSocketServer::NON_SECURE_MODE).

    \sa sslConfiguration(), SecureMode
 */
void QWebSocketServer::setSslConfiguration(const QSslConfiguration &sslConfiguration)
{
    Q_D(QWebSocketServer);
    d->setSslConfiguration(sslConfiguration);
}

/*!
    Returns the SSL configuration used by the websocket server.
    If the server is not running in secure mode (QWebSocketServer::SECURE_MODE),
    this method returns QSslConfiguration::defaultConfiguration().

    \sa setSslConfiguration(), SecureMode, QSslConfiguration::defaultConfiguration()
 */
QSslConfiguration QWebSocketServer::sslConfiguration() const
{
    Q_D(const QWebSocketServer);
    return d->sslConfiguration();
}
#endif

/*!
    Resumes accepting new connections.
    \sa pauseAccepting()
 */
void QWebSocketServer::resumeAccepting()
{
    Q_D(QWebSocketServer);
    d->resumeAccepting();
}

/*!
    Sets the server name that will be used during the http handshake phase to the given
    \a serverName.
    Existing connected clients will not be notified of this change, only newly connecting clients
    will see this new name.
 */
void QWebSocketServer::setServerName(const QString &serverName)
{
    Q_D(QWebSocketServer);
    d->setServerName(serverName);
}

/*!
    Returns the server name that is used during the http handshake phase.
 */
QString QWebSocketServer::serverName() const
{
    Q_D(const QWebSocketServer);
    return d->serverName();
}

/*!
    Returns the server's address if the server is listening for connections; otherwise returns
    QHostAddress::Null.

    \sa serverPort(), listen()
 */
QHostAddress QWebSocketServer::serverAddress() const
{
    Q_D(const QWebSocketServer);
    return d->serverAddress();
}

/*!
    Returns the mode the server is running in.

    \sa QWebSocketServer(), SecureMode
 */
QWebSocketServer::SecureMode QWebSocketServer::secureMode() const
{
#ifndef QT_NO_SSL
    Q_D(const QWebSocketServer);
    return (d->secureMode() == QWebSocketServerPrivate::SECURE_MODE) ?
                QWebSocketServer::SECURE_MODE : QWebSocketServer::NON_SECURE_MODE;
#else
    return QWebSocketServer::NON_SECURE_MODE;
#endif
}

/*!
    Returns an error code for the last error that occurred.

    \sa errorString()
 */
QWebSocketProtocol::CloseCode QWebSocketServer::error() const
{
    Q_D(const QWebSocketServer);
    return d->serverError();
}

/*!
    Returns the server's port if the server is listening for connections; otherwise returns 0.

    \sa serverAddress(), listen()
 */
quint16 QWebSocketServer::serverPort() const
{
    Q_D(const QWebSocketServer);
    return d->serverPort();
}

/*!
    Sets the maximum number of pending accepted connections to \a numConnections.
    WebSocketServer will accept no more than \a numConnections incoming connections before
    nextPendingConnection() is called.
    By default, the limit is 30 pending connections.

    Clients may still able to connect after the server has reached its maximum number of
    pending connections (i.e., WebSocket can still emit the connected() signal).
    WebSocketServer will stop accepting the new connections, but the operating system may still
    keep them in queue.

    \sa maxPendingConnections(), hasPendingConnections()
 */
void QWebSocketServer::setMaxPendingConnections(int numConnections)
{
    Q_D(QWebSocketServer);
    d->setMaxPendingConnections(numConnections);
}

/*!
    Sets the socket descriptor this server should use when listening for incoming connections to
    \a socketDescriptor.

    Returns true if the socket is set successfully; otherwise returns false.
    The socket is assumed to be in listening state.

    \sa socketDescriptor(), isListening()
 */
bool QWebSocketServer::setSocketDescriptor(int socketDescriptor)
{
    Q_D(QWebSocketServer);
    return d->setSocketDescriptor(socketDescriptor);
}

/*!
    Returns the native socket descriptor the server uses to listen for incoming instructions,
    or -1 if the server is not listening.
    If the server is using QNetworkProxy, the returned descriptor may not be usable with
    native socket functions.

    \sa setSocketDescriptor(), isListening()
 */
int QWebSocketServer::socketDescriptor() const
{
    Q_D(const QWebSocketServer);
    return d->socketDescriptor();
}

/*!
  Returns a list of websocket versions that this server is supporting.
 */
QList<QWebSocketProtocol::Version> QWebSocketServer::supportedVersions() const
{
    Q_D(const QWebSocketServer);
    return d->supportedVersions();
}

/*!
  Returns a list of websocket subprotocols that this server supports.
 */
QStringList QWebSocketServer::supportedProtocols() const
{
    Q_D(const QWebSocketServer);
    return d->supportedProtocols();
}

/*!
  Returns a list of websocket extensions that this server supports.
 */
QStringList QWebSocketServer::supportedExtensions() const
{
    Q_D(const QWebSocketServer);
    return d->supportedExtensions();
}

QT_END_NAMESPACE
