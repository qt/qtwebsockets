// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QtTest/QtTest>
#include <QtTest/qtestcase.h>
#include <QtCore/QDebug>
#include <QtCore/QByteArray>
#include <QtCore/QtEndian>

#include "private/qwebsockethandshakerequest_p.h"
#include "private/qwebsocketprotocol_p.h"
#include "QtWebSockets/qwebsocketprotocol.h"

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QWebSocketProtocol::CloseCode)
Q_DECLARE_METATYPE(QWebSocketProtocol::OpCode)

const int MAX_HEADERLINE_LENGTH = 8 * 1024;
const int MAX_HEADERS = 100;

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
    void tst_parsingWhitespaceInHeaders();

    void tst_qtbug_39355();
    void tst_qtbug_48123_data();
    void tst_qtbug_48123();

    void tst_qtbug_57357_data();
    void tst_qtbug_57357(); // ipv6 related
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
        QCOMPARE(request.extensions().size(), 0);
        QCOMPARE(request.protocols().size(), 0);
        QCOMPARE(request.headers().size(), 0);
        QCOMPARE(request.key().size(), 0);
        QCOMPARE(request.origin().size(), 0);
        QCOMPARE(request.host().size(), 0);
        QVERIFY(request.requestUrl().isEmpty());
        QCOMPARE(request.resourceName().size(), 0);
        QCOMPARE(request.versions().size(), 0);
    }
    {
        QWebSocketHandshakeRequest request(80, true);
        QCOMPARE(request.port(), 80);
        QVERIFY(request.isSecure());
        QVERIFY(!request.isValid());
        QCOMPARE(request.extensions().size(), 0);
        QCOMPARE(request.protocols().size(), 0);
        QCOMPARE(request.headers().size(), 0);
        QCOMPARE(request.key().size(), 0);
        QCOMPARE(request.origin().size(), 0);
        QCOMPARE(request.host().size(), 0);
        QVERIFY(request.requestUrl().isEmpty());
        QCOMPARE(request.resourceName().size(), 0);
        QCOMPARE(request.versions().size(), 0);
    }
    {
        QWebSocketHandshakeRequest request(80, true);
        request.clear();
        QCOMPARE(request.port(), 80);
        QVERIFY(request.isSecure());
        QVERIFY(!request.isValid());
        QCOMPARE(request.extensions().size(), 0);
        QCOMPARE(request.protocols().size(), 0);
        QCOMPARE(request.headers().size(), 0);
        QCOMPARE(request.key().size(), 0);
        QCOMPARE(request.origin().size(), 0);
        QCOMPARE(request.host().size(), 0);
        QVERIFY(request.requestUrl().isEmpty());
        QCOMPARE(request.resourceName().size(), 0);
        QCOMPARE(request.versions().size(), 0);
    }
}

void tst_HandshakeRequest::tst_invalidStream_data()
{
    QTest::addColumn<QString>("dataStream");

    QTest::newRow("garbage on 2 lines") << QStringLiteral("foofoofoo\r\nfoofoo\r\n\r\n");
    QTest::newRow("garbage on 1 line") << QStringLiteral("foofoofoofoofoo");
    QTest::newRow("Correctly formatted but invalid fields")
            << QStringLiteral("VERB RESOURCE PROTOCOL");

    //internally the fields are parsed and indexes are used to convert
    //to a http version for instance
    //this test checks if there doesn't occur an out-of-bounds exception
    QTest::newRow("Correctly formatted but invalid short fields") << QStringLiteral("V R P");
    QTest::newRow("Invalid \\0 character in header") << QStringLiteral("V R\0 P");
    QTest::newRow("Invalid HTTP version in header") << QStringLiteral("V R HTTP/invalid");
    QTest::newRow("Empty header field") << QStringLiteral("GET . HTTP/1.1\r\nHEADER: ");
    QTest::newRow("All zeros") << QString::fromUtf8(QByteArray(10, char(0)));
    QTest::newRow("Invalid hostname") << QStringLiteral("GET . HTTP/1.1\r\nHost: \xFF\xFF");
    //doing extensive QStringLiteral concatenations here, because
    //MSVC 2010 complains when using concatenation literal strings about
    //concatenation of wide and narrow strings (error C2308)
    QTest::newRow("Complete header - Invalid WebSocket version")
            << QStringLiteral("GET . HTTP/1.1\r\nHost: foo\r\nSec-WebSocket-Version: ") +
               QStringLiteral("\xFF\xFF\r\n") +
               QStringLiteral("Sec-WebSocket-Key: AVDFBDDFF\r\n") +
               QStringLiteral("Upgrade: websocket\r\n") +
               QStringLiteral("Connection: Upgrade\r\n\r\n");
    QTest::newRow("Complete header - Invalid verb")
            << QStringLiteral("XXX . HTTP/1.1\r\nHost: foo\r\nSec-WebSocket-Version: 13\r\n") +
               QStringLiteral("Sec-WebSocket-Key: AVDFBDDFF\r\n") +
               QStringLiteral("Upgrade: websocket\r\n") +
               QStringLiteral("Connection: Upgrade\r\n\r\n");
    QTest::newRow("Complete header - Invalid HTTP version")
            << QStringLiteral("GET . HTTP/a.1\r\nHost: foo\r\nSec-WebSocket-Version: 13\r\n") +
               QStringLiteral("Sec-WebSocket-Key: AVDFBDDFF\r\n") +
               QStringLiteral("Upgrade: websocket\r\n") +
               QStringLiteral("Connection: Upgrade\r\n\r\n");
    QTest::newRow("Complete header - Invalid connection")
            << QStringLiteral("GET . HTTP/1.1\r\nHost: foo\r\nSec-WebSocket-Version: 13\r\n") +
               QStringLiteral("Sec-WebSocket-Key: AVDFBDDFF\r\n") +
               QStringLiteral("Upgrade: websocket\r\n") +
               QStringLiteral("Connection: xxxxxxx\r\n\r\n");
    QTest::newRow("Complete header - Invalid upgrade")
            << QStringLiteral("GET . HTTP/1.1\r\nHost: foo\r\nSec-WebSocket-Version: 13\r\n") +
               QStringLiteral("Sec-WebSocket-Key: AVDFBDDFF\r\n") +
               QStringLiteral("Upgrade: wabsocket\r\n") +
               QStringLiteral("Connection: Upgrade\r\n\r\n");
    QTest::newRow("Complete header - Upgrade contains too many values")
            << QStringLiteral("GET . HTTP/1.1\r\nHost: foo\r\nSec-WebSocket-Version: 13\r\n") +
               QStringLiteral("Sec-WebSocket-Key: AVDFBDDFF\r\n") +
               QStringLiteral("Upgrade: websocket,ftp\r\n") +
               QStringLiteral("Connection: Upgrade\r\n\r\n");
    QTest::newRow("Invalid header - starts with continuation")
            << QStringLiteral("GET . HTTP/1.1\r\n Host: foo\r\nSec-WebSocket-Version: 13\r\n") +
               QStringLiteral("Sec-WebSocket-Key: AVDFBDDFF\r\n") +
               QStringLiteral("Upgrade: websocket\r\n") +
               QStringLiteral("Connection: Upgrade\r\n\r\n");
    QTest::newRow("Invalid header - no colon")
            << QStringLiteral("GET . HTTP/1.1\r\nHost: foo\r\nSec-WebSocket-Version: 13\r\n") +
               QStringLiteral("Sec-WebSocket-Key: AVDFBDDFF\r\n") +
               QStringLiteral("Upgrade: websocket\r\n") +
               QStringLiteral("X-Custom foo\r\n") +
               QStringLiteral("Connection: Upgrade\r\n\r\n");
}

void tst_HandshakeRequest::tst_invalidStream()
{
    QFETCH(QString, dataStream);

    QByteArray data;
    QTextStream(&data) << dataStream;
    QWebSocketHandshakeRequest request(80, true);
    request.readHandshake(data, MAX_HEADERLINE_LENGTH);

    QVERIFY(!request.isValid());
    QCOMPARE(request.port(), 80);
    QVERIFY(request.isSecure());
    QCOMPARE(request.extensions().size(), 0);
    QCOMPARE(request.protocols().size(), 0);
    QCOMPARE(request.headers().size(), 0);
    QCOMPARE(request.key().size(), 0);
    QCOMPARE(request.origin().size(), 0);
    QCOMPARE(request.host().size(), 0);
    QVERIFY(request.requestUrl().isEmpty());
    QCOMPARE(request.resourceName().size(), 0);
    QCOMPARE(request.versions().size(), 0);
}

/*
 * This is a regression test
 * Checks for validity when more than one value is present in Connection
 */
void tst_HandshakeRequest::tst_multipleValuesInConnectionHeader()
{
    //doing extensive QStringLiteral concatenations here, because
    //MSVC 2010 complains when using concatenation literal strings about
    //concatenation of wide and narrow strings (error C2308)
    QString header = QStringLiteral("GET /test HTTP/1.1\r\nHost: ") +
                     QStringLiteral("foo.com\r\nSec-WebSocket-Version: 13\r\n") +
                     QStringLiteral("Sec-WebSocket-Key: AVDFBDDFF\r\n") +
                     QStringLiteral("Upgrade: websocket\r\n") +
                     QStringLiteral("Connection: Upgrade,keepalive\r\n\r\n");
    QByteArray data;
    QTextStream(&data) << header;
    QWebSocketHandshakeRequest request(80, false);
    request.readHandshake(data, MAX_HEADERLINE_LENGTH);

    QVERIFY(request.isValid());
    QCOMPARE(request.port(), 80);
    QVERIFY(!request.isSecure());
    QCOMPARE(request.extensions().size(), 0);
    QCOMPARE(request.protocols().size(), 0);
    QCOMPARE(request.headers().size(), 5);
    QCOMPARE(request.key().size(), 9);
    QCOMPARE(request.origin().size(), 0);
    QCOMPARE(request.requestUrl(), QUrl("ws://foo.com/test"));
    QCOMPARE(request.host(), QStringLiteral("foo.com"));
    QCOMPARE(request.resourceName().size(), 5);
    QCOMPARE(request.versions().size(), 1);
    QCOMPARE(request.versions().at(0), QWebSocketProtocol::Version13);
}

/*
 * This is a regression test
 * Checks for RFC compliant header parsing
 */
void tst_HandshakeRequest::tst_parsingWhitespaceInHeaders()
{
    //doing extensive QStringLiteral concatenations here, because
    //MSVC 2010 complains when using concatenation literal strings about
    //concatenation of wide and narrow strings (error C2308)
    QString header = QStringLiteral("GET /test HTTP/1.1\r\nHost: ") +
                     QStringLiteral("foo.com\r\nSec-WebSocket-Version:13\r\n") +
                     QStringLiteral("Sec-WebSocket-Key:   AVD  \r\n\tFBDDFF \r\n") +
                     QStringLiteral("Upgrade:websocket \r\n") +
                     QStringLiteral("Connection: Upgrade,keepalive\r\n\r\n");
    QByteArray data;
    QTextStream(&data) << header;
    QWebSocketHandshakeRequest request(80, false);
    request.readHandshake(data, MAX_HEADERLINE_LENGTH);

    QVERIFY(request.isValid());
    QCOMPARE(request.key(), QStringLiteral("AVD FBDDFF"));
    QCOMPARE(request.versions().size(), 1);
    QCOMPARE(request.versions().at(0), QWebSocketProtocol::Version13);
}

void tst_HandshakeRequest::tst_multipleVersions()
{
    QString header = QStringLiteral("GET /test HTTP/1.1\r\nHost: foo.com\r\n") +
                     QStringLiteral("Sec-WebSocket-Version: 4, 5, 6, 7, 8, 13\r\n") +
                     QStringLiteral("Sec-WebSocket-Key: AVDFBDDFF\r\n") +
                     QStringLiteral("Upgrade: websocket\r\n") +
                     QStringLiteral("Connection: Upgrade,keepalive\r\n\r\n");
    QByteArray data;
    QTextStream(&data) << header;
    QWebSocketHandshakeRequest request(80, false);
    request.readHandshake(data, MAX_HEADERLINE_LENGTH);

    QVERIFY(request.isValid());
    QCOMPARE(request.port(), 80);
    QVERIFY(!request.isSecure());
    QCOMPARE(request.extensions().size(), 0);
    QCOMPARE(request.protocols().size(), 0);
    QCOMPARE(request.headers().size(), 5);
    QVERIFY(request.hasHeader("host"));
    QVERIFY(request.hasHeader("sec-websocket-version"));
    QVERIFY(request.hasHeader("sec-websocket-key"));
    QVERIFY(request.hasHeader("upgrade"));
    QVERIFY(request.hasHeader("connection"));
    QCOMPARE(request.key(), QStringLiteral("AVDFBDDFF"));
    QCOMPARE(request.origin().size(), 0);
    QCOMPARE(request.requestUrl(), QUrl("ws://foo.com/test"));
    QCOMPARE(request.host(), QStringLiteral("foo.com"));
    QCOMPARE(request.resourceName().size(), 5);
    QCOMPARE(request.versions().size(), 6);
    //should be 13 since the list is ordered in decreasing order
    QCOMPARE(request.versions().at(0), QWebSocketProtocol::Version13);
}

void tst_HandshakeRequest::tst_qtbug_39355()
{
    QString header = QStringLiteral("GET /ABC/DEF/ HTTP/1.1\r\nHost: localhost:1234\r\n") +
                     QStringLiteral("Sec-WebSocket-Version: 13\r\n") +
                     QStringLiteral("Sec-WebSocket-Key: 2Wg20829/4ziWlmsUAD8Dg==\r\n") +
                     QStringLiteral("Upgrade: websocket\r\n") +
                     QStringLiteral("Connection: Upgrade\r\n\r\n");
    QByteArray data;
    QTextStream(&data) << header;
    QWebSocketHandshakeRequest request(8080, false);
    request.readHandshake(data, MAX_HEADERLINE_LENGTH);

    QVERIFY(request.isValid());
    QCOMPARE(request.port(), 1234);
    QCOMPARE(request.host(), QStringLiteral("localhost"));
}

void tst_HandshakeRequest::tst_qtbug_48123_data()
{
    QTest::addColumn<QString>("header");
    QTest::addColumn<bool>("shouldBeValid");
    const QString header = QStringLiteral("GET /ABC/DEF/ HTTP/1.1\r\nHost: localhost:1234\r\n") +
                           QStringLiteral("Sec-WebSocket-Version: 13\r\n") +
                           QStringLiteral("Sec-WebSocket-Key: 2Wg20829/4ziWlmsUAD8Dg==\r\n") +
                           QStringLiteral("Upgrade: websocket\r\n") +
                           QStringLiteral("Connection: Upgrade\r\n");
    const int numHeaderLines = header.count(QStringLiteral("\r\n")) - 1; //-1: exclude requestline

    // a headerline must contain colon
    QString illegalHeader = header;
    illegalHeader.append(QString(MAX_HEADERLINE_LENGTH, QLatin1Char('c')));
    illegalHeader.append(QStringLiteral("\r\n\r\n"));

    QTest::newRow("headerline missing colon") << illegalHeader << false;

    // a headerline should not be larger than MAX_HEADERLINE_LENGTH characters (excluding CRLF)
    QString tooLongHeader = header;
    QString fieldName = "Too-long: ";
    tooLongHeader.append(fieldName);
    tooLongHeader.append(QString(MAX_HEADERLINE_LENGTH + 1 - fieldName.size(), QLatin1Char('c')));
    tooLongHeader.append(QStringLiteral("\r\n\r\n"));

    QTest::newRow("headerline too long") << tooLongHeader << false;

    QString legalHeader = header;
    const QString headerKey = QStringLiteral("X-CUSTOM-KEY: ");
    legalHeader.append(headerKey);
    legalHeader.append(QString(MAX_HEADERLINE_LENGTH - headerKey.size(), QLatin1Char('c')));
    legalHeader.append(QStringLiteral("\r\n\r\n"));

    QTest::newRow("headerline with maximum length") << legalHeader << true;

    //a header should not contain more than MAX_HEADERS header lines (excluding the request line)
    //test with MAX_HEADERS + 1
    illegalHeader = header;
    const QString headerLine(QStringLiteral("Host: localhost:1234\r\n"));
    for (int i = 0; i < (MAX_HEADERS - numHeaderLines + 1); ++i) {
        illegalHeader.append(headerLine);
    }
    illegalHeader.append(QStringLiteral("\r\n"));

    QTest::newRow("too many headerlines") << illegalHeader << false;

    //test with MAX_HEADERS header lines (excluding the request line)
    legalHeader = header;
    for (int i = 0; i < (MAX_HEADERS - numHeaderLines); ++i) {
        legalHeader.append(headerLine);
    }
    legalHeader.append(QStringLiteral("\r\n"));

    QTest::newRow("just enough headerlines") << legalHeader << true;
}

void tst_HandshakeRequest::tst_qtbug_48123()
{
    QFETCH(QString, header);
    QFETCH(bool, shouldBeValid);

    QByteArray data;
    QTextStream(&data) << header;
    QWebSocketHandshakeRequest request(8080, false);
    request.readHandshake(data, MAX_HEADERLINE_LENGTH);

    QCOMPARE(request.isValid(), shouldBeValid);
}

void tst_HandshakeRequest::tst_qtbug_57357_data()
{
    QTest::addColumn<QString>("header");
    QTest::addColumn<bool>("valid");
    QTest::addColumn<QString>("host");
    QTest::addColumn<int>("port");

    QString header = QLatin1String("GET /ABC/DEF/ HTTP/1.1\r\nHost: %1%2\r\n"
                                   "Sec-WebSocket-Version: 13\r\n"
                                   "Sec-WebSocket-Key: 2Wg20829/4ziWlmsUAD8Dg==\r\n"
                                   "Upgrade: websocket\r\n"
                                   "Connection: Upgrade\r\n\r\n");

    QTest::newRow("ipv4-1") << header.arg(QStringLiteral("10.0.0.1")).arg(QStringLiteral(":1234")) << true
                            << QStringLiteral("10.0.0.1")
                            << 1234;
    QTest::newRow("ipv4-2") << header.arg(QStringLiteral("127.0.0.1")).arg(QStringLiteral(":1111")) << true
                            << QStringLiteral("127.0.0.1")
                            << 1111;
    QTest::newRow("ipv4-wo-port") << header.arg(QStringLiteral("10.0.0.1")).arg(QStringLiteral("")) << true
                            << QStringLiteral("10.0.0.1")
                            << 8080;

    QTest::newRow("ipv6-1") << header.arg(QStringLiteral("[56:56:56:56:56:56:56:56]")).arg(QStringLiteral(":1234")) << true
                            << QStringLiteral("56:56:56:56:56:56:56:56")
                            << 1234;
    QTest::newRow("ipv6-2") << header.arg(QStringLiteral("[::ffff:129.144.52.38]")).arg(QStringLiteral(":1111")) << true
                            << QStringLiteral("::ffff:129.144.52.38")
                            << 1111;
    QTest::newRow("ipv6-wo-port") << header.arg(QStringLiteral("[56:56:56:56:56:56:56:56]")).arg(QStringLiteral("")) << true
                            << QStringLiteral("56:56:56:56:56:56:56:56")
                            << 8080;
    QTest::newRow("ipv6-invalid-1") << header.arg(QStringLiteral("56:56:56:56:56:56:56:56]")).arg(QStringLiteral(":1234")) << false
                            << QStringLiteral("")
                            << 1234;

    QTest::newRow("host-1") << header.arg(QStringLiteral("foo.com")).arg(QStringLiteral(":1234")) << true
                            << QStringLiteral("foo.com")
                            << 1234;
    QTest::newRow("host-2") << header.arg(QStringLiteral("bar.net")).arg(QStringLiteral(":1111")) << true
                            << QStringLiteral("bar.net")
                            << 1111;
    QTest::newRow("host-wo-port") << header.arg(QStringLiteral("foo.com")).arg(QStringLiteral("")) << true
                            << QStringLiteral("foo.com")
                            << 8080;

    QTest::newRow("localhost-1") << header.arg(QStringLiteral("localhost")).arg(QStringLiteral(":1234")) << true
                            << QStringLiteral("localhost")
                            << 1234;
    QTest::newRow("localhost-2") << header.arg(QStringLiteral("localhost")).arg(QStringLiteral(":1111")) << true
                            << QStringLiteral("localhost")
                            << 1111;
    QTest::newRow("localhost-wo-port") << header.arg(QStringLiteral("localhost")).arg(QStringLiteral("")) << true
                            << QStringLiteral("localhost")
                            << 8080;

    // reference: qtbase/tests/auto/corelib/io/qurl/tst_qurl.cpp: void tst_QUrl::ipvfuture_data()
    QTest::newRow("ipvfuture-1") << header.arg(QStringLiteral("[v7.1234]")).arg(QStringLiteral(":1234")) << true
                            << QStringLiteral("v7.1234")
                            << 1234;

    QTest::newRow("invalid-1") << header.arg(QStringLiteral("abc:def@foo.com")).arg(QStringLiteral("")) << false
                            << QStringLiteral("foo.com")
                            << 8080;
    QTest::newRow("invalid-2") << header.arg(QStringLiteral(":def@foo.com")).arg(QStringLiteral("")) << false
                            << QStringLiteral("foo.com")
                            << 8080;
    QTest::newRow("invalid-3") << header.arg(QStringLiteral("abc:@foo.com")).arg(QStringLiteral("")) << false
                            << QStringLiteral("foo.com")
                            << 8080;
    QTest::newRow("invalid-4") << header.arg(QStringLiteral("@foo.com")).arg(QStringLiteral("")) << false
                            << QStringLiteral("foo.com")
                            << 8080;
    QTest::newRow("invalid-5") << header.arg(QStringLiteral("foo.com/")).arg(QStringLiteral("")) << false
                            << QStringLiteral("foo.com")
                            << 8080;
}

void tst_HandshakeRequest::tst_qtbug_57357()
{
    QFETCH(QString, header);
    QFETCH(bool, valid);
    QFETCH(QString, host);
    QFETCH(int, port);

    QByteArray data;
    QTextStream(&data) << header;
    QWebSocketHandshakeRequest request(8080, false);
    request.readHandshake(data, MAX_HEADERLINE_LENGTH);

    QCOMPARE(request.isValid(), valid);
    if (valid) {
        QCOMPARE(request.host(), host);
        QCOMPARE(request.port(), port);
    }
}

QTEST_MAIN(tst_HandshakeRequest)

#include "tst_handshakerequest.moc"
