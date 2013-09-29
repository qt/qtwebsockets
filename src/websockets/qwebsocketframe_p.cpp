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

#include "qwebsocketframe_p.h"

#include <QtEndian>
#include <QDebug>

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
const QWebSocketFrame &QWebSocketFrame::operator =(const QWebSocketFrame &other)
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

/*!
    \internal
 */
QWebSocketProtocol::CloseCode QWebSocketFrame::getCloseCode() const
{
    return m_closeCode;
}

/*!
    \internal
 */
QString QWebSocketFrame::getCloseReason() const
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
quint32 QWebSocketFrame::getMask() const
{
    return m_mask;
}

/*!
    \internal
 */
int QWebSocketFrame::getRsv1() const
{
    return m_rsv1;
}

/*!
    \internal
 */
int QWebSocketFrame::getRsv2() const
{
    return m_rsv2;
}

/*!
    \internal
 */
int QWebSocketFrame::getRsv3() const
{
    return m_rsv3;
}

/*!
    \internal
 */
QWebSocketProtocol::OpCode QWebSocketFrame::getOpCode() const
{
    return m_opCode;
}

/*!
    \internal
 */
QByteArray QWebSocketFrame::getPayload() const
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
                bool ok = pIoDevice->waitForReadyRead(5000);
                if (!ok)
                {
                    frame.setError(QWebSocketProtocol::CC_GOING_AWAY, QObject::tr("Timeout when reading data from socket."));
                    processingState = PS_DISPATCH_RESULT;
                }
                else
                {
                    processingState = returnState;
                }
                break;
            }
            case PS_READ_HEADER:
            {
                if (pIoDevice->bytesAvailable() >= 2)
                {
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
                    if (!frame.checkValidity())
                    {
                        processingState = PS_DISPATCH_RESULT;
                    }
                }
                else
                {
                    WAIT_FOR_MORE_DATA(2);
                }
                break;
            }

            case PS_READ_PAYLOAD_LENGTH:
            {
                if (pIoDevice->bytesAvailable() >= 2)
                {
                    uchar length[2] = {0};
                    bytesRead = pIoDevice->read(reinterpret_cast<char *>(length), 2);
                    if (bytesRead == -1)
                    {
                        frame.setError(QWebSocketProtocol::CC_GOING_AWAY, QObject::tr("Error occurred while reading from the network: %1").arg(pIoDevice->errorString()));
                        processingState = PS_DISPATCH_RESULT;
                    }
                    else
                    {
                        payloadLength = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(length));
                        if (payloadLength < 126)
                        {
                            //see http://tools.ietf.org/html/rfc6455#page-28 paragraph 5.2
                            //"in all cases, the minimal number of bytes MUST be used to encode
                            //the length, for example, the length of a 124-byte-long string
                            //can't be encoded as the sequence 126, 0, 124"
                            frame.setError(QWebSocketProtocol::CC_PROTOCOL_ERROR, QObject::tr("Lengths smaller than 126 must be expressed as one byte."));
                            processingState = PS_DISPATCH_RESULT;
                        }
                        else
                        {
                            processingState = hasMask ? PS_READ_MASK : PS_READ_PAYLOAD;
                        }
                    }
                }
                else
                {
                    WAIT_FOR_MORE_DATA(2);
                }
                break;
            }

            case PS_READ_BIG_PAYLOAD_LENGTH:
            {
                if (pIoDevice->bytesAvailable() >= 8)
                {
                    uchar length[8] = {0};
                    bytesRead = pIoDevice->read(reinterpret_cast<char *>(length), 8);
                    if (bytesRead < 8)
                    {
                        frame.setError(QWebSocketProtocol::CC_ABNORMAL_DISCONNECTION, QObject::tr("Something went wrong during reading from the network."));
                        processingState = PS_DISPATCH_RESULT;
                    }
                    else
                    {
                        //Most significant bit must be set to 0 as per http://tools.ietf.org/html/rfc6455#section-5.2
                        //TODO: Do we check for that? Now we just strip off the highest bit
                        payloadLength = qFromBigEndian<quint64>(length) & ~(1ULL << 63);
                        if (payloadLength <= 0xFFFFu)
                        {
                            //see http://tools.ietf.org/html/rfc6455#page-28 paragraph 5.2
                            //"in all cases, the minimal number of bytes MUST be used to encode
                            //the length, for example, the length of a 124-byte-long string
                            //can't be encoded as the sequence 126, 0, 124"
                            frame.setError(QWebSocketProtocol::CC_PROTOCOL_ERROR, QObject::tr("Lengths smaller than 65536 (2^16) must be expressed as 2 bytes."));
                            processingState = PS_DISPATCH_RESULT;
                        }
                        else
                        {
                            processingState = hasMask ? PS_READ_MASK : PS_READ_PAYLOAD;
                        }
                    }
                }
                else
                {
                    WAIT_FOR_MORE_DATA(8);
                }

                break;
            }

            case PS_READ_MASK:
            {
                if (pIoDevice->bytesAvailable() >= 4)
                {
                    bytesRead = pIoDevice->read(reinterpret_cast<char *>(&frame.m_mask), sizeof(frame.m_mask));
                    if (bytesRead == -1)
                    {
                        frame.setError(QWebSocketProtocol::CC_GOING_AWAY, QObject::tr("Error while reading from the network: %1.").arg(pIoDevice->errorString()));
                        processingState = PS_DISPATCH_RESULT;
                    }
                    else
                    {
                        processingState = PS_READ_PAYLOAD;
                    }
                }
                else
                {
                    WAIT_FOR_MORE_DATA(4);
                }
                break;
            }

            case PS_READ_PAYLOAD:
            {
                if (!payloadLength)
                {
                    processingState = PS_DISPATCH_RESULT;
                }
                else if (payloadLength > MAX_FRAME_SIZE_IN_BYTES)
                {
                    frame.setError(QWebSocketProtocol::CC_TOO_MUCH_DATA, QObject::tr("Maximum framesize exceeded."));
                    processingState = PS_DISPATCH_RESULT;
                }
                else
                {
                    quint64 bytesAvailable = static_cast<quint64>(pIoDevice->bytesAvailable());
                    if (bytesAvailable >= payloadLength)
                    {
                        frame.m_payload = pIoDevice->read(payloadLength);
                        //payloadLength can be safely cast to an integer, as the MAX_FRAME_SIZE_IN_BYTES = MAX_INT
                        if (frame.m_payload.length() != static_cast<int>(payloadLength))  //some error occurred; refer to the Qt documentation
                        {
                            frame.setError(QWebSocketProtocol::CC_ABNORMAL_DISCONNECTION, QObject::tr("Some serious error occurred while reading from the network."));
                            processingState = PS_DISPATCH_RESULT;
                        }
                        else
                        {
                            if (hasMask)
                            {
                                QWebSocketProtocol::mask(&frame.m_payload, frame.m_mask);
                            }
                            processingState = PS_DISPATCH_RESULT;
                        }
                    }
                    else
                    {
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
    if (!isValid())
    {
        if (m_rsv1 || m_rsv2 || m_rsv3)
        {
            setError(QWebSocketProtocol::CC_PROTOCOL_ERROR, QObject::tr("Rsv field is non-zero"));
        }
        else if (QWebSocketProtocol::isOpCodeReserved(m_opCode))
        {
            setError(QWebSocketProtocol::CC_PROTOCOL_ERROR, QObject::tr("Used reserved opcode"));
        }
        else if (isControlFrame())
        {
            if (m_length > 125)
            {
                setError(QWebSocketProtocol::CC_PROTOCOL_ERROR, QObject::tr("Controle frame is larger than 125 bytes"));
            }
            else if (!m_isFinalFrame)
            {
                setError(QWebSocketProtocol::CC_PROTOCOL_ERROR, QObject::tr("Controle frames cannot be fragmented"));
            }
            else
            {
                m_isValid = true;
            }
        }
        else
        {
            m_isValid = true;
        }
    }
    return m_isValid;
}
