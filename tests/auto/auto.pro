TEMPLATE = subdirs

SUBDIRS += \
   websocketcorsauthenticator

contains(QT_CONFIG, private_tests): SUBDIRS += \
   websocketprotocol \
   dataprocessor \
   websocketframe \
   handshakerequest
