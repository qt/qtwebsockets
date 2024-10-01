### Introduction
`QtWebSockets` is a pure Qt implementation of WebSockets - both client and server.
It is implemented as a Qt add-on module, that can easily be embedded into existing Qt projects. It has no other dependencies than Qt.

### Features
* Client and server capable
* Text and binary sockets
* Frame-based and message-based signals
* Strict Unicode checking
* WSS and proxy support

### Update Notice
In this update, we have added a pre-shared encryption handshake feature to QWebSocketServer, enhancing the security of communication between the server and clients.
#### New Feature
- Pre-Shared Encryption Handshake
    - Files: `QWebSocketServer.h` and `QWebSocketServer.cpp`
    - Added the `preSharedEncryptionHandshake` function to handle encryption handshakes using pre-shared keys.

```c++
void preStartedEncryptionHandshake(QSslSocket *pTcpSocket);
```
This modification is based on the Qt source code. For more information, please visit the [Qt website](https://qt.io).

### Requirements
Qt 5.x

### Build And Usage
Checkout the source code from code.qt.io
Go into the source directory and execute:

    qmake
    make
    make install


The last command will install `QtWebSockets` as a Qt module.

To use, add `websockets` to the QT variable.

`QT += websockets`

### Compliance
`QtWebSockets` is compliant with [RFC6455](http://datatracker.ietf.org/doc/rfc6455/?include_text=1) and has been tested with the [Autobahn Testsuite](http://autobahn.ws/testsuite).

### Missing Features
* Extensions and sub-protocols

### License
This code is licensed under LGPLv3.
