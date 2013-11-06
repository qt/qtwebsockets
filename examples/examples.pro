TEMPLATE = subdirs

SUBDIRS = echoclient \
          echoserver \
          sslechoserver \
          sslechoclient \
          simplechat
qtHaveModule(quick): SUBDIRS += qmlwebsocketclient
