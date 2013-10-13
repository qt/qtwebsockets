TEMPLATE = subdirs

SUBDIRS += \

contains(QT_CONFIG, private_tests): SUBDIRS += \
   dataprocessor \
   websocketprotocol \
   websocketframe \
   websocketcorsauthenticator
