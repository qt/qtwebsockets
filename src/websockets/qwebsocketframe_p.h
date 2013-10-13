/*
QWebSockets implements the WebSocket protocol as defined in RFC 6455.
Copyright (C) 2013 Kurt Pattyn (pattyn.kurt@gmail.com)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef QWEBSOCKETFRAME_P_H
#define QWEBSOCKETFRAME_P_H

#include "qwebsocketprotocol.h"

#include <QIODevice>

QT_BEGIN_NAMESPACE

const quint64 MAX_FRAME_SIZE_IN_BYTES = INT_MAX - 1;
const quint64 MAX_MESSAGE_SIZE_IN_BYTES = INT_MAX - 1;

class Q_AUTOTEST_EXPORT QWebSocketFrame
{
public:
    QWebSocketFrame();
    QWebSocketFrame(const QWebSocketFrame &other);

    const QWebSocketFrame &operator =(const QWebSocketFrame &other);

    QWebSocketProtocol::CloseCode getCloseCode() const;
    QString getCloseReason() const;
    bool isFinalFrame() const;
    bool isControlFrame() const;
    bool isDataFrame() const;
    bool isContinuationFrame() const;
    bool hasMask() const;
    quint32 getMask() const;    //returns 0 if no mask
    int getRsv1() const;
    int getRsv2() const;
    int getRsv3() const;
    QWebSocketProtocol::OpCode getOpCode() const;
    QByteArray getPayload() const;

    void clear();       //resets all member variables, and invalidates the object

    bool isValid() const;

    static QWebSocketFrame readFrame(QIODevice *pIoDevice);

private:
    QWebSocketProtocol::CloseCode m_closeCode;
    QString m_closeReason;
    bool m_isFinalFrame;
    quint32 m_mask;
    int m_rsv1; //reserved field 1
    int m_rsv2; //reserved field 2
    int m_rsv3; //reserved field 3
    QWebSocketProtocol::OpCode m_opCode;

    quint8 m_length;        //length field as read from the header; this is 1 byte, which when 126 or 127, indicates a large payload
    QByteArray m_payload;

    bool m_isValid;

    enum ProcessingState
    {
        PS_READ_HEADER,
        PS_READ_PAYLOAD_LENGTH,
        PS_READ_BIG_PAYLOAD_LENGTH,
        PS_READ_MASK,
        PS_READ_PAYLOAD,
        PS_DISPATCH_RESULT,
        PS_WAIT_FOR_MORE_DATA
    };

    void setError(QWebSocketProtocol::CloseCode code, QString closeReason);
    bool checkValidity();
};

QT_END_NAMESPACE

#endif // QWEBSOCKETFRAME_P_H
