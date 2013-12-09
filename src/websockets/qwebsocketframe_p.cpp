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

/*!
    \class QWebSocketFrame
    The class QWebSocketFrame is responsible for reading, validating and interpreting frames from a websocket.
    It reads data from a QIODevice, validates it against RFC 6455, and parses it into a frame (data, control).
    Whenever an error is detected, isValid() returns false.

    \note The QWebSocketFrame class does not look at valid sequences of frames. It processes frames one at a time.
    \note It is the QWebSocketDataProcessor that takes the sequence into account.

    \sa DataProcessor()
    \internal
 */

#include "qwebsocketframe_p.h"
#include "qwebsocketprotocol_p.h"

#include <QtCore/QtEndian>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

/*!
    \internal
 */
QWebSocketFrame::QWebSocketFrame() :
    m_closeCode(QWebSocketProtocol::CC_NORMAL),
    m_closeReason(),
    m_isFinalFrame(true),
    m_mask(0),
    m_rsv1(0),
    m_rsv2(0),
    m_rsv3(0),
    m_opCode(QWebSocketProtocol::OC_RESERVED_C),
    m_length(0),
    m_payload(),
    m_isValid(false)
{
}

/*!
    \internal
 */
QWebSocketFrame::QWebSocketFrame(const QWebSocketFrame &other) :
    m_closeCode(other.m_closeCode),
    m_closeReason(other.m_closeReason),
    m_isFinalFrame(other.m_isFinalFrame),
    m_mask(other.m_mask),
    m_rsv1(other.m_rsv1),
    m_rsv2(other.m_rsv2),
    m_rsv3(other.m_rsv3),
    m_opCode(other.m_opCode),
    m_length(other.m_length),
    m_payload(other.m_payload),
    m_isValid(other.m_isValid)
{
}

/*!
    \internal
 */
QWebSocketFrame &QWebSocketFrame::operator =(const QWebSocketFrame &other)
{
    m_closeCode = other.m_closeCode;
    m_closeReason = other.m_closeReason;
    m_isFinalFrame = other.m_isFinalFrame;
    m_mask = other.m_mask;
    m_rsv1 = other.m_rsv1;
    m_rsv2 = other.m_rsv2;
    m_rsv3 = other.m_rsv2;
    m_opCode = other.m_opCode;
    m_length = other.m_length;
    m_payload = other.m_payload;
    m_isValid = other.m_isValid;

    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
/*!
    \internal
 */
QWebSocketFrame::QWebSocketFrame(QWebSocketFrame &&other) :
    m_closeCode(qMove(other.m_closeCode)),
    m_closeReason(qMove(other.m_closeReason)),
    m_isFinalFrame(qMove(other.m_isFinalFrame)),
    m_mask(qMove(other.m_mask)),
    m_rsv1(qMove(other.m_rsv1)),
    m_rsv2(qMove(other.m_rsv2)),
    m_rsv3(qMove(other.m_rsv3)),
    m_opCode(qMove(other.m_opCode)),
    m_length(qMove(other.m_length)),
    m_payload(qMove(other.m_payload)),
    m_isValid(qMove(other.m_isValid))
{}


/*!
    \internal
 */
QWebSocketFrame &QWebSocketFrame::operator =(QWebSocketFrame &&other)
{
    qSwap(m_closeCode, other.m_closeCode);
    qSwap(m_closeReason, other.m_closeReason);
    qSwap(m_isFinalFrame, other.m_isFinalFrame);
    qSwap(m_mask, other.m_mask);
    qSwap(m_rsv1, other.m_rsv1);
    qSwap(m_rsv2, other.m_rsv2);
    qSwap(m_rsv3, other.m_rsv3);
    qSwap(m_opCode, other.m_opCode);
    qSwap(m_length, other.m_length);
    qSwap(m_payload, other.m_payload);
    qSwap(m_isValid, other.m_isValid);

    return *this;
}

#endif

/*!
    \internal
 */
void QWebSocketFrame::swap(QWebSocketFrame &other)
{
    if (&other != this) {
        qSwap(m_closeCode, other.m_closeCode);
        qSwap(m_closeReason, other.m_closeReason);
        qSwap(m_isFinalFrame, other.m_isFinalFrame);
        qSwap(m_mask, other.m_mask);
        qSwap(m_rsv1, other.m_rsv1);
        qSwap(m_rsv2, other.m_rsv2);
        qSwap(m_rsv3, other.m_rsv3);
        qSwap(m_opCode, other.m_opCode);
        qSwap(m_length, other.m_length);
        qSwap(m_payload, other.m_payload);
        qSwap(m_isValid, other.m_isValid);
    }
}

/*!
    \internal
 */
QWebSocketProtocol::CloseCode QWebSocketFrame::closeCode() const
{
    return m_closeCode;
}

/*!
    \internal
 */
QString QWebSocketFrame::closeReason() const
{
    return m_closeReason;
}

/*!
    \internal
 */
bool QWebSocketFrame::isFinalFrame() const
{
    return m_isFinalFrame;
}

/*!
    \internal
 */
bool QWebSocketFrame::isControlFrame() const
{
    return (m_opCode & 0x08) == 0x08;
}

/*!
    \internal
 */
bool QWebSocketFrame::isDataFrame() const
{
    return !isControlFrame();
}

/*!
    \internal
 */
bool QWebSocketFrame::isContinuationFrame() const
{
    return isDataFrame() && (m_opCode == QWebSocketProtocol::OC_CONTINUE);
}

/*!
    \internal
 */
bool QWebSocketFrame::hasMask() const
{
    return m_mask != 0;
}

/*!
    \internal
 */
quint32 QWebSocketFrame::mask() const
{
    return m_mask;
}

/*!
    \internal
 */
int QWebSocketFrame::rsv1() const
{
    return m_rsv1;
}

/*!
    \internal
 */
int QWebSocketFrame::rsv2() const
{
    return m_rsv2;
}

/*!
    \internal
 */
int QWebSocketFrame::rsv3() const
{
    return m_rsv3;
}

/*!
    \internal
 */
QWebSocketProtocol::OpCode QWebSocketFrame::opCode() const
{
    return m_opCode;
}

/*!
    \internal
 */
QByteArray QWebSocketFrame::payload() const
{
    return m_payload;
}

/*!
    \internal
 */
void QWebSocketFrame::clear()
{
    m_closeCode = QWebSocketProtocol::CC_NORMAL;
    m_closeReason.clear();
    m_isFinalFrame = true;
    m_mask = 0;
    m_rsv1 = 0;
    m_rsv2 =0;
    m_rsv3 = 0;
    m_opCode = QWebSocketProtocol::OC_RESERVED_C;
    m_length = 0;
    m_payload.clear();
    m_isValid = false;
}

/*!
    \internal
 */
bool QWebSocketFrame::isValid() const
{
    return m_isValid;
}

#define WAIT_FOR_MORE_DATA(dataSizeInBytes)  { returnState = processingState; processingState = PS_WAIT_FOR_MORE_DATA; dataWaitSize = dataSizeInBytes; }

/*!
    \internal
 */
QWebSocketFrame QWebSocketFrame::readFrame(QIODevice *pIoDevice)
{
    bool isDone = false;
    qint64 bytesRead = 0;
    QWebSocketFrame frame;
    quint64 dataWaitSize = 0;
    Q_UNUSED(dataWaitSize); // value is used in MACRO, Q_UNUSED to avoid compiler warnings
    ProcessingState processingState = PS_READ_HEADER;
    ProcessingState returnState = PS_READ_HEADER;
    bool hasMask = false;
    quint64 payloadLength = 0;

    while (!isDone)
    {
        switch (processingState)
        {
            case PS_WAIT_FOR_MORE_DATA:
            {
                //TODO: waitForReadyRead should really be changed
                //now, when a websocket is used in a GUI thread
                //the GUI will hang for at most 5 seconds
                //maybe, a QStateMachine should be used
                bool ok = pIoDevice->waitForReadyRead(5000);
                if (!ok) {
                    frame.setError(QWebSocketProtocol::CC_GOING_AWAY, QObject::tr("Timeout when reading data from socket."));
                    processingState = PS_DISPATCH_RESULT;
                } else {
                    processingState = returnState;
                }
                break;
            }
            case PS_READ_HEADER:
            {
                if (pIoDevice->bytesAvailable() >= 2) {
                    //FIN, RSV1-3, Opcode
                    char header[2] = {0};
                    bytesRead = pIoDevice->read(header, 2);
                    frame.m_isFinalFrame = (header[0] & 0x80) != 0;
                    frame.m_rsv1 = (header[0] & 0x40);
                    frame.m_rsv2 = (header[0] & 0x20);
                    frame.m_rsv3 = (header[0] & 0x10);
                    frame.m_opCode = static_cast<QWebSocketProtocol::OpCode>(header[0] & 0x0F);

                    //Mask, PayloadLength
                    hasMask = (header[1] & 0x80) != 0;
                    frame.m_length = (header[1] & 0x7F);

                    switch (frame.m_length)
                    {
                        case 126:
                        {
                            processingState = PS_READ_PAYLOAD_LENGTH;
                            break;
                        }
                        case 127:
                        {
                            processingState = PS_READ_BIG_PAYLOAD_LENGTH;
                            break;
                        }
                        default:
                        {
                            payloadLength = frame.m_length;
                            processingState = hasMask ? PS_READ_MASK : PS_READ_PAYLOAD;
                            break;
                        }
                    }
                    if (!frame.checkValidity()) {
                        processingState = PS_DISPATCH_RESULT;
                    }
                } else {
                    WAIT_FOR_MORE_DATA(2);
                }
                break;
            }

            case PS_READ_PAYLOAD_LENGTH:
            {
                if (pIoDevice->bytesAvailable() >= 2) {
                    uchar length[2] = {0};
                    bytesRead = pIoDevice->read(reinterpret_cast<char *>(length), 2);
                    if (bytesRead == -1) {
                        frame.setError(QWebSocketProtocol::CC_GOING_AWAY, QObject::tr("Error occurred while reading from the network: %1").arg(pIoDevice->errorString()));
                        processingState = PS_DISPATCH_RESULT;
                    } else {
                        payloadLength = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(length));
                        if (payloadLength < 126) {
                            //see http://tools.ietf.org/html/rfc6455#page-28 paragraph 5.2
                            //"in all cases, the minimal number of bytes MUST be used to encode
                            //the length, for example, the length of a 124-byte-long string
                            //can't be encoded as the sequence 126, 0, 124"
                            frame.setError(QWebSocketProtocol::CC_PROTOCOL_ERROR, QObject::tr("Lengths smaller than 126 must be expressed as one byte."));
                            processingState = PS_DISPATCH_RESULT;
                        } else {
                            processingState = hasMask ? PS_READ_MASK : PS_READ_PAYLOAD;
                        }
                    }
                } else {
                    WAIT_FOR_MORE_DATA(2);
                }
                break;
            }

            case PS_READ_BIG_PAYLOAD_LENGTH:
            {
                if (pIoDevice->bytesAvailable() >= 8) {
                    uchar length[8] = {0};
                    bytesRead = pIoDevice->read(reinterpret_cast<char *>(length), 8);
                    if (bytesRead < 8) {
                        frame.setError(QWebSocketProtocol::CC_ABNORMAL_DISCONNECTION, QObject::tr("Something went wrong during reading from the network."));
                        processingState = PS_DISPATCH_RESULT;
                    } else {
                        //Most significant bit must be set to 0 as per http://tools.ietf.org/html/rfc6455#section-5.2
                        //We don't check for that. We just strip off the highest bit
                        payloadLength = qFromBigEndian<quint64>(length) & ~(1ULL << 63);
                        if (payloadLength <= 0xFFFFu) {
                            //see http://tools.ietf.org/html/rfc6455#page-28 paragraph 5.2
                            //"in all cases, the minimal number of bytes MUST be used to encode
                            //the length, for example, the length of a 124-byte-long string
                            //can't be encoded as the sequence 126, 0, 124"
                            frame.setError(QWebSocketProtocol::CC_PROTOCOL_ERROR, QObject::tr("Lengths smaller than 65536 (2^16) must be expressed as 2 bytes."));
                            processingState = PS_DISPATCH_RESULT;
                        } else {
                            processingState = hasMask ? PS_READ_MASK : PS_READ_PAYLOAD;
                        }
                    }
                } else {
                    WAIT_FOR_MORE_DATA(8);
                }

                break;
            }

            case PS_READ_MASK:
            {
                if (pIoDevice->bytesAvailable() >= 4) {
                    bytesRead = pIoDevice->read(reinterpret_cast<char *>(&frame.m_mask), sizeof(frame.m_mask));
                    if (bytesRead == -1) {
                        frame.setError(QWebSocketProtocol::CC_GOING_AWAY, QObject::tr("Error while reading from the network: %1.").arg(pIoDevice->errorString()));
                        processingState = PS_DISPATCH_RESULT;
                    } else {
                        frame.m_mask = qFromBigEndian(frame.m_mask);
                        processingState = PS_READ_PAYLOAD;
                    }
                } else {
                    WAIT_FOR_MORE_DATA(4);
                }
                break;
            }

            case PS_READ_PAYLOAD:
            {
                if (!payloadLength) {
                    processingState = PS_DISPATCH_RESULT;
                } else if (payloadLength > MAX_FRAME_SIZE_IN_BYTES) {
                    frame.setError(QWebSocketProtocol::CC_TOO_MUCH_DATA, QObject::tr("Maximum framesize exceeded."));
                    processingState = PS_DISPATCH_RESULT;
                } else {
                    quint64 bytesAvailable = quint64(pIoDevice->bytesAvailable());
                    if (bytesAvailable >= payloadLength) {
                        frame.m_payload = pIoDevice->read(payloadLength);
                        //payloadLength can be safely cast to an integer, as the MAX_FRAME_SIZE_IN_BYTES = MAX_INT
                        if (frame.m_payload.length() != int(payloadLength)) {
                            //some error occurred; refer to the Qt documentation of QIODevice::read()
                            frame.setError(QWebSocketProtocol::CC_ABNORMAL_DISCONNECTION, QObject::tr("Some serious error occurred while reading from the network."));
                            processingState = PS_DISPATCH_RESULT;
                        } else {
                            if (hasMask) {
                                QWebSocketProtocol::mask(&frame.m_payload, frame.m_mask);
                            }
                            processingState = PS_DISPATCH_RESULT;
                        }
                    } else {
                        WAIT_FOR_MORE_DATA(payloadLength);  //if payload is too big, then this will timeout
                    }
                }
                break;
            }

            case PS_DISPATCH_RESULT:
            {
                processingState = PS_READ_HEADER;
                isDone = true;
                break;
            }

            default:
            {
                //should not come here
                qWarning() << "DataProcessor::process: Found invalid state. This should not happen!";
                frame.clear();
                isDone = true;
                break;
            }
        }	//end switch
    }

    return frame;
}

/*!
    \internal
 */
void QWebSocketFrame::setError(QWebSocketProtocol::CloseCode code, QString closeReason)
{
    clear();
    m_closeCode = code;
    m_closeReason = closeReason;
    m_isValid = false;
}

/*!
    \internal
 */
bool QWebSocketFrame::checkValidity()
{
    if (isValid()) {
        return true;
    }
    if (m_rsv1 || m_rsv2 || m_rsv3) {
        setError(QWebSocketProtocol::CC_PROTOCOL_ERROR, QObject::tr("Rsv field is non-zero"));
    } else if (QWebSocketProtocol::isOpCodeReserved(m_opCode)) {
        setError(QWebSocketProtocol::CC_PROTOCOL_ERROR, QObject::tr("Used reserved opcode"));
    } else if (isControlFrame()) {
        if (m_length > 125) {
            setError(QWebSocketProtocol::CC_PROTOCOL_ERROR, QObject::tr("Controle frame is larger than 125 bytes"));
        } else if (!m_isFinalFrame) {
            setError(QWebSocketProtocol::CC_PROTOCOL_ERROR, QObject::tr("Controle frames cannot be fragmented"));
        } else {
            m_isValid = true;
        }
    } else {
        m_isValid = true;
    }
    return m_isValid;
}

QT_END_NAMESPACE
