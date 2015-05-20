TEMPLATE = subdirs

SUBDIRS += websockets

qtHaveModule(quick) {
    SUBDIRS += qml
}
