#include <QtTest/QtTest>
#include <QtTest/qtestcase.h>
#include <QDebug>

#include "qwebsocketprotocol.h"

Q_DECLARE_METATYPE(QWebSocketProtocol::CloseCode)
Q_DECLARE_METATYPE(QWebSocketProtocol::OpCode)

QT_USE_NAMESPACE

class tst_WebSocketProtocol : public QObject
{
    Q_OBJECT

public:
    tst_WebSocketProtocol();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void tst_validMasks_data();
    void tst_validMasks();

    void tst_opCodes_data();
    void tst_opCodes();

    void tst_closeCodes_data();
    void tst_closeCodes();
};

tst_WebSocketProtocol::tst_WebSocketProtocol()
{}

void tst_WebSocketProtocol::initTestCase()
{
}

void tst_WebSocketProtocol::cleanupTestCase()
{}

void tst_WebSocketProtocol::init()
{
    qRegisterMetaType<QWebSocketProtocol::OpCode>("QWebSocketProtocol::OpCode");
    qRegisterMetaType<QWebSocketProtocol::CloseCode>("QWebSocketProtocol::CloseCode");
}

void tst_WebSocketProtocol::cleanup()
{
}

void tst_WebSocketProtocol::tst_validMasks_data()
{
    QTest::addColumn<quint32>("mask");
    QTest::addColumn<QString>("inputdata");
    QTest::addColumn<QByteArray>("result");

    QTest::newRow("Empty payload") << 0x12345678u << QString("") << QByteArray("");
    QTest::newRow("ASCII payload of 8 characters") << 0x12345678u << QString("abcdefgh") << QByteArray("\x19\x34\x57\x76\x1D\x30\x53\x7A");
    QTest::newRow("ASCII payload of 9 characters") << 0x12345678u << QString("abcdefghi") << QByteArray("\x19\x34\x57\x76\x1D\x30\x53\x7A\x11");
    QTest::newRow("UTF-8 payload") << 0x12345678u << QString("∫∂ƒ©øØ") << QByteArray("\x47\x69\x0B\xBB\x80\x8E");
}

void tst_WebSocketProtocol::tst_validMasks()
{
    QFETCH(quint32, mask);
    QFETCH(QString, inputdata);
    QFETCH(QByteArray, result);

    char *data = inputdata.toLatin1().data();

    QWebSocketProtocol::mask(data, inputdata.size(), mask);
    QCOMPARE(QByteArray::fromRawData(data, inputdata.size()), result);
}

void tst_WebSocketProtocol::tst_opCodes_data()
{
    QTest::addColumn<QWebSocketProtocol::OpCode>("opCode");
    QTest::addColumn<bool>("isReserved");

    QTest::newRow("OC_BINARY") << QWebSocketProtocol::OC_BINARY << false;
    QTest::newRow("OC_CLOSE") << QWebSocketProtocol::OC_CLOSE << false;
    QTest::newRow("OC_CONTINUE") << QWebSocketProtocol::OC_CONTINUE << false;
    QTest::newRow("OC_PING") << QWebSocketProtocol::OC_PING << false;
    QTest::newRow("OC_PONG") << QWebSocketProtocol::OC_PONG << false;
    QTest::newRow("OC_RESERVED3") << QWebSocketProtocol::OC_RESERVED_3 << true;
    QTest::newRow("OC_RESERVED4") << QWebSocketProtocol::OC_RESERVED_4 << true;
    QTest::newRow("OC_RESERVED5") << QWebSocketProtocol::OC_RESERVED_5 << true;
    QTest::newRow("OC_RESERVED6") << QWebSocketProtocol::OC_RESERVED_6 << true;
    QTest::newRow("OC_RESERVED7") << QWebSocketProtocol::OC_RESERVED_7 << true;
    QTest::newRow("OC_RESERVEDB") << QWebSocketProtocol::OC_RESERVED_B << true;
    QTest::newRow("OC_RESERVEDC") << QWebSocketProtocol::OC_RESERVED_C << true;
    QTest::newRow("OC_RESERVEDD") << QWebSocketProtocol::OC_RESERVED_D << true;
    QTest::newRow("OC_RESERVEDE") << QWebSocketProtocol::OC_RESERVED_E << true;
    QTest::newRow("OC_RESERVEDF") << QWebSocketProtocol::OC_RESERVED_F << true;
    QTest::newRow("OC_TEXT") << QWebSocketProtocol::OC_TEXT << false;
}

void tst_WebSocketProtocol::tst_opCodes()
{
    QFETCH(QWebSocketProtocol::OpCode, opCode);
    QFETCH(bool, isReserved);

    bool result = QWebSocketProtocol::isOpCodeReserved(opCode);

    QCOMPARE(result, isReserved);
}

void tst_WebSocketProtocol::tst_closeCodes_data()
{
    QTest::addColumn<int>("closeCode");
    QTest::addColumn<bool>("isValid");

    for (int i = 0; i < 1000; ++i)
    {
        QTest::newRow(QString("Close code %1").arg(i).toLatin1().constData()) << i << false;
    }

    for (int i = 1000; i < 1004; ++i)
    {
        QTest::newRow(QString("Close code %1").arg(i).toLatin1().constData()) << i << true;
    }

    QTest::newRow("Close code 1004") << 1004 << false;
    QTest::newRow("Close code 1005") << 1005 << false;
    QTest::newRow("Close code 1006") << 1006 << false;

    for (int i = 1007; i < 1012; ++i)
    {
        QTest::newRow(QString("Close code %1").arg(i).toLatin1().constData()) << i << true;
    }

    for (int i = 1013; i < 3000; ++i)
    {
        QTest::newRow(QString("Close code %1").arg(i).toLatin1().constData()) << i << false;
    }

    for (int i = 3000; i < 5000; ++i)
    {
        QTest::newRow(QString("Close code %1").arg(i).toLatin1().constData()) << i << true;
    }

    QTest::newRow("Close code 5000") << 1004 << false;
    QTest::newRow("Close code 6000") << 1004 << false;
    QTest::newRow("Close code 7000") << 1004 << false;
}

void tst_WebSocketProtocol::tst_closeCodes()
{
    QFETCH(int, closeCode);
    QFETCH(bool, isValid);

    bool result = QWebSocketProtocol::isCloseCodeValid(closeCode);

    QCOMPARE(result, isValid);
}

QTEST_MAIN(tst_WebSocketProtocol)

#include "tst_websocketprotocol.moc"

