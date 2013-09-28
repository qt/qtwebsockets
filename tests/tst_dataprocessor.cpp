#include <QtTest/QtTest>
#include <QtTest/qtestcase.h>
#include <QSignalSpy>
#include <QBuffer>
#include <QByteArray>
#include <QDebug>
#include "dataprocessor_p.h"
#include "unittests.h"

Q_DECLARE_METATYPE(QWebSocketProtocol::CloseCode)
Q_DECLARE_METATYPE(QWebSocketProtocol::OpCode)

//happy flow
//TODO: test valid UTF8 sequences (see UC 6.2)
//TODO: test on masking correctness
//TODO: test for valid fields
//TODO: test for valid opcodes
//DONE: test for valid close codes
//TODO: test close frame with no close code and reason
//TODO: test if opcode is correct after processing of a continuation frame (text and binary frames)

//TODO: test valid frame sequences

//unhappy flow
//DONE: test invalid UTF8 sequences
//DONE: test invalid UTF8 sequences in control/close frames
//TODO: test invalid masks
//TODO: test for AutoBahn testcase 5
//TODO: test for AutoBahn testcase 6.1
//TODO: test for AutoBahn testcase 6.3 (fragmentation test)
//TODO: test for AutoBahn testcase 6.4 (fragmentation test)
//DONE: test for invalid fields
//DONE: test for invalid opcodes
//DONE: test continuation frames for too big payload
//DONE: test continuation frames for too small frame
//DONE: test continuation frames for bad rsv fields, etc.
//DONE: test continuation frames for incomplete payload
//DONE: test close frame with payload length 1 (is either 0, if no close code, or at least 2, close code + optional reason)
//DONE: besides spying on errors, we should also check if the frame and message signals are not emitted (or partially emitted)

//TODO: test invalid frame sequences

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

    /***************************************************************************
     * Happy Flows
     ***************************************************************************/
    /*!
      \brief Tests all kinds of valid binary frames, including zero length frames
      */
    void goodBinaryFrames();

    /*!
      \brief Tests all kinds of valid text frames, including zero length frames
      */
    void goodTextFrames();

    void goodControlFrames();

    //void goodHeaders();   //test all valid control codes

    /*!
      Tests the DataProcessor for the correct handling of non-charactercodes
      Due to a workaround in QTextCodec, non-characters are treated as illegal.
      This workaround is not necessary anymore, and hence code should be changed in Qt
      to allow non-characters again.
     */
    void nonCharacterCodes();

    /***************************************************************************
     * Unhappy Flows
     ***************************************************************************/
    /*!
        \brief Tests the DataProcessor for correct handling of frames that don't contain the starting 2 bytes.
        This test is a border case, where not enough bytes are received to even start parsing a frame.
        This test does not test sequences of frames, only single frames are tested
     */
    void frameTooSmall();

    /*!
        \brief Tests the DataProcessor for correct handling of frames that are oversized.
        This test does not test sequences of frames, only single frames are tested
     */
    void frameTooBig();

    /*!
        \brief Tests the DataProcessor for the correct handling of malformed frame headers.
        This test does not test sequences of frames, only single frames are tested
     */
    void invalidHeader();

    /*!
        \brief Tests the DataProcessor for the correct handling of invalid control frames.
        Invalid means: payload bigger than 125, frame is fragmented, ...
        This test does not test sequences of frames, only single frames are tested
     */
    void invalidControlFrame();
    void invalidCloseFrame();

    /*!
        \brief Tests the DataProcess for the correct handling of incomplete size fields for large and big payloads.
     */
    void incompleteSizeField();

    /*!
        \brief Tests the DataProcessor for the correct handling of incomplete payloads.
        This includes:
        - incomplete length bytes for large and big payloads (16- and 64-bit values),
        - minimum size representation (see RFC 6455 paragraph 5.2),
        - frames that are too large (larger than MAX_INT in bytes)
        - incomplete payloads (less bytes than indicated in the size field)
        This test does not test sequences of frames, only single frames are tested
     */
    void incompletePayload();

    /*!
        \brief Tests the DataProcessor for the correct handling of invalid UTF-8 payloads.
        This test does not test sequences of frames, only single frames are tested
     */
    void invalidPayload();

    void invalidPayloadInCloseFrame();

    /*!
      Tests the DataProcessor for the correct handling of the minimum size representation requirement of RFC 6455 (see paragraph 5.2)
     */
    void minimumSizeRequirement();

    void invalidHeader_data();
    void incompleteSizeField_data();
    void incompletePayload_data();
    void invalidPayload_data(bool isControlFrame = false);
    void invalidPayloadInCloseFrame_data();
    void minimumSizeRequirement_data();
    void invalidControlFrame_data();
    void invalidCloseFrame_data();
    void frameTooBig_data();

    void nonCharacterCodes_data();
    void goodTextFrames_data();
    void goodBinaryFrames_data();
    void goodControlFrames_data();

private:
    //helper function that constructs a new row of test data for invalid UTF8 sequences
    void invalidUTF8(const char *dataTag, const char *utf8Sequence, bool isCloseFrame);
    //helper function that constructs a new row of test data for invalid leading field values
    void invalidField(const char *dataTag, quint8 invalidFieldValue);
    //helper functions that construct a new row of test data for size fields that do not adhere to the minimum size requirement
    void minimumSize16Bit(quint16 sizeInBytes);
    void minimumSize64Bit(quint64 sizeInBytes);
    //helper function to construct a new row of test data containing frames with a payload size smaller than indicated in the header
    void incompleteFrame(quint8 controlCode, quint64 indicatedSize, quint64 actualPayloadSize);
    void insertIncompleteSizeFieldTest(quint8 payloadCode, quint8 numBytesFollowing);

    //helper function to construct a new row of test data containing text frames containing  sequences
    void nonCharacterSequence(const char *sequence);

    void doTest();
    void doCloseFrameTest();

    QString opCodeToString(quint8 opCode);
};

tst_DataProcessor::tst_DataProcessor()
{
}

void tst_DataProcessor::initTestCase()
{
}

void tst_DataProcessor::cleanupTestCase()
{
}

void tst_DataProcessor::init()
{
    qRegisterMetaType<QWebSocketProtocol::OpCode>("QWebSocketProtocol::OpCode");
    qRegisterMetaType<QWebSocketProtocol::CloseCode>("QWebSocketProtocol::CloseCode");
}

void tst_DataProcessor::cleanup()
{
}

void tst_DataProcessor::goodTextFrames_data()
{
    QTest::addColumn<QByteArray>("payload");

    for (int i = 0; i < (65536 + 256); i += 128)
    {
        QTest::newRow(QString("Text frame with %1 ASCII characters").arg(i).toStdString().data()) << QByteArray(i, 'a');
    }
    for (int i = 0; i < 128; ++i)
    {
        QTest::newRow(QString("Text frame with containing ASCII character '0x%1'").arg(QByteArray(1, char(i)).toHex().constData()).toStdString().data()) << QByteArray(1, char(i));
    }
}

void tst_DataProcessor::goodBinaryFrames_data()
{
    QTest::addColumn<QByteArray>("payload");
    for (int i = 0; i < (65536 + 256); i += 128)
    {
        QTest::newRow(QString("Binary frame with %1 bytes").arg(i).toStdString().data()) << QByteArray(i, char(1));
    }
    for (int i = 0; i < 256; ++i)
    {
        QTest::newRow(QString("Binary frame containing byte: '0x%1'").arg(QByteArray(1, char(i)).toHex().constData()).toStdString().data()) << QByteArray(i, char(1));
    }
}

void tst_DataProcessor::goodControlFrames_data()
{
    //TODO: still to test with no close code and no close reason (is valid)
    QTest::addColumn<QString>("payload");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("closeCode");
    //control frame data cannot exceed 125 bytes; smaller than 124, because we also need a 2 byte close code
    for (int i = 0; i < 124; ++i)
    {
        QTest::newRow(QString("Close frame with %1 ASCII characters").arg(i).toStdString().data()) << QString(i, 'a') << QWebSocketProtocol::CC_NORMAL;
    }
    for (int i = 0; i < 126; ++i)
    {
        QTest::newRow(QString("Text frame with containing ASCII character '0x%1'").arg(QByteArray(1, char(i)).toHex().constData()).toStdString().data()) << QString(1, char(i)) << QWebSocketProtocol::CC_NORMAL;
    }
    QTest::newRow("Close frame with close code NORMAL") << QString(1, 'a') << QWebSocketProtocol::CC_NORMAL;
    QTest::newRow("Close frame with close code BAD OPERATION") << QString(1, 'a') << QWebSocketProtocol::CC_BAD_OPERATION;
    QTest::newRow("Close frame with close code DATATYPE NOT SUPPORTED") << QString(1, 'a') << QWebSocketProtocol::CC_DATATYPE_NOT_SUPPORTED;
    QTest::newRow("Close frame with close code GOING AWAY") << QString(1, 'a') << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow("Close frame with close code MISSING EXTENSION") << QString(1, 'a') << QWebSocketProtocol::CC_MISSING_EXTENSION;
    QTest::newRow("Close frame with close code POLICY VIOLATED") << QString(1, 'a') << QWebSocketProtocol::CC_POLICY_VIOLATED;
    QTest::newRow("Close frame with close code PROTOCOL ERROR") << QString(1, 'a') << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Close frame with close code TOO MUCH DATA") << QString(1, 'a') << QWebSocketProtocol::CC_TOO_MUCH_DATA;
    QTest::newRow("Close frame with close code WRONG DATATYPE") << QString(1, 'a') << QWebSocketProtocol::CC_WRONG_DATATYPE;
    QTest::newRow("Close frame with close code 3000") << QString(1, 'a') << QWebSocketProtocol::CloseCode(3000);
    QTest::newRow("Close frame with close code 3999") << QString(1, 'a') << QWebSocketProtocol::CloseCode(3999);
    QTest::newRow("Close frame with close code 4000") << QString(1, 'a') << QWebSocketProtocol::CloseCode(4000);
    QTest::newRow("Close frame with close code 4999") << QString(1, 'a') << QWebSocketProtocol::CloseCode(4999);

    //Not allowed per RFC 6455 (see para 7.4.1)
    //QTest::newRow("Close frame with close code ABNORMAL DISCONNECTION") << QString(1, 'a') << QWebSocketProtocol::CC_ABNORMAL_DISCONNECTION;
    //QTest::newRow("Close frame with close code MISSING STATUS CODE") << QString(1, 'a') << QWebSocketProtocol::CC_MISSING_STATUS_CODE;
    //QTest::newRow("Close frame with close code HANDSHAKE FAILED") << QString(1, 'a') << QWebSocketProtocol::CC_TLS_HANDSHAKE_FAILED;
}

void tst_DataProcessor::goodBinaryFrames()
{
    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;
    QFETCH(QByteArray, payload);

    data.append((char)(FIN | QWebSocketProtocol::OC_BINARY));

    if (payload.length() < 126)
    {
        data.append(char(payload.length()));
    }
    else if (payload.length() < 65536)
    {
        quint16 swapped = qToBigEndian<quint16>(payload.length());
        const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
        data.append(char(126)).append(wireRepresentation, 2);
    }
    else
    {
        quint64 swapped = qToBigEndian<quint64>(payload.length());
        const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
        data.append(char(127)).append(wireRepresentation, 8);
    }

    data.append(payload);
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);

    QSignalSpy spyFrameReceived(&dataProcessor, SIGNAL(binaryFrameReceived(QByteArray,bool)));
    QSignalSpy spyMessageReceived(&dataProcessor, SIGNAL(binaryMessageReceived(QByteArray)));
    QSignalSpy spyTextFrameReceived(&dataProcessor, SIGNAL(textFrameReceived(QString,bool)));
    QSignalSpy spyTextMessageReceived(&dataProcessor, SIGNAL(textMessageReceived(QString)));
    dataProcessor.process(&buffer);
    QCOMPARE(spyFrameReceived.count(), 1);
    QCOMPARE(spyMessageReceived.count(), 1);
    QCOMPARE(spyTextFrameReceived.count(), 0);
    QCOMPARE(spyTextMessageReceived.count(), 0);
    QList<QVariant> arguments = spyFrameReceived.takeFirst();
    QCOMPARE(arguments.at(0).toByteArray().length(), payload.length());
    arguments = spyMessageReceived.takeFirst();
    QCOMPARE(arguments.at(0).toByteArray().length(), payload.length());
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
    QFETCH(QByteArray, payload);

    data.append((char)(FIN | QWebSocketProtocol::OC_TEXT));

    if (payload.length() < 126)
    {
        data.append(char(payload.length()));
    }
    else if (payload.length() < 65536)
    {
        quint16 swapped = qToBigEndian<quint16>(payload.length());
        const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
        data.append(char(126)).append(wireRepresentation, 2);
    }
    else
    {
        quint64 swapped = qToBigEndian<quint64>(payload.length());
        const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
        data.append(char(127)).append(wireRepresentation, 8);
    }

    data.append(payload);
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);

    QSignalSpy spyFrameReceived(&dataProcessor, SIGNAL(textFrameReceived(QString,bool)));
    QSignalSpy spyMessageReceived(&dataProcessor, SIGNAL(textMessageReceived(QString)));
    QSignalSpy spyBinaryFrameReceived(&dataProcessor, SIGNAL(binaryFrameReceived(QString,bool)));
    QSignalSpy spyBinaryMessageReceived(&dataProcessor, SIGNAL(binaryMessageReceived(QString)));
    dataProcessor.process(&buffer);
    QCOMPARE(spyFrameReceived.count(), 1);
    QCOMPARE(spyMessageReceived.count(), 1);
    QCOMPARE(spyBinaryFrameReceived.count(), 0);
    QCOMPARE(spyBinaryMessageReceived.count(), 0);
    QList<QVariant> arguments = spyFrameReceived.takeFirst();
    QCOMPARE(arguments.at(0).toString().length(), payload.length());
    arguments = spyMessageReceived.takeFirst();
    QCOMPARE(arguments.at(0).toString().length(), payload.length());
    buffer.close();
    spyFrameReceived.clear();
    spyMessageReceived.clear();
    data.clear();
}

void tst_DataProcessor::goodControlFrames()
{
    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;
    QFETCH(QString, payload);
    QFETCH(QWebSocketProtocol::CloseCode, closeCode);
    quint16 swapped = qToBigEndian<quint16>(closeCode);
    const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));

    data.append((char)(FIN | QWebSocketProtocol::OC_CLOSE)).append(char(payload.length() + 2)).append(wireRepresentation, 2).append(payload);
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);

    QSignalSpy spyCloseFrameReceived(&dataProcessor, SIGNAL(closeReceived(QWebSocketProtocol::CloseCode,QString)));
    QSignalSpy spyTextFrameReceived(&dataProcessor, SIGNAL(textFrameReceived(QString,bool)));
    QSignalSpy spyTextMessageReceived(&dataProcessor, SIGNAL(textMessageReceived(QString)));
    QSignalSpy spyBinaryFrameReceived(&dataProcessor, SIGNAL(binaryFrameReceived(QByteArray,bool)));
    QSignalSpy spyBinaryMessageReceived(&dataProcessor, SIGNAL(binaryMessageReceived(QByteArray)));

    dataProcessor.process(&buffer);
    QCOMPARE(spyCloseFrameReceived.count(), 1);
    QCOMPARE(spyTextFrameReceived.count(), 0);
    QCOMPARE(spyTextMessageReceived.count(), 0);
    QCOMPARE(spyBinaryFrameReceived.count(), 0);
    QCOMPARE(spyBinaryMessageReceived.count(), 0);
    QList<QVariant> arguments = spyCloseFrameReceived.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), closeCode);
    QCOMPARE(arguments.at(1).toString().length(), payload.length());
    buffer.close();
    spyCloseFrameReceived.clear();
    data.clear();
}

void tst_DataProcessor::nonCharacterCodes()
{
    QFETCH(quint8, firstByte);
    QFETCH(quint8, secondByte);
    QFETCH(QByteArray, payload);

    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;
    QSignalSpy frameSpy(&dataProcessor, SIGNAL(textFrameReceived(QString,bool)));
    QSignalSpy messageSpy(&dataProcessor, SIGNAL(textMessageReceived(QString)));
    QSignalSpy binaryFrameSpy(&dataProcessor, SIGNAL(binaryFrameReceived(QByteArray,bool)));
    QSignalSpy binaryMessageSpy(&dataProcessor, SIGNAL(textMessageReceived(QByteArray)));

    data.append(firstByte).append(secondByte);
    data.append(payload);
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QEXPECT_FAIL(QTest::currentDataTag(), "Due to QTextCode interpeting non-characters unicode points as invalid (QTBUG-33229).", Abort);

    QCOMPARE(frameSpy.count(), 1);
    QCOMPARE(messageSpy.count(), 1);
    QCOMPARE(binaryFrameSpy.count(), 0);
    QCOMPARE(binaryMessageSpy.count(), 0);

    QVariantList arguments = frameSpy.takeFirst();
    QCOMPARE(arguments.at(0).value<QString>().toUtf8(), payload);
    arguments = messageSpy.takeFirst();
    QCOMPARE(arguments.at(0).value<QString>().toUtf8(), payload);
    buffer.close();
    frameSpy.clear();
    messageSpy.clear();
    data.clear();
}

void tst_DataProcessor::frameTooSmall()
{
    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;
    QByteArray firstFrame;

    firstFrame.append(quint8(QWebSocketProtocol::OC_TEXT)).append(char(1)).append(QByteArray(1, 'a'));

    //with nothing in the buffer, the dataProcessor should time out and the error should be CC_GOING_AWAY
    //meaning the socket will be closed
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    QSignalSpy spy(&dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)));
    QSignalSpy textMessageSpy(&dataProcessor, SIGNAL(textMessageReceived(QString)));
    QSignalSpy binaryMessageSpy(&dataProcessor, SIGNAL(binaryMessageReceived(QByteArray)));
    QSignalSpy textFrameSpy(&dataProcessor, SIGNAL(textFrameReceived(QString, bool)));
    QSignalSpy binaryFrameSpy(&dataProcessor, SIGNAL(binaryFrameReceived(QByteArray,bool)));
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(textMessageSpy.count(), 0);
    QCOMPARE(binaryMessageSpy.count(), 0);
    QCOMPARE(textFrameSpy.count(), 0);
    QCOMPARE(binaryFrameSpy.count(), 0);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_GOING_AWAY);
    spy.clear();
    textMessageSpy.clear();
    binaryMessageSpy.clear();
    textFrameSpy.clear();
    binaryFrameSpy.clear();
    buffer.close();
    data.clear();

    //only one byte; this is far too little; should get a time out as well and the error should be CC_GOING_AWAY
    //meaning the socket will be closed
    data.append(quint8('1'));   //put 1 byte in the buffer; this is too little
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(textMessageSpy.count(), 0);
    QCOMPARE(binaryMessageSpy.count(), 0);
    QCOMPARE(textFrameSpy.count(), 0);
    QCOMPARE(binaryFrameSpy.count(), 0);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_GOING_AWAY);
    buffer.close();
    spy.clear();
    textMessageSpy.clear();
    binaryMessageSpy.clear();
    textFrameSpy.clear();
    binaryFrameSpy.clear();
    data.clear();


    {
        //text frame with final bit not set
        data.append((char)(QWebSocketProtocol::OC_TEXT)).append(char(0x0));
        buffer.setData(data);
        buffer.open(QIODevice::ReadOnly);
        dataProcessor.process(&buffer);

        buffer.close();
        data.clear();

        //with nothing in the buffer, the dataProcessor should time out and the error should be CC_GOING_AWAY
        //meaning the socket will be closed
        buffer.setData(data);
        buffer.open(QIODevice::ReadOnly);
        QSignalSpy spy(&dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)));
        dataProcessor.process(&buffer);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(textMessageSpy.count(), 0);
        QCOMPARE(binaryMessageSpy.count(), 0);
        QCOMPARE(textFrameSpy.count(), 1);
        QCOMPARE(binaryFrameSpy.count(), 0);
        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_GOING_AWAY);
        spy.clear();
        textMessageSpy.clear();
        binaryMessageSpy.clear();
        textFrameSpy.clear();
        binaryFrameSpy.clear();
        buffer.close();
        data.clear();

        //text frame with final bit not set
        data.append((char)(QWebSocketProtocol::OC_TEXT)).append(char(0x0));
        buffer.setData(data);
        buffer.open(QIODevice::ReadOnly);
        dataProcessor.process(&buffer);

        QCOMPARE(textMessageSpy.count(), 0);
        QCOMPARE(binaryMessageSpy.count(), 0);
        QCOMPARE(textFrameSpy.count(), 1);
        QCOMPARE(binaryFrameSpy.count(), 0);

        buffer.close();
        data.clear();
        spy.clear();
        textMessageSpy.clear();
        binaryMessageSpy.clear();
        textFrameSpy.clear();
        binaryFrameSpy.clear();

        //only 1 byte follows in continuation frame; should time out with close code CC_GOING_AWAY
        data.append('a');
        buffer.setData(data);
        buffer.open(QIODevice::ReadOnly);

        dataProcessor.process(&buffer);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(textMessageSpy.count(), 0);
        QCOMPARE(binaryMessageSpy.count(), 0);
        QCOMPARE(textFrameSpy.count(), 0);
        QCOMPARE(binaryFrameSpy.count(), 0);
        arguments = spy.takeFirst();
        QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), QWebSocketProtocol::CC_GOING_AWAY);
        spy.clear();
        buffer.close();
    }
}

void tst_DataProcessor::frameTooBig()
{
    doTest();
}

void tst_DataProcessor::invalidHeader()
{
    doTest();
}

void tst_DataProcessor::invalidControlFrame()
{
    doTest();
}

void tst_DataProcessor::invalidCloseFrame()
{
    doCloseFrameTest();
}

void tst_DataProcessor::minimumSizeRequirement()
{
    doTest();
}

void tst_DataProcessor::invalidPayload()
{
    doTest();
}

void tst_DataProcessor::invalidPayloadInCloseFrame()
{
    QFETCH(quint8, firstByte);
    QFETCH(quint8, secondByte);
    QFETCH(QByteArray, payload);
    QFETCH(bool, isContinuationFrame);
    QFETCH(QWebSocketProtocol::CloseCode, expectedCloseCode);

    Q_UNUSED(isContinuationFrame)

    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;
    QSignalSpy spy(&dataProcessor, SIGNAL(closeReceived(QWebSocketProtocol::CloseCode,QString)));
    QSignalSpy textMessageSpy(&dataProcessor, SIGNAL(textMessageReceived(QString)));
    QSignalSpy binaryMessageSpy(&dataProcessor, SIGNAL(binaryMessageReceived(QByteArray)));
    QSignalSpy textFrameSpy(&dataProcessor, SIGNAL(textFrameReceived(QString, bool)));
    QSignalSpy binaryFrameSpy(&dataProcessor, SIGNAL(binaryFrameReceived(QByteArray,bool)));

    data.append(firstByte).append(secondByte);
    data.append(payload);
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(textMessageSpy.count(), 0);
    QCOMPARE(binaryMessageSpy.count(), 0);
    QCOMPARE(textFrameSpy.count(), 0);
    QCOMPARE(binaryFrameSpy.count(), 0);
    QVariantList arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), expectedCloseCode);
    buffer.close();
    spy.clear();
    data.clear();
}

void tst_DataProcessor::incompletePayload()
{
    doTest();
}

void tst_DataProcessor::incompleteSizeField()
{
    doTest();
}

//////////////////////////////////////////////////////////////////////////////////////////
/// HELPER FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////
void tst_DataProcessor::doTest()
{
    QFETCH(quint8, firstByte);
    QFETCH(quint8, secondByte);
    QFETCH(QByteArray, payload);
    QFETCH(bool, isContinuationFrame);
    QFETCH(QWebSocketProtocol::CloseCode, expectedCloseCode);

    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;
    QSignalSpy spy(&dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)));
    QSignalSpy textMessageSpy(&dataProcessor, SIGNAL(textMessageReceived(QString)));
    QSignalSpy binaryMessageSpy(&dataProcessor, SIGNAL(binaryMessageReceived(QByteArray)));
    QSignalSpy textFrameSpy(&dataProcessor, SIGNAL(textFrameReceived(QString, bool)));
    QSignalSpy binaryFrameSpy(&dataProcessor, SIGNAL(binaryFrameReceived(QByteArray,bool)));

    if (isContinuationFrame)
    {
        data.append(quint8(QWebSocketProtocol::OC_TEXT)).append(char(1)).append(QByteArray(1, 'a'));
    }
    data.append(firstByte).append(secondByte);
    data.append(payload);
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(textMessageSpy.count(), 0);
    QCOMPARE(binaryMessageSpy.count(), 0);
    QCOMPARE(textFrameSpy.count(), isContinuationFrame ? 1 : 0);
    QCOMPARE(binaryFrameSpy.count(), 0);
    QVariantList arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), expectedCloseCode);
    buffer.close();
    spy.clear();
    data.clear();
}

void tst_DataProcessor::doCloseFrameTest()
{
    QFETCH(quint8, firstByte);
    QFETCH(quint8, secondByte);
    QFETCH(QByteArray, payload);
    QFETCH(bool, isContinuationFrame);
    QFETCH(QWebSocketProtocol::CloseCode, expectedCloseCode);

    Q_UNUSED(isContinuationFrame)

    QByteArray data;
    QBuffer buffer;
    DataProcessor dataProcessor;
    QSignalSpy spy(&dataProcessor, SIGNAL(closeReceived(QWebSocketProtocol::CloseCode,QString)));
    QSignalSpy errorSpy(&dataProcessor, SIGNAL(errorEncountered(QWebSocketProtocol::CloseCode,QString)));
    QSignalSpy textMessageSpy(&dataProcessor, SIGNAL(textMessageReceived(QString)));
    QSignalSpy binaryMessageSpy(&dataProcessor, SIGNAL(binaryMessageReceived(QByteArray)));
    QSignalSpy textFrameSpy(&dataProcessor, SIGNAL(textFrameReceived(QString, bool)));
    QSignalSpy binaryFrameSpy(&dataProcessor, SIGNAL(binaryFrameReceived(QByteArray,bool)));

    data.append(firstByte).append(secondByte);
    data.append(payload);
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    dataProcessor.process(&buffer);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(textMessageSpy.count(), 0);
    QCOMPARE(binaryMessageSpy.count(), 0);
    QCOMPARE(textFrameSpy.count(), 0);
    QCOMPARE(binaryFrameSpy.count(), 0);
    QVariantList arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QWebSocketProtocol::CloseCode>(), expectedCloseCode);
    buffer.close();
    spy.clear();
    data.clear();
}

QString tst_DataProcessor::opCodeToString(quint8 opCode)
{
    QString frameType;
    switch (opCode)
    {
        case QWebSocketProtocol::OC_BINARY:
        {
            frameType = "Binary";
            break;
        }
        case QWebSocketProtocol::OC_TEXT:
        {
            frameType = "Text";
            break;
        }
        case QWebSocketProtocol::OC_PING:
        {
            frameType = "Ping";
            break;
        }
        case QWebSocketProtocol::OC_PONG:
        {
            frameType = "Pong";
            break;
        }
        case QWebSocketProtocol::OC_CLOSE:
        {
            frameType = "Close";
            break;
        }
        case QWebSocketProtocol::OC_CONTINUE:
        {
            frameType = "Continuation";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_3:
        {
            frameType = "Reserved3";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_4:
        {
            frameType = "Reserved5";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_5:
        {
            frameType = "Reserved5";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_6:
        {
            frameType = "Reserved6";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_7:
        {
            frameType = "Reserved7";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_B:
        {
            frameType = "ReservedB";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_C:
        {
            frameType = "ReservedC";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_D:
        {
            frameType = "ReservedD";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_E:
        {
            frameType = "ReservedE";
            break;
        }
        case QWebSocketProtocol::OC_RESERVED_F:
        {
            frameType = "ReservedF";
            break;
        }
        default:
        {
            //should never come here
            Q_ASSERT(false);
        }
    }
    return frameType;
}

void tst_DataProcessor::minimumSize16Bit(quint16 sizeInBytes)
{
    quint16 swapped16 = qToBigEndian<quint16>(sizeInBytes);
    const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped16));
    QTest::newRow(QString("Text frame with payload size %1, represented in 2 bytes").arg(sizeInBytes).toStdString().data())
            << quint8(FIN | QWebSocketProtocol::OC_TEXT)
            << quint8(126)
            << QByteArray(wireRepresentation, 2)
            << false
            << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow(QString("Binary frame with payload size %1, represented in 2 bytes").arg(sizeInBytes).toStdString().data())
            << quint8(FIN | QWebSocketProtocol::OC_BINARY)
            << quint8(126)
            << QByteArray(wireRepresentation, 2)
            << false
            << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow(QString("Continuation frame with payload size %1, represented in 2 bytes").arg(sizeInBytes).toStdString().data())
            << quint8(FIN | QWebSocketProtocol::OC_CONTINUE)
            << quint8(126)
            << QByteArray(wireRepresentation, 2)
            << true
            << QWebSocketProtocol::CC_PROTOCOL_ERROR;
}

void tst_DataProcessor::minimumSize64Bit(quint64 sizeInBytes)
{
    quint64 swapped64 = qToBigEndian<quint64>(sizeInBytes);
    const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped64));

    QTest::newRow(QString("Text frame with payload size %1, represented in 8 bytes").arg(sizeInBytes).toStdString().data())
            << quint8(FIN | QWebSocketProtocol::OC_TEXT)
            << quint8(127)
            << QByteArray(wireRepresentation, 8)
            << false
            << QWebSocketProtocol::CC_PROTOCOL_ERROR;

    QTest::newRow(QString("Binary frame with payload size %1, represented in 8 bytes").arg(sizeInBytes).toStdString().data())
            << quint8(FIN | QWebSocketProtocol::OC_BINARY)
            << quint8(127)
            << QByteArray(wireRepresentation, 8)
            << false
            << QWebSocketProtocol::CC_PROTOCOL_ERROR;

    QTest::newRow(QString("Continuation frame with payload size %1, represented in 8 bytes").arg(sizeInBytes).toStdString().data())
            << quint8(FIN | QWebSocketProtocol::OC_CONTINUE)
            << quint8(127)
            << QByteArray(wireRepresentation, 8)
            << true
            << QWebSocketProtocol::CC_PROTOCOL_ERROR;
}

void tst_DataProcessor::invalidUTF8(const char *dataTag, const char *utf8Sequence, bool isCloseFrame)
{
    QByteArray payload = QByteArray::fromHex(utf8Sequence);

    if (isCloseFrame)
    {
        quint16 closeCode = qToBigEndian<quint16>(QWebSocketProtocol::CC_NORMAL);
        const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&closeCode));
        QTest::newRow(QString("Close frame with invalid UTF8-sequence: %1").arg(dataTag).toStdString().data())
                << quint8(FIN | QWebSocketProtocol::OC_CLOSE)
                << quint8(payload.length() + 2)
                << QByteArray(wireRepresentation, 2).append(payload)
                << false
                << QWebSocketProtocol::CC_WRONG_DATATYPE;
    }
    else
    {
        QTest::newRow(QString("Text frame with invalid UTF8-sequence: %1").arg(dataTag).toStdString().data())
                << quint8(FIN | QWebSocketProtocol::OC_TEXT)
                << quint8(payload.length())
                << payload
                << false
                << QWebSocketProtocol::CC_WRONG_DATATYPE;

        QTest::newRow(QString("Continuation text frame with invalid UTF8-sequence: %1").arg(dataTag).toStdString().data())
                << quint8(FIN | QWebSocketProtocol::OC_CONTINUE)
                << quint8(payload.length())
                << payload
                << true
                << QWebSocketProtocol::CC_WRONG_DATATYPE;
    }
}

void tst_DataProcessor::invalidField(const char *dataTag, quint8 invalidFieldValue)
{
    QTest::newRow(dataTag) << quint8(FIN | invalidFieldValue)
                           << quint8(0x00)
                           << QByteArray()
                           << false
                           << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow(QString(dataTag).append(" with continuation frame").toStdString().data())
                            << quint8(FIN | invalidFieldValue)
                            << quint8(0x00)
                            << QByteArray()
                            << true
                            << QWebSocketProtocol::CC_PROTOCOL_ERROR;
}

void tst_DataProcessor::incompleteFrame(quint8 controlCode, quint64 indicatedSize, quint64 actualPayloadSize)
{
    QVERIFY(!QWebSocketProtocol::isOpCodeReserved(QWebSocketProtocol::OpCode(controlCode)));
    QVERIFY(indicatedSize > actualPayloadSize);

    QString frameType = opCodeToString(controlCode);
    QByteArray firstFrame;

    if (indicatedSize < 126)
    {
        QTest::newRow(frameType.append(QString(" frame with payload size %1, but only %2 bytes of data").arg(indicatedSize).arg(actualPayloadSize)).toStdString().data())
                << quint8(FIN | controlCode)
                << quint8(indicatedSize)
                << firstFrame.append(QByteArray(actualPayloadSize, 'a'))
                << (controlCode == QWebSocketProtocol::OC_CONTINUE)
                << QWebSocketProtocol::CC_GOING_AWAY;
    }
    else if (indicatedSize <= 0xFFFFu)
    {
        quint16 swapped16 = qToBigEndian<quint16>(static_cast<quint16>(indicatedSize));
        const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped16));
        QTest::newRow(frameType.append(QString(" frame with payload size %1, but only %2 bytes of data").arg(indicatedSize).arg(actualPayloadSize)).toStdString().data())
                << quint8(FIN | controlCode)
                << quint8(126)
                << firstFrame.append(QByteArray(wireRepresentation, 2).append(QByteArray(actualPayloadSize, 'a')))
                << (controlCode == QWebSocketProtocol::OC_CONTINUE)
                << QWebSocketProtocol::CC_GOING_AWAY;
    }
    else
    {
        quint64 swapped64 = qToBigEndian<quint64>(indicatedSize);
        const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped64));
        QTest::newRow(frameType.append(QString(" frame with payload size %1, but only %2 bytes of data").arg(indicatedSize).arg(actualPayloadSize)).toStdString().data())
                << quint8(FIN | controlCode)
                << quint8(127)
                << firstFrame.append(QByteArray(wireRepresentation, 8).append(QByteArray(actualPayloadSize, 'a')))
                << (controlCode == QWebSocketProtocol::OC_CONTINUE)
                << QWebSocketProtocol::CC_GOING_AWAY;
    }
}

void tst_DataProcessor::nonCharacterSequence(const char *sequence)
{
    QByteArray utf8Sequence = QByteArray::fromHex(sequence);
    //TODO: qstrdup - memory leak! qstrdup is necessary because once QString goes out of scope we have garbage
    const char *tagName = qstrdup(qPrintable(QString("Text frame with payload containing the non-control character sequence 0x%1").arg(QString(sequence))));
    const char *tagName2 = qstrdup(qPrintable(QString("Continuation frame with payload containing the non-control character sequence 0x%1").arg(QString(sequence))));

    QTest::newRow(tagName)
            << quint8(FIN | QWebSocketProtocol::OC_TEXT)
            << quint8(utf8Sequence.size())
            << utf8Sequence
            << false;

    QTest::newRow(tagName2)
            << quint8(FIN | QWebSocketProtocol::OC_CONTINUE)
            << quint8(utf8Sequence.size())
            << utf8Sequence
            << true;
}

void tst_DataProcessor::insertIncompleteSizeFieldTest(quint8 payloadCode, quint8 numBytesFollowing)
{
    QTest::newRow(QString("Text frame with payload size %1, with %2 bytes following.").arg(payloadCode).arg(numBytesFollowing).toStdString().data())
            << quint8(FIN | QWebSocketProtocol::OC_TEXT)
            << quint8(payloadCode)
            << QByteArray(numBytesFollowing, quint8(1))
            << false
            << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow(QString("Binary frame with payload size %1, with %2 bytes following.").arg(payloadCode).arg(numBytesFollowing).toStdString().data())
            << quint8(FIN | QWebSocketProtocol::OC_BINARY)
            << quint8(payloadCode)
            << QByteArray(numBytesFollowing, quint8(1))
            << false
            << QWebSocketProtocol::CC_GOING_AWAY;
    QTest::newRow(QString("Continuation frame with payload size %1, with %2 bytes following.").arg(payloadCode).arg(numBytesFollowing).toStdString().data())
            << quint8(FIN | QWebSocketProtocol::OC_CONTINUE)
            << quint8(payloadCode)
            << QByteArray(numBytesFollowing, quint8(1))
            << true
            << QWebSocketProtocol::CC_GOING_AWAY;
}

//////////////////////////////////////////////////////////////////////////////////
//// TEST DATA
//////////////////////////////////////////////////////////////////////////////////
void tst_DataProcessor::invalidHeader_data()
{
    //The first byte contain the FIN, RSV1, RSV2, RSV3 and the Opcode
    //The second byte contains the MaskFlag and the length of the frame
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");    //superfluous, but present to be able to call doTest(), which expects a payload field
    QTest::addColumn<bool>("isContinuationFrame");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    //invalid bit fields
    invalidField("RSV1 set", RSV1);
    invalidField("RSV2 set", RSV2);
    invalidField("RSV3 set", RSV3);
    invalidField("RSV1 and RSV2 set", RSV1 | RSV2);
    invalidField("RSV1 and RSV3 set", RSV1 | RSV3);
    invalidField("RSV2 and RSV3 set", RSV2 | RSV3);
    invalidField("RSV1, RSV2 and RSV3 set", RSV1 | RSV2 | RSV3);

    //invalid opcodes
    invalidField("Invalid OpCode 3", QWebSocketProtocol::OC_RESERVED_3);
    invalidField("Invalid OpCode 4", QWebSocketProtocol::OC_RESERVED_4);
    invalidField("Invalid OpCode 5", QWebSocketProtocol::OC_RESERVED_5);
    invalidField("Invalid OpCode 6", QWebSocketProtocol::OC_RESERVED_6);
    invalidField("Invalid OpCode 7", QWebSocketProtocol::OC_RESERVED_7);
    invalidField("Invalid OpCode B", QWebSocketProtocol::OC_RESERVED_B);
    invalidField("Invalid OpCode C", QWebSocketProtocol::OC_RESERVED_C);
    invalidField("Invalid OpCode D", QWebSocketProtocol::OC_RESERVED_D);
    invalidField("Invalid OpCode E", QWebSocketProtocol::OC_RESERVED_E);
    invalidField("Invalid OpCode F", QWebSocketProtocol::OC_RESERVED_F);
}

void tst_DataProcessor::invalidPayloadInCloseFrame_data()
{
    invalidPayload_data(true);
}

void tst_DataProcessor::invalidPayload_data(bool isControlFrame)
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<bool>("isContinuationFrame");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    //6.3: invalid UTF-8 sequence
    invalidUTF8("case 6.3.1",   "cebae1bdb9cf83cebcceb5eda080656469746564", isControlFrame);

    //6.4.: fail fast tests; invalid UTF-8 in middle of string
    invalidUTF8("case 6.4.1",   "cebae1bdb9cf83cebcceb5f4908080656469746564", isControlFrame);
    invalidUTF8("case 6.4.4",   "cebae1bdb9cf83cebcceb5eda080656469746564", isControlFrame);

    //6.6: All prefixes of a valid UTF-8 string that contains multi-byte code points
    invalidUTF8("case 6.6.1",   "ce", isControlFrame);
    invalidUTF8("case 6.6.3",   "cebae1", isControlFrame);
    invalidUTF8("case 6.6.4",   "cebae1bd", isControlFrame);
    invalidUTF8("case 6.6.6",   "cebae1bdb9cf", isControlFrame);
    invalidUTF8("case 6.6.8",   "cebae1bdb9cf83ce", isControlFrame);
    invalidUTF8("case 6.6.10",  "cebae1bdb9cf83cebcce", isControlFrame);

    //6.8: First possible sequence length 5/6 (invalid codepoints)
    invalidUTF8("case 6.8.1",   "f888808080", isControlFrame);
    invalidUTF8("case 6.8.2",   "fc8480808080", isControlFrame);

    //6.10: Last possible sequence length 4/5/6 (invalid codepoints)
    invalidUTF8("case 6.10.1",  "f7bfbfbf", isControlFrame);
    invalidUTF8("case 6.10.2",  "fbbfbfbfbf", isControlFrame);
    invalidUTF8("case 6.10.3",  "fdbfbfbfbfbf", isControlFrame);

    //5.11: boundary conditions
    invalidUTF8("case 6.11.5",  "f4908080", isControlFrame);

    //6.12: unexpected continuation bytes
    invalidUTF8("case 6.12.1",  "80", isControlFrame);
    invalidUTF8("case 6.12.2",  "bf", isControlFrame);
    invalidUTF8("case 6.12.3",  "80bf", isControlFrame);
    invalidUTF8("case 6.12.4",  "80bf80", isControlFrame);
    invalidUTF8("case 6.12.5",  "80bf80bf", isControlFrame);
    invalidUTF8("case 6.12.6",  "80bf80bf80", isControlFrame);
    invalidUTF8("case 6.12.7",  "80bf80bf80bf", isControlFrame);
    invalidUTF8("case 6.12.8",  "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3b4b5b6b7b8b9babbbcbdbe", isControlFrame);

    //6.13: lonely start characters
    invalidUTF8("case 6.13.1",  "c020c120c220c320c420c520c620c720c820c920ca20cb20cc20cd20ce20cf20d020d120d220d320d420d520d620d720d820d920da20db20dc20dd20de20", isControlFrame);
    invalidUTF8("case 6.13.2",  "e020e120e220e320e420e520e620e720e820e920ea20eb20ec20ed20ee20", isControlFrame);
    invalidUTF8("case 6.13.3",  "f020f120f220f320f420f520f620", isControlFrame);
    invalidUTF8("case 6.13.4",  "f820f920fa20", isControlFrame);
    invalidUTF8("case 6.13.5",  "fc20", isControlFrame);

    //6.14: sequences with last continuation byte missing
    invalidUTF8("case 6.14.1",  "c0", isControlFrame);
    invalidUTF8("case 6.14.2",  "e080", isControlFrame);
    invalidUTF8("case 6.14.3",  "f08080", isControlFrame);
    invalidUTF8("case 6.14.4",  "f8808080", isControlFrame);
    invalidUTF8("case 6.14.5",  "fc80808080", isControlFrame);
    invalidUTF8("case 6.14.6",  "df", isControlFrame);
    invalidUTF8("case 6.14.7",  "efbf", isControlFrame);
    invalidUTF8("case 6.14.8",  "f7bfbf", isControlFrame);
    invalidUTF8("case 6.14.9",  "fbbfbfbf", isControlFrame);
    invalidUTF8("case 6.14.10", "fdbfbfbfbf", isControlFrame);

    //6.15: concatenation of incomplete sequences
    invalidUTF8("case 6.15.1",  "c0e080f08080f8808080fc80808080dfefbff7bfbffbbfbfbffdbfbfbfbf", isControlFrame);

    //6.16: impossible bytes
    invalidUTF8("case 6.16.1",  "fe", isControlFrame);
    invalidUTF8("case 6.16.2",  "ff", isControlFrame);
    invalidUTF8("case 6.16.3",  "fefeffff", isControlFrame);

    //6.17: overlong ASCII characters
    invalidUTF8("case 6.17.1",  "c0af", isControlFrame);
    invalidUTF8("case 6.17.2",  "e080af", isControlFrame);
    invalidUTF8("case 6.17.3",  "f08080af", isControlFrame);
    invalidUTF8("case 6.17.4",  "f8808080af", isControlFrame);
    invalidUTF8("case 6.17.5",  "fc80808080af", isControlFrame);

    //6.18: maximum overlong sequences
    invalidUTF8("case 6.18.1",  "c1bf", isControlFrame);
    invalidUTF8("case 6.18.2",  "e09fbf", isControlFrame);
    invalidUTF8("case 6.18.3",  "f08fbfbf", isControlFrame);
    invalidUTF8("case 6.18.4",  "f887bfbfbf", isControlFrame);
    invalidUTF8("case 6.18.5",  "fc83bfbfbfbf", isControlFrame);

    //6.19: overlong presentation of the NUL character
    invalidUTF8("case 6.19.1",  "c080", isControlFrame);
    invalidUTF8("case 6.19.2",  "e08080", isControlFrame);
    invalidUTF8("case 6.19.3",  "f0808080", isControlFrame);
    invalidUTF8("case 6.19.4",  "f880808080", isControlFrame);
    invalidUTF8("case 6.19.5",  "fc8080808080", isControlFrame);

    //6.20: Single UTF-16 surrogates
    invalidUTF8("case 6.20.1",  "eda080", isControlFrame);
    invalidUTF8("case 6.20.2",  "edadbf", isControlFrame);
    invalidUTF8("case 6.20.3",  "edae80", isControlFrame);
    invalidUTF8("case 6.20.4",  "edafbf", isControlFrame);
    invalidUTF8("case 6.20.5",  "edb080", isControlFrame);
    invalidUTF8("case 6.20.6",  "edbe80", isControlFrame);
    invalidUTF8("case 6.20.7",  "edbfbf", isControlFrame);

    //6.21: Paired UTF-16 surrogates
    invalidUTF8("case 6.21.1",  "eda080edb080", isControlFrame);
    invalidUTF8("case 6.21.2",  "eda080edbfbf", isControlFrame);
    invalidUTF8("case 6.21.3",  "edadbfedb080", isControlFrame);
    invalidUTF8("case 6.21.4",  "edadbfedbfbf", isControlFrame);
    invalidUTF8("case 6.21.5",  "edae80edb080", isControlFrame);
    invalidUTF8("case 6.21.6",  "edae80edbfbf", isControlFrame);
    invalidUTF8("case 6.21.7",  "edafbfedb080", isControlFrame);
    invalidUTF8("case 6.21.8",  "edafbfedbfbf", isControlFrame);
}

void tst_DataProcessor::minimumSizeRequirement_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<bool>("isContinuationFrame");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    minimumSize16Bit(0);
    minimumSize16Bit(64);
    minimumSize16Bit(125);

    minimumSize64Bit(0);
    minimumSize64Bit(64);
    minimumSize64Bit(125);
    minimumSize64Bit(126);
    minimumSize64Bit(256);
    minimumSize64Bit(512);
    minimumSize64Bit(1024);
    minimumSize64Bit(2048);
    minimumSize64Bit(4096);
    minimumSize64Bit(8192);
    minimumSize64Bit(16384);
    minimumSize64Bit(32768);
    minimumSize64Bit(0xFFFFu);
}

void tst_DataProcessor::invalidControlFrame_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<bool>("isContinuationFrame");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");


    QTest::newRow("Close control frame with payload size 126")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) << quint8(126) << QByteArray() << false << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Ping control frame with payload size 126")
            << quint8(FIN | QWebSocketProtocol::OC_PING) << quint8(126) << QByteArray() << false << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Close control frame with payload size 126")
            << quint8(FIN | QWebSocketProtocol::OC_PONG) << quint8(126) << QByteArray() << false << QWebSocketProtocol::CC_PROTOCOL_ERROR;

    QTest::newRow("Non-final close control frame (fragmented)")
            << quint8(QWebSocketProtocol::OC_CLOSE) << quint8(32) << QByteArray() << false << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Non-final ping control frame (fragmented)")
            << quint8(QWebSocketProtocol::OC_PING) << quint8(32) << QByteArray() << false << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    QTest::newRow("Non-final pong control frame (fragmented)")
            << quint8(QWebSocketProtocol::OC_PONG) << quint8(32) << QByteArray() << false << QWebSocketProtocol::CC_PROTOCOL_ERROR;
}

void tst_DataProcessor::invalidCloseFrame_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<bool>("isContinuationFrame");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    QTest::newRow("Close control frame with payload size 1")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) << quint8(1) << QByteArray(1, 'a') << false << QWebSocketProtocol::CC_PROTOCOL_ERROR;
    quint16 swapped = qToBigEndian<quint16>(QWebSocketProtocol::CC_ABNORMAL_DISCONNECTION);
    const char *wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Close control frame close code ABNORMAL DISCONNECTION")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) <<
               quint8(2) <<
               QByteArray(wireRepresentation, 2) <<
               false <<
               QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint16>(QWebSocketProtocol::CC_MISSING_STATUS_CODE);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Close control frame close code MISSING STATUS CODE")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) <<
               quint8(2) <<
               QByteArray(wireRepresentation, 2) <<
               false <<
               QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint16>(1004);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Close control frame close code 1004")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) <<
               quint8(2) <<
               QByteArray(wireRepresentation, 2) <<
               false <<
               QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint16>(QWebSocketProtocol::CC_TLS_HANDSHAKE_FAILED);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Close control frame close code TLS HANDSHAKE FAILED")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) <<
               quint8(2) <<
               QByteArray(wireRepresentation, 2) <<
               false <<
               QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint16>(0);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Close control frame close code 0")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) <<
               quint8(2) <<
               QByteArray(wireRepresentation, 2) <<
               false <<
               QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint16>(999);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Close control frame close code 999")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) <<
               quint8(2) <<
               QByteArray(wireRepresentation, 2) <<
               false <<
               QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint16>(1012);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Close control frame close code 1012")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) <<
               quint8(2) <<
               QByteArray(wireRepresentation, 2) <<
               false <<
               QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint16>(1013);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Close control frame close code 1013")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) <<
               quint8(2) <<
               QByteArray(wireRepresentation, 2) <<
               false <<
               QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint16>(1014);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Close control frame close code 1014")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) <<
               quint8(2) <<
               QByteArray(wireRepresentation, 2) <<
               false <<
               QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint16>(1100);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Close control frame close code 1100")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) <<
               quint8(2) <<
               QByteArray(wireRepresentation, 2) <<
               false <<
               QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint16>(2000);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Close control frame close code 2000")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) <<
               quint8(2) <<
               QByteArray(wireRepresentation, 2) <<
               false <<
               QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint16>(2999);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Close control frame close code 2999")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) <<
               quint8(2) <<
               QByteArray(wireRepresentation, 2) <<
               false <<
               QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint16>(5000);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Close control frame close code 5000")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) <<
               quint8(2) <<
               QByteArray(wireRepresentation, 2) <<
               false <<
               QWebSocketProtocol::CC_PROTOCOL_ERROR;
    swapped = qToBigEndian<quint16>(65535u);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped));
    QTest::newRow("Close control frame close code 65535")
            << quint8(FIN | QWebSocketProtocol::OC_CLOSE) <<
               quint8(2) <<
               QByteArray(wireRepresentation, 2) <<
               false <<
               QWebSocketProtocol::CC_PROTOCOL_ERROR;
}

void tst_DataProcessor::incompleteSizeField_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<bool>("isContinuationFrame");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //for a frame length value of 126, there should be 2 bytes following to form a 16-bit frame length
    //////////////////////////////////////////////////////////////////////////////////////////////////
    insertIncompleteSizeFieldTest(126, 0);
    insertIncompleteSizeFieldTest(126, 1);

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //for a frame length value of 127, there should be 8 bytes following to form a 64-bit frame length
    //////////////////////////////////////////////////////////////////////////////////////////////////
    insertIncompleteSizeFieldTest(127, 0);
    insertIncompleteSizeFieldTest(127, 1);
    insertIncompleteSizeFieldTest(127, 2);
    insertIncompleteSizeFieldTest(127, 3);
    insertIncompleteSizeFieldTest(127, 4);
    insertIncompleteSizeFieldTest(127, 5);
    insertIncompleteSizeFieldTest(127, 6);
    insertIncompleteSizeFieldTest(127, 7);
}

void tst_DataProcessor::nonCharacterCodes_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<bool>("isContinuationFrame");

    nonCharacterSequence("efbfbe");
    nonCharacterSequence("efbfbf");
    nonCharacterSequence("f09fbfbe");
    nonCharacterSequence("f09fbfbf");
    nonCharacterSequence("f0afbfbe");
    nonCharacterSequence("f0afbfbf");
    nonCharacterSequence("f0bfbfbe");
    nonCharacterSequence("f0bfbfbf");
    nonCharacterSequence("f18fbfbe");
    nonCharacterSequence("f18fbfbf");
    nonCharacterSequence("f19fbfbe");
    nonCharacterSequence("f19fbfbf");
    nonCharacterSequence("f1afbfbe");
    nonCharacterSequence("f1afbfbf");
    nonCharacterSequence("f1bfbfbe");
    nonCharacterSequence("f1bfbfbf");
    nonCharacterSequence("f28fbfbe");
    nonCharacterSequence("f28fbfbf");
    nonCharacterSequence("f29fbfbe");
    nonCharacterSequence("f29fbfbf");
    nonCharacterSequence("f2afbfbe");
    nonCharacterSequence("f2afbfbf");
    nonCharacterSequence("f2bfbfbe");
    nonCharacterSequence("f2bfbfbf");
    nonCharacterSequence("f38fbfbe");
    nonCharacterSequence("f38fbfbf");
    nonCharacterSequence("f39fbfbe");
    nonCharacterSequence("f39fbfbf");
    nonCharacterSequence("f3afbfbe");
    nonCharacterSequence("f3afbfbf");
    nonCharacterSequence("f3bfbfbe");
    nonCharacterSequence("f3bfbfbf");
    nonCharacterSequence("f48fbfbe");
    nonCharacterSequence("f48fbfbf");
}

void tst_DataProcessor::incompletePayload_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<bool>("isContinuationFrame");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    incompleteFrame(QWebSocketProtocol::OC_TEXT, 125, 0);
    incompleteFrame(QWebSocketProtocol::OC_TEXT, 64, 32);
    incompleteFrame(QWebSocketProtocol::OC_TEXT, 256, 32);
    incompleteFrame(QWebSocketProtocol::OC_TEXT, 128000, 32);
    incompleteFrame(QWebSocketProtocol::OC_BINARY, 125, 0);
    incompleteFrame(QWebSocketProtocol::OC_BINARY, 64, 32);
    incompleteFrame(QWebSocketProtocol::OC_BINARY, 256, 32);
    incompleteFrame(QWebSocketProtocol::OC_BINARY, 128000, 32);
    incompleteFrame(QWebSocketProtocol::OC_CONTINUE, 125, 0);
    incompleteFrame(QWebSocketProtocol::OC_CONTINUE, 64, 32);
    incompleteFrame(QWebSocketProtocol::OC_CONTINUE, 256, 32);
    incompleteFrame(QWebSocketProtocol::OC_CONTINUE, 128000, 32);

    incompleteFrame(QWebSocketProtocol::OC_CLOSE, 64, 32);
    incompleteFrame(QWebSocketProtocol::OC_PING, 64, 32);
    incompleteFrame(QWebSocketProtocol::OC_PONG, 64, 32);
}

void tst_DataProcessor::frameTooBig_data()
{
    QTest::addColumn<quint8>("firstByte");
    QTest::addColumn<quint8>("secondByte");
    QTest::addColumn<QByteArray>("payload");
    QTest::addColumn<bool>("isContinuationFrame");
    QTest::addColumn<QWebSocketProtocol::CloseCode>("expectedCloseCode");

    quint64 swapped64 = 0;
    const char *wireRepresentation = 0;

    //only data frames are checked for being too big
    //control frames have explicit checking on a maximum payload size of 125, which is tested elsewhere

    swapped64 = qToBigEndian<quint64>(DataProcessor::maxFrameSize() + 1);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped64));
    QTest::newRow("Text frame with payload size > INT_MAX")
            << quint8(FIN | QWebSocketProtocol::OC_TEXT)
            << quint8(127)
            << QByteArray(wireRepresentation, 8).append(QByteArray(32, 'a'))
            << false
            << QWebSocketProtocol::CC_TOO_MUCH_DATA;

    swapped64 = qToBigEndian<quint64>(DataProcessor::maxFrameSize() + 1);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped64));
    QTest::newRow("Binary frame with payload size > INT_MAX")
            << quint8(FIN | QWebSocketProtocol::OC_BINARY)
            << quint8(127)
            << QByteArray(wireRepresentation, 8).append(QByteArray(32, 'a'))
            << false
            << QWebSocketProtocol::CC_TOO_MUCH_DATA;

    swapped64 = qToBigEndian<quint64>(DataProcessor::maxFrameSize() + 1);
    wireRepresentation = static_cast<const char *>(static_cast<const void *>(&swapped64));
    QTest::newRow("Continuation frame with payload size > INT_MAX")
            << quint8(FIN | QWebSocketProtocol::OC_CONTINUE)
            << quint8(127)
            << QByteArray(wireRepresentation, 8).append(QByteArray(32, 'a'))
            << true
            << QWebSocketProtocol::CC_TOO_MUCH_DATA;
}

DECLARE_TEST(tst_DataProcessor)

#include "tst_dataprocessor.moc"

