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
#include <QtEndian>

#include <QDebug>

#include "qwebsocketprotocol.h"
#include "private/qwebsocketprotocol_p.h"

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QWebSocketProtocol::CloseCode)
Q_DECLARE_METATYPE(QWebSocketProtocol::OpCode)

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

    QTest::newRow("Empty payload") << 0x12345678u << QString() << QByteArray();
    QTest::newRow("ASCII payload of 8 characters")
            << 0x12345678u
            << QStringLiteral("abcdefgh")
            << QByteArrayLiteral("\x73\x56\x35\x1C\x77\x52\x31\x10");
    QTest::newRow("ASCII payload of 9 characters")
            << 0x12345678u
            << QStringLiteral("abcdefghi")
            << QByteArrayLiteral("\x73\x56\x35\x1C\x77\x52\x31\x10\x7B");
    //MSVC doesn't like UTF-8 in source code;
    //the following text is represented in the string below: ∫∂ƒ©øØ
    QTest::newRow("UTF-8 payload")
            << 0x12345678u
            << QString::fromUtf8("\xE2\x88\xAB\xE2\x88\x82\xC6\x92\xC2\xA9\xC3\xB8\xC3\x98")
            << QByteArrayLiteral("\x2D\x0B\x69\xD1\xEA\xEC");
}

void tst_WebSocketProtocol::tst_validMasks()
{
    QFETCH(quint32, mask);
    QFETCH(QString, inputdata);
    QFETCH(QByteArray, result);

    //put latin1 into an explicit array
    //otherwise, the intermediate object is deleted and the data pointer becomes invalid
    QByteArray latin1 = inputdata.toLatin1();
    char *data = latin1.data();

    QWebSocketProtocol::mask(data, inputdata.size(), mask);
    QCOMPARE(QByteArray::fromRawData(data, inputdata.size()), result);
}

void tst_WebSocketProtocol::tst_opCodes_data()
{
    QTest::addColumn<QWebSocketProtocol::OpCode>("opCode");
    QTest::addColumn<bool>("isReserved");

    QTest::newRow("OpCodeBinary")       << QWebSocketProtocol::OpCodeBinary << false;
    QTest::newRow("OpCodeClose")        << QWebSocketProtocol::OpCodeClose << false;
    QTest::newRow("OpCodeContinue")     << QWebSocketProtocol::OpCodeContinue << false;
    QTest::newRow("OpCodePing")         << QWebSocketProtocol::OpCodePing << false;
    QTest::newRow("OpCodePong")         << QWebSocketProtocol::OpCodePong << false;
    QTest::newRow("OpCodeReserved3")    << QWebSocketProtocol::OpCodeReserved3 << true;
    QTest::newRow("OpCodeReserved4")    << QWebSocketProtocol::OpCodeReserved4 << true;
    QTest::newRow("OpCodeReserved5")    << QWebSocketProtocol::OpCodeReserved5 << true;
    QTest::newRow("OpCodeReserved6")    << QWebSocketProtocol::OpCodeReserved6 << true;
    QTest::newRow("OpCodeReserved7")    << QWebSocketProtocol::OpCodeReserved7 << true;
    QTest::newRow("OpCodeReserved8")    << QWebSocketProtocol::OpCodeReservedB << true;
    QTest::newRow("OpCodeReservedC")    << QWebSocketProtocol::OpCodeReservedC << true;
    QTest::newRow("OpCodeReservedD")    << QWebSocketProtocol::OpCodeReservedD << true;
    QTest::newRow("OpCodeReservedE")    << QWebSocketProtocol::OpCodeReservedE << true;
    QTest::newRow("OpCodeReservedF")    << QWebSocketProtocol::OpCodeReservedF << true;
    QTest::newRow("OpCodeText")         << QWebSocketProtocol::OpCodeText << false;
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
        QTest::newRow(QStringLiteral("Close code %1").arg(i).toLatin1().constData()) << i << false;
    }

    for (int i = 1000; i < 1004; ++i)
    {
        QTest::newRow(QStringLiteral("Close code %1").arg(i).toLatin1().constData()) << i << true;
    }

    QTest::newRow("Close code 1004") << 1004 << false;
    QTest::newRow("Close code 1005") << 1005 << false;
    QTest::newRow("Close code 1006") << 1006 << false;

    for (int i = 1007; i < 1012; ++i)
    {
        QTest::newRow(QStringLiteral("Close code %1").arg(i).toLatin1().constData()) << i << true;
    }

    for (int i = 1013; i < 3000; ++i)
    {
        QTest::newRow(QStringLiteral("Close code %1").arg(i).toLatin1().constData()) << i << false;
    }

    for (int i = 3000; i < 5000; ++i)
    {
        QTest::newRow(QStringLiteral("Close code %1").arg(i).toLatin1().constData()) << i << true;
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

