/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <QtTest/QtTest>
#include <QtTest/qtestcase.h>
#include <QtCore/QDebug>
#include <QtCore/QByteArray>
#include <QtCore/QtEndian>

#include "private/qwebsockethandshakerequest_p.h"
#include "QtWebSockets/qwebsocketprotocol.h"

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QWebSocketProtocol::CloseCode)
Q_DECLARE_METATYPE(QWebSocketProtocol::OpCode)

class tst_HandshakeRequest : public QObject
{
    Q_OBJECT

public:
    tst_HandshakeRequest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void tst_initialization();

    void tst_invalidStream_data();
    void tst_invalidStream();

    void tst_multipleValuesInConnectionHeader();
    void tst_multipleVersions();
};

tst_HandshakeRequest::tst_HandshakeRequest()
{}

void tst_HandshakeRequest::initTestCase()
{
}

void tst_HandshakeRequest::cleanupTestCase()
{}

void tst_HandshakeRequest::init()
{
    qRegisterMetaType<QWebSocketProtocol::OpCode>("QWebSocketProtocol::OpCode");
    qRegisterMetaType<QWebSocketProtocol::CloseCode>("QWebSocketProtocol::CloseCode");
}

void tst_HandshakeRequest::cleanup()
{
}

void tst_HandshakeRequest::tst_initialization()
{
    {
        QWebSocketHandshakeRequest request(0, false);
        QCOMPARE(request.port(), 0);
        QVERIFY(!request.isSecure());
        QVERIFY(!request.isValid());
        QCOMPARE(request.extensions().length(), 0);
        QCOMPARE(request.protocols().length(), 0);
        QCOMPARE(request.headers().size(), 0);
        QCOMPARE(request.key().length(), 0);
        QCOMPARE(request.origin().length(), 0);
        QCOMPARE(request.host().length(), 0);
        QVERIFY(request.requestUrl().isEmpty());
        QCOMPARE(request.resourceName().length(), 0);
        QCOMPARE(request.versions().length(), 0);
    }
    {
        QWebSocketHandshakeRequest request(80, true);
        QCOMPARE(request.port(), 80);
        QVERIFY(request.isSecure());
        QVERIFY(!request.isValid());
        QCOMPARE(request.extensions().length(), 0);
        QCOMPARE(request.protocols().length(), 0);
        QCOMPARE(request.headers().size(), 0);
        QCOMPARE(request.key().length(), 0);
        QCOMPARE(request.origin().length(), 0);
        QCOMPARE(request.host().length(), 0);
        QVERIFY(request.requestUrl().isEmpty());
        QCOMPARE(request.resourceName().length(), 0);
        QCOMPARE(request.versions().length(), 0);
    }
    {
        QWebSocketHandshakeRequest request(80, true);
        request.clear();
        QCOMPARE(request.port(), 80);
        QVERIFY(request.isSecure());
        QVERIFY(!request.isValid());
        QCOMPARE(request.extensions().length(), 0);
        QCOMPARE(request.protocols().length(), 0);
        QCOMPARE(request.headers().size(), 0);
        QCOMPARE(request.key().length(), 0);
        QCOMPARE(request.origin().length(), 0);
        QCOMPARE(request.host().length(), 0);
        QVERIFY(request.requestUrl().isEmpty());
        QCOMPARE(request.resourceName().length(), 0);
        QCOMPARE(request.versions().length(), 0);
    }
}

void tst_HandshakeRequest::tst_invalidStream_data()
{
    QTest::addColumn<QString>("dataStream");

    QTest::newRow("garbage on 2 lines") << "foofoofoo\r\nfoofoo\r\n\r\n";
    QTest::newRow("garbage on 1 line") << "foofoofoofoofoo";
    QTest::newRow("Correctly formatted but invalid fields") << "VERB RESOURCE PROTOCOL";

    //internally the fields are parsed and indexes are used to convert to a http version for instance
    //this test checks if there doesn't occur an out-of-bounds exception
    QTest::newRow("Correctly formatted but invalid short fields") << "V R P";
    QTest::newRow("Invalid \\0 character in header") << "V R\0 P";
    QTest::newRow("Invalid http version in header") << "V R HTTP/invalid";
    QTest::newRow("Empty header field") << "GET . HTTP/1.1\r\nHEADER: ";
    QTest::newRow("All zeros") << QString(QByteArray(10, char(0)));
    QTest::newRow("Invalid hostname") << "GET . HTTP/1.1\r\nHost: \xFF\xFF";
    QTest::newRow("Complete heaer - Invalid websocket version") << "GET . HTTP/1.1\r\nHost: foo\r\nSec-WebSocket-Version: \xFF\xFF\r\n" \
                                                                   "Sec-WebSocket-Key: AVDFBDDFF\r\n" \
                                                                   "Upgrade: websocket\r\n" \
                                                                   "Connection: Upgrade\r\n\r\n";
    QTest::newRow("Complete header - Invalid verb") << "XXX . HTTP/1.1\r\nHost: foo\r\nSec-WebSocket-Version: 13\r\n" \
                                                       "Sec-WebSocket-Key: AVDFBDDFF\r\n" \
                                                       "Upgrade: websocket\r\n" \
                                                       "Connection: Upgrade\r\n\r\n";
    QTest::newRow("Complete header - Invalid http version") << "GET . HTTP/a.1\r\nHost: foo\r\nSec-WebSocket-Version: 13\r\n" \
                                                               "Sec-WebSocket-Key: AVDFBDDFF\r\n" \
                                                               "Upgrade: websocket\r\n" \
                                                               "Connection: Upgrade\r\n\r\n";
    QTest::newRow("Complete header - Invalid connection") << "GET . HTTP/1.1\r\nHost: foo\r\nSec-WebSocket-Version: 13\r\n" \
                                                             "Sec-WebSocket-Key: AVDFBDDFF\r\n" \
                                                             "Upgrade: websocket\r\n" \
                                                             "Connection: xxxxxxx\r\n\r\n";
    QTest::newRow("Complete header - Invalid upgrade") << "GET . HTTP/1.1\r\nHost: foo\r\nSec-WebSocket-Version: 13\r\n" \
                                                          "Sec-WebSocket-Key: AVDFBDDFF\r\n" \
                                                          "Upgrade: wabsocket\r\n" \
                                                          "Connection: Upgrade\r\n\r\n";
    QTest::newRow("Complete header - Upgrade contains too many values") << "GET . HTTP/1.1\r\nHost: foo\r\nSec-WebSocket-Version: 13\r\n" \
                                                          "Sec-WebSocket-Key: AVDFBDDFF\r\n" \
                                                          "Upgrade: websocket,ftp\r\n" \
                                                          "Connection: Upgrade\r\n\r\n";
}

void tst_HandshakeRequest::tst_invalidStream()
{
    QFETCH(QString, dataStream);

    QByteArray data;
    QTextStream textStream(&data);
    QWebSocketHandshakeRequest request(80, true);

    textStream << dataStream;
    textStream.seek(0);
    textStream >> request;

    QVERIFY(!request.isValid());
    QCOMPARE(request.port(), 80);
    QVERIFY(request.isSecure());
    QCOMPARE(request.extensions().length(), 0);
    QCOMPARE(request.protocols().length(), 0);
    QCOMPARE(request.headers().size(), 0);
    QCOMPARE(request.key().length(), 0);
    QCOMPARE(request.origin().length(), 0);
    QCOMPARE(request.host().length(), 0);
    QVERIFY(request.requestUrl().isEmpty());
    QCOMPARE(request.resourceName().length(), 0);
    QCOMPARE(request.versions().length(), 0);
}

/*
 * This is a regression test
 * Checks for validity when more than one value is present in Connection
 */
void tst_HandshakeRequest::tst_multipleValuesInConnectionHeader()
{
    QString header = "GET /test HTTP/1.1\r\nHost: foo.com\r\nSec-WebSocket-Version: 13\r\n" \
                     "Sec-WebSocket-Key: AVDFBDDFF\r\n" \
                     "Upgrade: websocket\r\n" \
                     "Connection: Upgrade,keepalive\r\n\r\n";
    QByteArray data;
    QTextStream textStream(&data);
    QWebSocketHandshakeRequest request(80, false);

    textStream << header;
    textStream.seek(0);
    textStream >> request;

    QVERIFY(request.isValid());
    QCOMPARE(request.port(), 80);
    QVERIFY(!request.isSecure());
    QCOMPARE(request.extensions().length(), 0);
    QCOMPARE(request.protocols().length(), 0);
    QCOMPARE(request.headers().size(), 5);
    QCOMPARE(request.key().length(), 9);
    QCOMPARE(request.origin().length(), 0);
    QCOMPARE(request.requestUrl(), QUrl("ws://foo.com/test"));
    QCOMPARE(request.host(), QStringLiteral("foo.com"));
    QCOMPARE(request.resourceName().length(), 5);
    QCOMPARE(request.versions().length(), 1);
    QCOMPARE(request.versions().at(0), QWebSocketProtocol::V_13);
}

void tst_HandshakeRequest::tst_multipleVersions()
{
    QString header = "GET /test HTTP/1.1\r\nHost: foo.com\r\n" \
                     "Sec-WebSocket-Version: 4, 5, 6, 7, 8, 13\r\n" \
                     "Sec-WebSocket-Key: AVDFBDDFF\r\n" \
                     "Upgrade: websocket\r\n" \
                     "Connection: Upgrade,keepalive\r\n\r\n";
    QByteArray data;
    QTextStream textStream(&data);
    QWebSocketHandshakeRequest request(80, false);

    textStream << header;
    textStream.seek(0);
    textStream >> request;

    QVERIFY(request.isValid());
    QCOMPARE(request.port(), 80);
    QVERIFY(!request.isSecure());
    QCOMPARE(request.extensions().length(), 0);
    QCOMPARE(request.protocols().length(), 0);
    QCOMPARE(request.headers().size(), 5);
    QVERIFY(request.headers().contains(QStringLiteral("Host")));
    QVERIFY(request.headers().contains(QStringLiteral("Sec-WebSocket-Version")));
    QVERIFY(request.headers().contains(QStringLiteral("Sec-WebSocket-Key")));
    QVERIFY(request.headers().contains(QStringLiteral("Upgrade")));
    QVERIFY(request.headers().contains(QStringLiteral("Connection")));
    QCOMPARE(request.key(), QStringLiteral("AVDFBDDFF"));
    QCOMPARE(request.origin().length(), 0);
    QCOMPARE(request.requestUrl(), QUrl("ws://foo.com/test"));
    QCOMPARE(request.host(), QStringLiteral("foo.com"));
    QCOMPARE(request.resourceName().length(), 5);
    QCOMPARE(request.versions().length(), 6);
    //should be 13 since the list is ordered in decreasing order
    QCOMPARE(request.versions().at(0), QWebSocketProtocol::V_13);
}

QTEST_MAIN(tst_HandshakeRequest)

#include "tst_handshakerequest.moc"
