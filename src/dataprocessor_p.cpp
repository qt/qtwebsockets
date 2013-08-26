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

#include "dataprocessor_p.h"
#include "qwebsocketprotocol.h"
#include <QIODevice>
#include <QtEndian>
#include <limits.h>
#include <QTextCodec>
#include <QTextDecoder>
#include <QDebug>

QT_BEGIN_NAMESPACE

const quint64 MAX_FRAME_SIZE_IN_BYTES = INT_MAX - 1;
const quint64 MAX_MESSAGE_SIZE_IN_BYTES = INT_MAX - 1;

/*!
    \internal
 */
class Frame
{
public:
    Frame();
    Frame(const Frame &other);

    const Frame &operator =(const Frame &other);

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

    static Frame readFrame(QIODevice *pIoDevice);

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

/*!
    \internal
 */
Frame::Frame() :
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
Frame::Frame(const Frame &other) :
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
const Frame &Frame::operator =(const Frame &other)
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
QWebSocketProtocol::CloseCode Frame::getCloseCode() const
{
    return m_closeCode;
}

/*!
    \internal
 */
QString Frame::getCloseReason() const
{
    return m_closeReason;
}

/*!
    \internal
 */
bool Frame::isFinalFrame() const
{
    return m_isFinalFrame;
}

/*!
    \internal
 */
bool Frame::isControlFrame() const
{
    return (m_opCode & 0x08) == 0x08;
}

/*!
    \internal
 */
bool Frame::isDataFrame() const
{
    return !isControlFrame();
}

/*!
    \internal
 */
bool Frame::isContinuationFrame() const
{
    return isDataFrame() && (m_opCode == QWebSocketProtocol::OC_CONTINUE);
}

/*!
    \internal
 */
bool Frame::hasMask() const
{
    return m_mask != 0;
}

/*!
    \internal
 */
quint32 Frame::getMask() const
{
    return m_mask;
}

/*!
    \internal
 */
int Frame::getRsv1() const
{
    return m_rsv1;
}

/*!
    \internal
 */
int Frame::getRsv2() const
{
    return m_rsv2;
}

/*!
    \internal
 */
int Frame::getRsv3() const
{
    return m_rsv3;
}

/*!
    \internal
 */
QWebSocketProtocol::OpCode Frame::getOpCode() const
{
    return m_opCode;
}

/*!
    \internal
 */
QByteArray Frame::getPayload() const
{
    return m_payload;
}

/*!
    \internal
 */
void Frame::clear()
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
bool Frame::isValid() const
{
    return m_isValid;
}

#define WAIT_FOR_MORE_DATA(dataSizeInBytes)  { returnState = processingState; processingState = PS_WAIT_FOR_MORE_DATA; dataWaitSize = dataSizeInBytes; }

/*!
    \internal
 */
Frame Frame::readFrame(QIODevice *pIoDevice)
{
    bool isDone = false;
    qint64 bytesRead = 0;
    Frame frame;
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
                    isDone = true;
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
                        isDone = true;
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
                    //TODO: Handle return value
                    bytesRead = pIoDevice->read(reinterpret_cast<char *>(length), 2);
                    payloadLength = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(length));
                    processingState = hasMask ? PS_READ_MASK : PS_READ_PAYLOAD;
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
                    //TODO: Handle return value
                    bytesRead = pIoDevice->read(reinterpret_cast<char *>(length), 8);
                    //Most significant bit must be set to 0 as per http://tools.ietf.org/html/rfc6455#section-5.2
                    //TODO: Do we check for that?
                    payloadLength = qFromBigEndian<quint64>(length) & ~(1ULL << 63);
                    processingState = hasMask ? PS_READ_MASK : PS_READ_PAYLOAD;
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
                    //TODO: Handle return value
                    bytesRead = pIoDevice->read(reinterpret_cast<char *>(&frame.m_mask), sizeof(frame.m_mask));
                    processingState = PS_READ_PAYLOAD;
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
                        if (hasMask)
                        {
                            QWebSocketProtocol::mask(&frame.m_payload, frame.m_mask);
                        }
                        processingState = PS_DISPATCH_RESULT;
                    }
                    else
                    {
                        WAIT_FOR_MORE_DATA(payloadLength);
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
void Frame::setError(QWebSocketProtocol::CloseCode code, QString closeReason)
{
    clear();
    m_closeCode = code;
    m_closeReason = closeReason;
    m_isValid = false;
}

/*!
    \internal
 */
bool Frame::checkValidity()
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

/*!
    \internal
 */
DataProcessor::DataProcessor(QObject *parent) :
    QObject(parent),
    m_processingState(PS_READ_HEADER),
    m_isFinalFrame(false),
    m_isFragmented(false),
    m_opCode(QWebSocketProtocol::OC_CLOSE),
    m_isControlFrame(false),
    m_hasMask(false),
    m_mask(0),
    m_binaryMessage(),
    m_textMessage(),
    m_payloadLength(0),
    m_pConverterState(0),
    m_pTextCodec(QTextCodec::codecForName("UTF-8"))
{
    clear();
}

/*!
    \internal
 */
DataProcessor::~DataProcessor()
{
    clear();
    if (m_pConverterState)
    {
        delete m_pConverterState;
        m_pConverterState = 0;
    }
}

/*!
    \internal
 */
void DataProcessor::process(QIODevice *pIoDevice)
{
    bool isDone = false;

    while (!isDone)
    {
        Frame frame = Frame::readFrame(pIoDevice);
        if (frame.isValid())
        {
            if (frame.isControlFrame())
            {
                Q_EMIT controlFrameReceived(frame.getOpCode(), frame.getPayload());
                isDone = true;  //exit the loop after a control frame, so we can get a chance to close the socket if necessary
            }
            else    //we have a dataframe; opcode can be OC_CONTINUE, OC_TEXT or OC_BINARY
            {
                if (!m_isFragmented && frame.isContinuationFrame())
                {
                    clear();
                    Q_EMIT errorEncountered(QWebSocketProtocol::CC_PROTOCOL_ERROR, tr("Received Continuation frame /*with FIN=true*/, while there is nothing to continue."));
                    return;
                }
                if (m_isFragmented && frame.isDataFrame() && !frame.isContinuationFrame())
                {
                    clear();
                    Q_EMIT errorEncountered(QWebSocketProtocol::CC_PROTOCOL_ERROR, tr("All data frames after the initial data frame must have opcode 0 (continuation)."));
                    return;
                }
                if (!frame.isContinuationFrame())
                {
                    m_opCode = frame.getOpCode();
                    m_isFragmented = !frame.isFinalFrame();
                }
                quint64 messageLength = (quint64)(m_opCode == QWebSocketProtocol::OC_TEXT) ? m_textMessage.length() : m_binaryMessage.length();
                if ((messageLength + quint64(frame.getPayload().length())) > MAX_MESSAGE_SIZE_IN_BYTES)
                {
                    clear();
                    Q_EMIT errorEncountered(QWebSocketProtocol::CC_TOO_MUCH_DATA, tr("Received message is too big."));
                    return;
                }

                if (m_opCode == QWebSocketProtocol::OC_TEXT)
                {
                    QString frameTxt = m_pTextCodec->toUnicode(frame.getPayload().constData(), frame.getPayload().size(), m_pConverterState);
                    bool failed = (m_pConverterState->invalidChars != 0) || (frame.isFinalFrame() && (m_pConverterState->remainingChars != 0));
                    if (failed)
                    {
                        clear();
                        Q_EMIT errorEncountered(QWebSocketProtocol::CC_WRONG_DATATYPE, tr("Invalid UTF-8 code encountered."));
                        return;
                    }
                    else
                    {
                        m_textMessage.append(frameTxt);
                        Q_EMIT textFrameReceived(frameTxt, frame.isFinalFrame());
                    }
                }
                else
                {
                    m_binaryMessage.append(frame.getPayload());
                    Q_EMIT binaryFrameReceived(frame.getPayload(), frame.isFinalFrame());
                }

                if (frame.isFinalFrame())
                {
                    if (m_opCode == QWebSocketProtocol::OC_TEXT)
                    {
                        Q_EMIT textMessageReceived(m_textMessage);
                    }
                    else
                    {
                        Q_EMIT binaryMessageReceived(m_binaryMessage);
                    }
                    clear();
                    isDone = true;
                }
            }
        }
        else
        {
            Q_EMIT errorEncountered(frame.getCloseCode(), frame.getCloseReason());
            clear();
            isDone = true;
        }
    }
}

/*!
    \internal
 */
void DataProcessor::clear()
{
    m_processingState = PS_READ_HEADER;
    m_isFinalFrame = false;
    m_isFragmented = false;
    m_opCode = QWebSocketProtocol::OC_CLOSE;
    m_hasMask = false;
    m_mask = 0;
    m_binaryMessage.clear();
    m_textMessage.clear();
    m_payloadLength = 0;
    if (m_pConverterState)
    {
        if ((m_pConverterState->remainingChars != 0) || (m_pConverterState->invalidChars != 0))
        {
            delete m_pConverterState;
            m_pConverterState = 0;
        }
    }
    if (!m_pConverterState)
    {
        m_pConverterState = new QTextCodec::ConverterState(QTextCodec::ConvertInvalidToNull | QTextCodec::IgnoreHeader);
    }
}

QT_END_NAMESPACE
