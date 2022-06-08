// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick 2.5
import QtWebSockets 1.1
import QtTest 1.1

Rectangle {
    width: 360
    height: 360

    WebSocketServer {
        id: server
        port: 1337

        supportedSubprotocols: [ "chat", "superchat" ]

        onClientConnected: {
            currentSocket = webSocket;
        }

        property var currentSocket
    }

    WebSocket {
        id: socket
        url: server.url
        requestedSubprotocols: [ "superchat", "chat" ]
    }

    TestCase {
        function ensureConnected() {
            server.listen = true;
            socket.active = true;
            tryCompare(socket, 'status', WebSocket.Open);
            verify(server.currentSocket);

            // Handshake should select client's first preference.
            compare(socket.negotiatedSubprotocol, "superchat")
            compare(server.currentSocket.negotiatedSubprotocol, "superchat")
        }

        function ensureDisconnected() {
            socket.active = false;
            server.listen = false;
            tryCompare(socket, 'status', WebSocket.Closed);
            server.currentSocket = null;
        }

        function test_send_receive_text() {
            ensureConnected();

            var o = {};
            var sending = 'hello.';
            server.currentSocket.textMessageReceived.connect(function(received) {
                compare(received, sending);
                o.called = true;
            });

            socket.sendTextMessage(sending);
            tryCompare(o, 'called', true);
        }

        function test_send_text_error_closed() {
            ensureDisconnected();
            socket.sendTextMessage('hello');
            tryCompare(socket, 'status', WebSocket.Error);
        }

        function test_send_receive_binary() {
            ensureConnected();

            var o = {};
            var sending = new Uint8Array([42, 43]);
            server.currentSocket.binaryMessageReceived.connect(function(received) {
                var view = new DataView(received);
                compare(received.byteLength, sending.length);
                compare(view.getUInt8(0), sending[0]);
                compare(view.getUInt8(1), sending[1]);
                o.called = true;
            });

            socket.sendBinaryMessage(sending.buffer);
            tryCompare(o, 'called', true);
        }

        function test_send_binary_error_closed() {
            ensureDisconnected();
            socket.sendBinaryMessage('hello');
            tryCompare(socket, 'status', WebSocket.Error);
        }
    }
}
