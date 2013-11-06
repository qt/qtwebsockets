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

#include "QtWebSockets/qwebsocketcorsauthenticator.h"
#include "QtWebSockets/qwebsocketprotocol.h"

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QWebSocketProtocol::CloseCode)
Q_DECLARE_METATYPE(QWebSocketProtocol::OpCode)

class tst_WebSocketCorsAuthenticator : public QObject
{
    Q_OBJECT

public:
    tst_WebSocketCorsAuthenticator();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void tst_initialization();
};

tst_WebSocketCorsAuthenticator::tst_WebSocketCorsAuthenticator()
{}

void tst_WebSocketCorsAuthenticator::initTestCase()
{
}

void tst_WebSocketCorsAuthenticator::cleanupTestCase()
{}

void tst_WebSocketCorsAuthenticator::init()
{
    qRegisterMetaType<QWebSocketProtocol::OpCode>("QWebSocketProtocol::OpCode");
    qRegisterMetaType<QWebSocketProtocol::CloseCode>("QWebSocketProtocol::CloseCode");
}

void tst_WebSocketCorsAuthenticator::cleanup()
{
}

void tst_WebSocketCorsAuthenticator::tst_initialization()
{
    {
        QWebSocketCorsAuthenticator authenticator(QStringLiteral(""));

        QCOMPARE(authenticator.allowed(), true);
        QCOMPARE(authenticator.origin(), QString());
    }
    {
        QWebSocketCorsAuthenticator authenticator(QStringLiteral("com.somesite"));

        QCOMPARE(authenticator.allowed(), true);
        QCOMPARE(authenticator.origin(), QStringLiteral("com.somesite"));

        QWebSocketCorsAuthenticator other(authenticator);
        QCOMPARE(other.origin(), authenticator.origin());
        QCOMPARE(other.allowed(), authenticator.allowed());

        authenticator.setAllowed(false);
        QVERIFY(!authenticator.allowed());
        QCOMPARE(other.allowed(), true);   //make sure other is a real copy

        authenticator.setAllowed(true);
        QVERIFY(authenticator.allowed());

        authenticator.setAllowed(false);
        other = authenticator;
        QCOMPARE(other.origin(), authenticator.origin());
        QCOMPARE(other.allowed(), authenticator.allowed());
    }
}

QTEST_MAIN(tst_WebSocketCorsAuthenticator)

#include "tst_websocketcorsauthenticator.moc"

