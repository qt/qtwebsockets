TEMPLATE = subdirs

SUBDIRS = echoclient \
        echoserver
qtHaveModule(quick): SUBDIRS += qmlwebsocketclient
