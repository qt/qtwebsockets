TEMPLATE = subdirs

SUBDIRS = echoclient \
          echoserver \
          simplechat

qtHaveModule(quick) {
SUBDIRS += qmlwebsocketclient \
           qmlwebsocketserver
}

qtConfig(ssl) {
SUBDIRS +=  \
            sslechoserver \
            sslechoclient
}
