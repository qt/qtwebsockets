#include <QtTest/QtTest>
#include <QtTest/qtestcase.h>
#include <QSignalSpy>
#include <QHostInfo>
#include <QDebug>
#include "websocket.h"
#include "unittests.h"

class ComplianceTest : public QObject
{
    Q_OBJECT

public:
    ComplianceTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    /**
     * @brief Runs the autobahn tests against our implementation
     */
    void autobahnTest();

private:
    WebSocket *m_pWebSocket;
    QUrl m_url;
};

ComplianceTest::ComplianceTest() :
    m_pWebSocket(0),
    m_url("ws://localhost:9001")
{
}

void ComplianceTest::initTestCase()
{
    //m_pWebSocket = new WebSocket(WebSocketProtocol::V_LATEST);
//    m_pWebSocket->open(m_url, true);
//    QTRY_VERIFY_WITH_TIMEOUT(m_pWebSocket->state() == QAbstractSocket::ConnectedState, 1000);
//    QVERIFY(m_pWebSocket->isValid());
}

void ComplianceTest::cleanupTestCase()
{
//    if (m_pWebSocket)
//    {
//        m_pWebSocket->close();
//        //QVERIFY2(m_pWebSocket->waitForDisconnected(1000), "Disconnection failed.");
//        delete m_pWebSocket;
//        m_pWebSocket = 0;
//    }
}

void ComplianceTest::init()
{
}

void ComplianceTest::cleanup()
{
}

void ComplianceTest::autobahnTest()
{
    //connect to autobahn server at url ws://ipaddress:port/getCaseCount
    WebSocket *pWebSocket = new WebSocket;
    int numberOfTestCases = 0;
    QString m;
    QObject::connect(pWebSocket, &WebSocket::textFrameReceived, [&](QString message, bool) {
        //qDebug() << "Received text message from autobahn:" << message;
        numberOfTestCases = message.toInt();
    });
    QObject::connect(pWebSocket, &WebSocket::aboutToClose, [=]() {
        //qDebug() << "About to close";
        //pWebSocket->deleteLater();
    });

    QUrl url = m_url;
//    url.setPort(9001);
    url.setPath("/getCaseCount");
    pWebSocket->open(url);
    QTRY_VERIFY_WITH_TIMEOUT(pWebSocket->state() == QAbstractSocket::UnconnectedState, 1000);
    QVERIFY(numberOfTestCases > 0);
    QObject::disconnect(pWebSocket, &WebSocket::textFrameReceived, 0, 0);

    //next for every case, connect to url
    //ws://ipaddress:port/runCase?case=<number>&agent=<agentname>
    //where agent name will be QtWebSocket
    QObject::connect(pWebSocket, &WebSocket::textFrameReceived, [=](QString message, bool) {
        //qDebug() << "Received text message from autobahn:" << message.length();
        pWebSocket->send(message);
        //pWebSocket->close();
    });
    QObject::connect(pWebSocket, &WebSocket::binaryFrameReceived, [=](QByteArray message, bool) {
        //qDebug() << "Received binary message from autobahn:" << message;
        pWebSocket->send(message);
        //pWebSocket->close();
    });

    for (int i = 0; i < numberOfTestCases; ++i)
    //for (int i = 16; i < 28; ++i)
    {
        qDebug() << "Executing test" << (i + 1) << "/" << numberOfTestCases;
        url.setPath("/runCase?");
        QUrlQuery query;
        query.addQueryItem("case", QString::number(i + 1));
        query.addQueryItem("agent", "QtWebSockets");
        url.setQuery(query);

        pWebSocket->open(url);
        QTRY_VERIFY_WITH_TIMEOUT(pWebSocket->state() == QAbstractSocket::UnconnectedState, 60000);
    }

    url.setPath("/updateReports?");
    QUrlQuery query;
    query.addQueryItem("agent", "QtWebSockets");
    url.setQuery(query);
    pWebSocket->open(url);
    QTRY_VERIFY_WITH_TIMEOUT(pWebSocket->state() == QAbstractSocket::UnconnectedState, 60000);
    pWebSocket->close();
}

DECLARE_TEST(ComplianceTest);

#include "tst_compliance.moc"

