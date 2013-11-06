TEMPLATE = subdirs

SUBDIRS = echoclient \
          echoserver \
          simplechat
qtHaveModule(quick): SUBDIRS += qmlwebsocketclient
