#include <QtTest/QtTest>
#include <QtTest/qtestcase.h>
#include <QSignalSpy>
#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include "dataprocessor_p.h"
#include "unittests.h"
Q_DECLARE_METATYPE(QWebSocketProtocol::CloseCode);

class DataProcessorTest : public QObject
{
    Q_OBJECT

public:
    DataProcessorTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    /*!
        \brief Tests the DataProcessor for correct handling of malformed frames.
        This test does not test sequences of frames, only single frames are tested
     */
    void testMalformedFrames();

    /*!
        \brief Tests the DataProcessor for correct handling of incorrect sequences of frames.
        This test does not test the welformedness of frames, only incorrect sequences, e.g. received a continuation frame without a preceding start frame
     */
    //void testBadSequenceOfFrames();
};

DataProcessorTest::DataProcessorTest()
{
}

void DataProcessorTest::initTestCase()
{
    qRegisterMetaType<QWebSocketProtocol::CloseCode>("QWebSocketProtocol::CloseCode");
}

void DataProcessorTest::cleanupTestCase()
{
}

void DataProcessorTest::init()
{
}

void DataProcessorTest::cleanup()
{
}

union FrameHeaderBytes
{
    int FIN     : 1;
    int RSV1    : 1;
    int RSV2    : 1;
    int RSV3    : 1;
    int OpCode  : 4;
    int Mask    : 1;
    int Len     : 7;
};

const quint8 FIN = 0x80;
const quint8 RSV1 = 0x40;
const quint8 RSV2 = 0x30;
const quint8 RSV3 = 0x10;
const quint8 MASK = 0x80;

void DataProcessorTest::testMalformedFrames()
{
    QByteArray data;
    QBuffer buffer;
    buffer.setData(data);
    buffer.open(QIODevice::ReadWrite);
    DataProcessor dataProcessor;

    //with nothing in the buffer, the dataProcessor should time out and the error should be CC_GOING_AWAY
    QSignalSpy spy(&dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)));
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_GOING_AWAY);

    //only one byte; this is far too little; should get a time out as well
    buffer.putChar('1');
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_GOING_AWAY);

    //The first byte contain the FIN, RSV1, RSV2, RSV3 and the Opcode
    //The second byte contains the MaskFlag and the length of the frame
    spy.clear();
    data.clear();
    data.append((char)(FIN | RSV1)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    //now check with RSV2 bit set
    spy.clear();
    data.clear();
    data.append((char)(FIN | RSV2)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    //now check with RSV3 bit set
    spy.clear();
    data.clear();
    data.append((char)(FIN | RSV3)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    //now check with RSV1 and RSV2 bit set
    spy.clear();
    data.clear();
    data.append((char)(FIN | RSV1 | RSV2)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    //now check with RSV1 and RSV3 bit set
    spy.clear();
    data.clear();
    data.append((char)(FIN | RSV1 | RSV3)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    //now check with RSV2 and RSV3 bit set
    spy.clear();
    data.clear();
    data.append((char)(FIN | RSV2 | RSV3)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    //check on invalid opcodes
    spy.clear();
    data.clear();
    data.append((char)(FIN | QWebSocketProtocol::OC_RESERVED_3)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    spy.clear();
    data.clear();
    data.append((char)(FIN | QWebSocketProtocol::OC_RESERVED_4)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    spy.clear();
    data.clear();
    data.append((char)(FIN | QWebSocketProtocol::OC_RESERVED_5)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    spy.clear();
    data.clear();
    data.append((char)(FIN | QWebSocketProtocol::OC_RESERVED_6)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    spy.clear();
    data.clear();
    data.append((char)(FIN | QWebSocketProtocol::OC_RESERVED_7)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    spy.clear();
    data.clear();
    data.append((char)(FIN | QWebSocketProtocol::OC_RESERVED_B)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    spy.clear();
    data.clear();
    data.append((char)(FIN | QWebSocketProtocol::OC_RESERVED_C)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    spy.clear();
    data.clear();
    data.append((char)(FIN | QWebSocketProtocol::OC_RESERVED_D)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    spy.clear();
    data.clear();
    data.append((char)(FIN | QWebSocketProtocol::OC_RESERVED_E)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);

    spy.clear();
    data.clear();
    data.append((char)(FIN | QWebSocketProtocol::OC_RESERVED_F)).append((char)0x0u);
    buffer.close();
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_PROTOCOL_ERROR);
}

DECLARE_TEST(DataProcessorTest)

#include "tst_dataprocessor.moc"

