TEMPLATE = subdirs

#SUBDIRS +=

contains(QT_CONFIG, private_tests): SUBDIRS += \
   websocketprotocol \
   dataprocessor \
   websocketframe \
   handshakerequest \
   websocketcorsauthenticator
