import QtQuick 2.0
import Qt.Playground.WebSockets 1.0

Rectangle {
    width: 360
    height: 360

    WebSocket {

    }

    Text {
        text: qsTr("Hello World")
        anchors.centerIn: parent
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            Qt.quit();
        }
    }
}
