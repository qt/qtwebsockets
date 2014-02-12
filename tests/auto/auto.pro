TEMPLATE = subdirs

SUBDIRS = \
    qwebsocketcorsauthenticator

contains(QT_CONFIG, private_tests): SUBDIRS += \
   websocketprotocol \
   dataprocessor \
   websocketframe \
   handshakerequest \
   qdefaultmaskgenerator

SUBDIRS += \
    qwebsocket \
    qwebsocketserver
