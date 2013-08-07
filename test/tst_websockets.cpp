#include <QtTest/QtTest>
#include <QtTest/qtestcase.h>
#include <QSignalSpy>
#include <QHostInfo>
#include <QDebug>
#include "websocket.h"
#include "unittests.h"

class WebSocketsTest : public QObject
{
	Q_OBJECT

public:
	WebSocketsTest();

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();
	void init();
	void cleanup();
	/**
	 * @brief testTextMessage Tests sending and receiving a text message
	 */
	void testTextMessage();

	void testBinaryMessage();

	/**
	 * @brief Tests the method localAddress and localPort
	 */
	void testLocalAddress();

	/**
	 * @brief Test the methods peerAddress, peerName and peerPort
	 */
	void testPeerAddress();

	/**
	 * @brief Test the methods proxy() and setProxy() and check if it can be correctly set
	 */
	void testProxy();

	/**
	 * @brief Runs the autobahn tests against our implementation
	 */
	//void autobahnTest();

private:
	WebSocket *m_pWebSocket;
	QUrl m_url;
};

WebSocketsTest::WebSocketsTest() :
	m_pWebSocket(0),
	m_url("ws://localhost:9000")
{
}

void WebSocketsTest::initTestCase()
{
	m_pWebSocket = new WebSocket();
	m_pWebSocket->open(m_url, true);
	QTRY_VERIFY_WITH_TIMEOUT(m_pWebSocket->state() == QAbstractSocket::ConnectedState, 1000);
	QVERIFY(m_pWebSocket->isValid());
}

void WebSocketsTest::cleanupTestCase()
{
	if (m_pWebSocket)
	{
		m_pWebSocket->close();
		//QVERIFY2(m_pWebSocket->waitForDisconnected(1000), "Disconnection failed.");
		delete m_pWebSocket;
		m_pWebSocket = 0;
	}
}

void WebSocketsTest::init()
{
}

void WebSocketsTest::cleanup()
{
}

void WebSocketsTest::testTextMessage()
{
	const char *message = "Hello world!";

	QSignalSpy spy(m_pWebSocket, SIGNAL(textMessageReceived(QString)));

	QCOMPARE(m_pWebSocket->send(message), (qint64)strlen(message));

	QTRY_VERIFY_WITH_TIMEOUT(spy.count() != 0, 1000);
	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.at(0).count(), 1);
	QCOMPARE(spy.takeFirst().at(0).toString(), QString(message));

	spy.clear();
	QString qMessage(message);
	QCOMPARE(m_pWebSocket->send(qMessage), (qint64)qMessage.length());
	QTRY_VERIFY_WITH_TIMEOUT(spy.count() != 0, 1000);
	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.at(0).count(), 1);
	QCOMPARE(spy.takeFirst().at(0).toString(), qMessage);
}

void WebSocketsTest::testBinaryMessage()
{
	QSignalSpy spy(m_pWebSocket, SIGNAL(binaryMessageReceived(QByteArray)));

	QByteArray data("Hello world!");

	QCOMPARE(m_pWebSocket->send(data), (qint64)data.size());

	QTRY_VERIFY_WITH_TIMEOUT(spy.count() != 0, 1000);
	QCOMPARE(spy.count(), 1);
	QCOMPARE(spy.at(0).count(), 1);
	QCOMPARE(spy.takeFirst().at(0).toByteArray(), data);
}

void WebSocketsTest::testLocalAddress()
{
	QCOMPARE(m_pWebSocket->localAddress().toString(), QString("127.0.0.1"));
	quint16 localPort = m_pWebSocket->localPort();
	QVERIFY2(localPort > 0, "Local port is invalid.");
}

void WebSocketsTest::testPeerAddress()
{
	QHostInfo hostInfo = QHostInfo::fromName(m_url.host());
	QList<QHostAddress> addresses = hostInfo.addresses();
	QVERIFY(addresses.length() > 0);
	QHostAddress peer = m_pWebSocket->peerAddress();
	bool found = false;
	Q_FOREACH(QHostAddress a, addresses)
	{
		if (a == peer)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		QFAIL("PeerAddress is not found as a result of a reverse lookup");
	}
	QCOMPARE(m_pWebSocket->peerName(), m_url.host());
	QCOMPARE(m_pWebSocket->peerPort(), (quint16)m_url.port(80));
}

void WebSocketsTest::testProxy()
{
	QNetworkProxy oldProxy = m_pWebSocket->proxy();
	QNetworkProxy proxy(QNetworkProxy::HttpProxy, QString("proxy.network.com"), 80);
	m_pWebSocket->setProxy(proxy);
	QCOMPARE(proxy, m_pWebSocket->proxy());
	m_pWebSocket->setProxy(oldProxy);
	QCOMPARE(oldProxy, m_pWebSocket->proxy());
}

//DECLARE_TEST(WebSocketsTest)

#include "tst_websockets.moc"

