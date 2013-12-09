/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
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
#include <QSignalSpy>
#include <QHostInfo>
#include <QSslError>
#include <QDebug>
#include <QtWebSockets/QWebSocket>

class tst_ComplianceTest : public QObject
{
    Q_OBJECT

public:
    tst_ComplianceTest();

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
    QUrl m_url;

    void runTestCases(int startNbr, int stopNbr = -1);
    void runTestCase(int nbr, int total);
};

tst_ComplianceTest::tst_ComplianceTest() :
    m_url("ws://localhost:9001")
{
}

void tst_ComplianceTest::initTestCase()
{
}

void tst_ComplianceTest::cleanupTestCase()
{
}

void tst_ComplianceTest::init()
{
}

void tst_ComplianceTest::cleanup()
{
}

void tst_ComplianceTest::runTestCase(int nbr, int total)
{
    if (nbr == total)
    {
        return;
    }

    QWebSocket *pWebSocket = new QWebSocket;
    QSignalSpy spy(pWebSocket, SIGNAL(disconnected()));

    //next for every case, connect to url
    //ws://ipaddress:port/runCase?case=<number>&agent=<agentname>
    //where agent name will be QWebSocket
    QObject::connect(pWebSocket, &QWebSocket::textMessageReceived, [=](QString message) {
        pWebSocket->write(message);
    });
    QObject::connect(pWebSocket, &QWebSocket::binaryMessageReceived, [=](QByteArray message) {
        pWebSocket->write(message);
    });

    qDebug() << "Executing test" << (nbr + 1) << "/" << total;
    QUrl url = m_url;
    url.setPath("/runCase?");
    QUrlQuery query;
    query.addQueryItem("case", QString::number(nbr + 1));
    query.addQueryItem("agent", "QtWebSockets/1.0");
    url.setQuery(query);
    pWebSocket->open(url);
    spy.wait(60000);
    pWebSocket->close();
    delete pWebSocket;
    pWebSocket = Q_NULLPTR;
    runTestCase(nbr + 1, total);
}

void tst_ComplianceTest::runTestCases(int startNbr, int stopNbr)
{
    runTestCase(startNbr, stopNbr);
}

void tst_ComplianceTest::autobahnTest()
{
    //connect to autobahn server at url ws://ipaddress:port/getCaseCount
    QWebSocket *pWebSocket = new QWebSocket;
    QUrl url = m_url;
    int numberOfTestCases = 0;
    QSignalSpy spy(pWebSocket, SIGNAL(disconnected()));
    QObject::connect(pWebSocket, &QWebSocket::textMessageReceived, [&](QString message) {
        numberOfTestCases = message.toInt();
    });

    url.setPath("/getCaseCount");
    pWebSocket->open(url);
    spy.wait(60000);
    QVERIFY(numberOfTestCases > 0);

    QObject::disconnect(pWebSocket, &QWebSocket::textMessageReceived, 0, 0);
    runTestCases(0, numberOfTestCases);

    url.setPath("/updateReports?");
    QUrlQuery query;
    query.addQueryItem("agent", "QWebSockets");
    url.setQuery(query);
    pWebSocket->open(url);
    spy.wait(60000);
    delete pWebSocket;
}

QTEST_MAIN(tst_ComplianceTest)

#include "tst_compliance.moc"

