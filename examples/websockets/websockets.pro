TEMPLATE = subdirs

SUBDIRS = echoclient \
          echoserver \
          simplechat
qtHaveModule(quick): SUBDIRS += qmlwebsocketclient

contains(QT_CONFIG, openssl) | contains(QT_CONFIG, openssl-linked) {
SUBDIRS +=  \
            sslechoserver \
            sslechoclient
}
