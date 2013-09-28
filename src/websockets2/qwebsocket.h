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

#ifndef QWEBSOCKET_H
#define QWEBSOCKET_H

#include "qwebsockets_global.h"

#include <QObject>
#include <QSharedPointer>
#include <QUrl>

QT_BEGIN_NAMESPACE

class QWebSocketFrame;
class QWebSocketFrames; // typdef QWebSocketFrames QList<QWebSocketFrame>
class QNetworkProxy;
class QAuthenticator;
class QWebSocketError; // small class, with error code, message and QDebug operator

class QWebSocketPrivate;
class Q_WEBSOCKETS_EXPORT QWebSocket : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QWebSocket)
    Q_DECLARE_PRIVATE(QWebSocket)

    Q_ENUMS(ConnectionState)
    Q_FLAGS(ReceiveSignals)

public:
    enum ConnectionState {
        ConnectingState = 0x0, // server lookup
        HandshakeState = 0x1, // while handshaking
        ConnectedState = 0x2,
        ClosingState = 0x3,
        DisconnectedState = 0x4 // after close, or before connecting
    };

    /*
    * Meaning of this enum / flags is to optimize incoming data,
    *
    * If someone wants to use the easy api (default) than receiveSignals method is set to: TextAndDataReceivedSignals
    * in this case, the signals: textReceived() & dataReceived() will be emitted accordingly on incoming data.
    * incoming frames will be queued, until the full message is received.
    *
    * In case someone wants to do stream reading (low level api), you could set receiveSignals to TextAndDataFrameReceivedSignals,
    * this way messages will not be queued, and every incoming frame will be directly emitted.
    * for large binary data messages, this can increase performance.
    *
    * Combinations can be made trough flags
    *
    * example:
    * # setReceiveSignals(TextReceivedSignal | DataFrameReceivedSignal);
    *
    * -> text message frames will be queued, untill the message is complete, and than emitted trough:
    *    void textReceived(const QString &message);
    *
    * -> binary messages will not be queued, and emitted directly trough:
    *    void dataFrameReceived(const QWebSocketFrame &dataFrame)
    *
    */
    enum ReceiveSignal {
        TextReceivedSignal = 0x1,
        DataReceivedSignal = 0x2,
        TextAndDataReceivedSignals = 0x3,

        TextFrameReceivedSignal = 0x4,
        DataFrameReceivedSignal = 0x8,
        TextAndDataFrameReceivedSignals = 0x12,

        AllReceivedSignals = 0x15
    };
    Q_DECLARE_FLAGS(ReceiveSignals, ReceiveSignal)

public:
    explicit QWebSocket(QObject *parent = 0);
    QWebSocket(const QUrl &url, const QStringList &subProtocols, QObject *parent = 0);
    ~QWebSocket();

public:
    // Get methods
    QUrl url() const; // defined server url
    bool isMasked() const; // does client use masked frames
    QStringList subProtocols() const; // client side defined subprotocols,
    QString serverSubProtocol() const; // subprotocol selected by the server
    ConnectionState state() const;
    bool isConnected() const;
    bool isReconnect() const; // does the socket tries to reconnect in case of network failure
    ReceiveSignals receiveSignals() const; // flags (enum) which defines emit signals
    QWebSocketError lastError() const; // return last error

    // Set methods
    void setUrl(const QUrl &url); // define server url, or use trough ctor (only writable if disconnected)
    void setMask(bool state); // define if client should use masked frames (only writable if disconnected)
    void setSubProtocols(const QStringList &subProtocols); // define client side subprotocols, or use trough ctor (only writable if disconnected)
    void setReconnect(bool state);
    void setReceiveSignals(ReceiveSignals receiveSignals); // define which signals should be emitted (performance optimization)

    // Connection methods
    void connectToHost(const QUrl &url = QUrl(), bool mask = true); // connect, optional define url and mask
    void disconnectFromHost(); // close connection

    // Message & data methods
    bool sendText(const QString &message); // sends QString, char *, ...
    bool sendData(const QByteArray &message); // sends data (binary)
    bool sendFrame(const QWebSocketFrame &messageFrame);  // send a frame, low level
    bool sendFrames(const QWebSocketFrames &messageFrames);  // send multiple frames at once, low level

    // Proxy suppo
    void setProxy(const QNetworkProxy &networkProxy);

Q_SIGNALS:
    // Connection signals
    void connected(); // ready to read / write
    void disconnected(); // socket closed
    void stateChanged(ConnectionState state); // emit on state change
    void serverSubProtocolNegotiated(const QString &subProtocol); // emit selected protocol after handshake
    void error(const QWebSocketError &error); // emitted on error

    // Message signals (related to enum ReceiveSignal)
    void textReceived(const QString &message); // full text message, easy api
    void textFrameReceived(const QWebSocketFrame &messageFrame); // text frame, for stream reading
    void dataReceived(const QByteArray &data); // full data message, easy api
    void dataFrameReceived(const QWebSocketFrame &dataFrame); // data frame, for stream reading
    void frameReceived(const QWebSocketFrame &messageFrame); // frame (text & data), low level

    // Proxy support
    void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);

private:
    QSharedPointer<QWebSocketPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBSOCKET_H
