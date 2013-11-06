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

#ifndef QWEBSOCKETDATAPROCESSOR_P_H
#define QWEBSOCKETDATAPROCESSOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include "qwebsocketprotocol.h"

QT_BEGIN_NAMESPACE

class QIODevice;
class QWebSocketFrame;

class Q_AUTOTEST_EXPORT QWebSocketDataProcessor : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QWebSocketDataProcessor)

public:
    explicit QWebSocketDataProcessor(QObject *parent = Q_NULLPTR);
    virtual ~QWebSocketDataProcessor();

    static quint64 maxMessageSize();
    static quint64 maxFrameSize();

Q_SIGNALS:
    void pingReceived(QByteArray data);
    void pongReceived(QByteArray data);
    void closeReceived(QWebSocketProtocol::CloseCode closeCode, QString closeReason);
    void textFrameReceived(QString frame, bool lastFrame);
    void binaryFrameReceived(QByteArray frame, bool lastFrame);
    void textMessageReceived(QString message);
    void binaryMessageReceived(QByteArray message);
    void errorEncountered(QWebSocketProtocol::CloseCode code, QString description);

public Q_SLOTS:
    void process(QIODevice *pIoDevice);
    void clear();

private:
    enum
    {
        PS_READ_HEADER,
        PS_READ_PAYLOAD_LENGTH,
        PS_READ_BIG_PAYLOAD_LENGTH,
        PS_READ_MASK,
        PS_READ_PAYLOAD,
        PS_DISPATCH_RESULT
    } m_processingState;

    bool m_isFinalFrame;
    bool m_isFragmented;
    QWebSocketProtocol::OpCode m_opCode;
    bool m_isControlFrame;
    bool m_hasMask;
    quint32 m_mask;
    QByteArray m_binaryMessage;
    QString m_textMessage;
    quint64 m_payloadLength;
    QTextCodec::ConverterState *m_pConverterState;
    QTextCodec *m_pTextCodec;

    bool processControlFrame(const QWebSocketFrame &frame);
};

QT_END_NAMESPACE

#endif // QWEBSOCKETDATAPROCESSOR_P_H
