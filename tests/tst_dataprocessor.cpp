#include <QtTest/QtTest>
#include <QtTest/qtestcase.h>
#include <QSignalSpy>
#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include "dataprocessor_p.h"
#include "unittests.h"

Q_DECLARE_METATYPE(QWebSocketProtocol::CloseCode)

const quint8 FIN = 0x80;
const quint8 RSV1 = 0x40;
const quint8 RSV2 = 0x30;
const quint8 RSV3 = 0x10;
const quint8 MASK = 0x80;

class tst_DataProcessor : public QObject
{
    Q_OBJECT

public:
    tst_DataProcessor();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    /*!
      \brief Tests all kinds of valid binary frames, including zero length frames
      */
    void goodBinaryFrames();

    /*!
      \brief Tests all kinds of valid text frames, including zero length frames
      */
    void goodTextFrames();

    /*!
        \brief Tests the DataProcessor for correct handling of frames that don't contain the starting 2 bytes.
        This test does not test sequences of frames, only single frames are tested
     */
    void frameTooSmall();
    /*!
        \brief Tests the DataProcessor for correct handling of malformed frame headers.
        This test does not test sequences of frames, only single frames are tested
     */
    void invalidHeader();
    /*!
        \brief Tests the DataProcessor for correct handling of incomplete payloads.
        This includes:
        - incomplete length bytes for large and big payloads (16- and 64-bit values),
        - minimum size representation (see RFC 6455 paragraph 5.2),
        - frames that are too large (larger than MAX_INT in bytes)
        - incomplete payloads (less bytes than indicated in the size field)
        - invalid UTF-8 sequences in text frames
        This test does not test sequences of frames, only single frames are tested
     */
    void incompletePayload();

    /*!
        \brief Tests the DataProcessor for correct handling of incorrect sequences of frames.
        This test does not test the welformedness of frames, only incorrect sequences,
        e.g. received a continuation frame without a preceding start frame
     */
    //void testBadSequenceOfFrames();

    void invalidHeader_data();
    void incompletePayload_data();
};

tst_DataProcessor::tst_DataProcessor()
{
}

void tst_DataProcessor::initTestCase()
{
    qRegisterMetaType<QWebSocketProtocol::CloseCode>("QWebSocketProtocol::CloseCode");
}

void tst_DataProcessor::cleanupTestCase()
{
}

void tst_DataProcessor::init()
{
}

void tst_DataProcessor::cleanup()
{
}

void tst_DataProcessor::goodBinaryFrames()
{
    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;

    //empty binary payload; this should be OK
    data.append((char)(FIN | QWebSocketProtocol::OC_BINARY)).append(char(0x0));
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);

    QSignalSpy spyFrameReceived(&dataProcessor, SIGNAL(binaryFrameReceived(QByteArray,bool)));
    QSignalSpy spyMessageReceived(&dataProcessor, SIGNAL(binaryMessageReceived(QByteArray)));
    dataProcessor.process(&buffer);
    QCOMPARE(spyFrameReceived.count(), 1);
    QCOMPARE(spyMessageReceived.count(), 1);
    QList<QVariant> arguments = spyFrameReceived.takeFirst();
    QCOMPARE(arguments.at(0).toByteArray().length(), 0);
    arguments = spyMessageReceived.takeFirst();
    QCOMPARE(arguments.at(0).toByteArray().length(), 0);
    buffer.close();
    spyFrameReceived.clear();
    spyMessageReceived.clear();
    data.clear();
}

void tst_DataProcessor::goodTextFrames()
{
    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;

    //empty text payload; this should be OK
    data.append((char)(FIN | QWebSocketProtocol::OC_TEXT)).append(char(0x0));
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);

    QSignalSpy spyFrameReceived(&dataProcessor, SIGNAL(textFrameReceived(QString,bool)));
    QSignalSpy spyMessageReceived(&dataProcessor, SIGNAL(textMessageReceived(QString)));
    dataProcessor.process(&buffer);
    QCOMPARE(spyFrameReceived.count(), 1);
    QCOMPARE(spyMessageReceived.count(), 1);
    QList<QVariant> arguments = spyFrameReceived.takeFirst();
    QCOMPARE(arguments.at(0).toString().length(), 0);
    arguments = spyMessageReceived.takeFirst();
    QCOMPARE(arguments.at(0).toString().length(), 0);
    buffer.close();
    spyFrameReceived.clear();
    spyMessageReceived.clear();
    data.clear();
}

void tst_DataProcessor::frameTooSmall()
{
    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;

    //with nothing in the buffer, the dataProcessor should time out and the error should be CC_GOING_AWAY
    //meaning the socket will be closed
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);
    QSignalSpy spy(&dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)));
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_GOING_AWAY);
    spy.clear();
    buffer.close();
    data.clear();

    //only one byte; this is far too little; should get a time out as well and the error should be CC_GOING_AWAY
    //meaning the socket will be closed
    data.append(quint8('1'));   //put 1 byte in the buffer; this is too little
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_GOING_AWAY);
    buffer.close();
    spy.clear();
    data.clear();
}

void tst_DataProcessor::invalidHeader_data()
{
    //The first byte contain the FIN, RSV1, RSV2, RSV3 and the Opcode
    //The second byte contains the MaskFlag and the length of the frame
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    //invalid bit fields
    QTest::newRow("RSV1 set")                << quint8(FIN | RSV1)              << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("RSV2 set")                << quint8(FIN | RSV2)              << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("RSV3 set")                << quint8(FIN | RSV3)              << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("RSV1 and RSV2 set")       << quint8(FIN | RSV1 | RSV2)       << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("RSV1 and RSV3 set")       << quint8(FIN | RSV1 | RSV3)       << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("RSV2 and RSV3 set")       << quint8(FIN | RSV2 | RSV3)       << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("RSV1, RSV2 and RSV3 set") << quint8(FIN | RSV1 |RSV2 | RSV3) << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;

    //invalid opcodes
    QTest::newRow("Invalid OpCode 3") << quint8(FIN | QWebSocketProtocol::OC_RESERVED_3) << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Invalid OpCode 4") << quint8(FIN | QWebSocketProtocol::OC_RESERVED_4) << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Invalid OpCode 5") << quint8(FIN | QWebSocketProtocol::OC_RESERVED_5) << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Invalid OpCode 6") << quint8(FIN | QWebSocketProtocol::OC_RESERVED_6) << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Invalid OpCode 7") << quint8(FIN | QWebSocketProtocol::OC_RESERVED_7) << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Invalid OpCode B") << quint8(FIN | QWebSocketProtocol::OC_RESERVED_B) << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Invalid OpCode C") << quint8(FIN | QWebSocketProtocol::OC_RESERVED_C) << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Invalid OpCode D") << quint8(FIN | QWebSocketProtocol::OC_RESERVED_D) << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Invalid OpCode E") << quint8(FIN | QWebSocketProtocol::OC_RESERVED_E) << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Invalid OpCode F") << quint8(FIN | QWebSocketProtocol::OC_RESERVED_F) << quint8(0x00) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
}

void tst_DataProcessor::invalidHeader()
{
    QFETCH(quint8, firstByte);
    QFETCH(quint8, secondByte);
    QFETCH(QWebSocketProtocol::CloseCode, expectedCloseCode);

    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;
    QSignalSpy spy(&dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)));

    data.append(char(firstByte)).append(char(secondByte));
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    QVariantList arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), expectedCloseCode);
    buffer.close();
    spy.clear();
    data.clear();
}

void tst_DataProcessor::incompletePayload_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    QTest::newRow("Text frame with payload size 125, but no data")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(125) << QByteArray() << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Binary frame with payloadsize 125, but no data")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(125) << QByteArray() << QWebSocketProtocol::CC_GOING_AWAY;

    //for a frame length value of 126, there should be 2 bytes following to form a 16-bit frame length
    QTest::newRow("Text frame with payload size 126, but no data")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(126) << QByteArray() << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Binary frame with payloadsize 126, but only 1 byte following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(126) << QByteArray().append(quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;

    //for a frame length value of 127, there should be 8 bytes following to form a 64-bit frame length
    QTest::newRow("Text frame with payload size 127, but no data")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray() << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Binary frame with payloadsize 127, but only 1 byte following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray(1, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Text frame with payload size 127, but only 2 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray(2, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Text frame with payload size 127, but only 3 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray(3, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Text frame with payload size 127, but only 4 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray(4, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Text frame with payload size 127, but only 5 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray(5, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Text frame with payload size 127, but only 6 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray(6, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Text frame with payload size 127, but only 7 bytes following")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY) << quint8(127) << QByteArray(7, quint8(1)) << QWebSocketProtocol::CC_GOING_AWAY;

    //testing for the minimum size representation requirement; see RFC 6455 para 5.2
    quint16 swapped16 = qToBigEndian<quint16>(0);
    const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped16));
    QTest::newRow("Text frame with payload size 0, represented in 2 bytes")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(126) << QByteArray(wireRepresentation, 2) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped16 = qToBigEndian<quint16>(64);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped16));
    QTest::newRow("Text frame with payload size 64, represented in 2 bytes")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(126) << QByteArray(wireRepresentation, 2) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped16 = qToBigEndian<quint16>(125);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped16));
    QTest::newRow("Text frame with payload size 125, represented in 2 bytes")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(126) << QByteArray(wireRepresentation, 2) << QWebSocketProtocol::CC_PROTOCOL_ERROR;

    QTest::newRow("Text frame with payload size 0, represented in 8 bytes")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(8, quint8(0)) << QWebSocketProtocol::CC_PROTOCOL_ERROR;

    quint64 swapped = qToBigEndian<quint64>(64);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Text frame with payload size 64, represented in 8 bytes")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(wireRepresentation, 8) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint64>(125);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Text frame with payload size 125, represented in 8 bytes")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(wireRepresentation, 8) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint64>(8192);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Text frame with payload size 8192, represented in 8 bytes")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(wireRepresentation, 8) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint64>(16384);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Text frame with payload size 16384, represented in 8 bytes")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(wireRepresentation, 8) << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint64>(0xFFFFu -1);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Text frame with payload size 65535, represented in 8 bytes")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(wireRepresentation, 8) << QWebSocketProtocol::CC_PROTOCOL_ERROR;

    //test frames with payloads that are too small; should result in timeout
    QTest::newRow("Payload size 64, but only 32 bytes of data")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(64) << QByteArray(32, 'a') << QWebSocketProtocol::CC_GOING_AWAY;
    swapped16 = qToBigEndian<quint16>(256);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped16));
    QTest::newRow("Payload size 256, but only 32 bytes of data")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(126) << QByteArray(wireRepresentation, 2).append(QByteArray(32, 'a')) << QWebSocketProtocol::CC_GOING_AWAY;
    swapped = qToBigEndian<quint64>(128000);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Payload size 128000, but only 32 bytes of data")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(wireRepresentation, 8).append(QByteArray(32, 'a')) << QWebSocketProtocol::CC_GOING_AWAY;

    swapped = qToBigEndian<quint64>(quint64(INT_MAX + 1));
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Payload size > INT_MAX")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT) << quint8(127) << QByteArray(wireRepresentation, 8).append(QByteArray(32, 'a')) << QWebSocketProtocol::CC_TOO_MUCH_DATA;
    //TODO: test for invalid payloads, i.e. UTF-8; see Autobahn
}

void tst_DataProcessor::incompletePayload()
{
    QFETCH(quint8, firstByte);
    QFETCH(quint8, secondByte);
    QFETCH(QByteArray, payload);
    QFETCH(QWebSocketProtocol::CloseCode, expectedCloseCode);

    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;
    QSignalSpy spy(&dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)));

    data.append(firstByte).append(secondByte);
    data.append(payload);
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    QVariantList arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), expectedCloseCode);
    buffer.close();
    spy.clear();
    data.clear();
}

//TODO: test on valid and invalid handshakes; like the errors I got with FireFox (multiple values in field)

DECLARE_TEST(tst_DataProcessor)

#include "tst_dataprocessor.moc"

