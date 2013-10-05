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
/*!
    \class QWebSocketDataProcessor
    The class QWebSocketDataProcessor is responsible for reading, validating and interpreting data from a websocket.
    It reads data from a QIODevice, validates it against RFC 6455, and parses it into frames (data, control).
    It emits signals that correspond to the type of the frame: textFrameReceived(), binaryFrameReceived(),
    textMessageReceived(), binaryMessageReceived(), pingReceived(), pongReceived() and closeReceived().
    Whenever an error is detected, the errorEncountered() signal is emitted.
    QWebSocketDataProcessor also checks if a frame is allowed in a sequence of frames (e.g. a continuation frame cannot follow a final frame).
    This class is an internal class used by QWebSocketInternal for data processing and validation.

    \sa Frame()

    \internal
*/
#include "qwebsocketdataprocessor_p.h"
#include "qwebsocketprotocol.h"
#include "qwebsocketframe_p.h"
#include <QtEndian>
#include <limits.h>
#include <QTextCodec>
#include <QTextDecoder>
#include <QDebug>

QT_BEGIN_NAMESPACE

/*!
    \internal
 */
QWebSocketDataProcessor::QWebSocketDataProcessor(QObject *parent) :
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
QWebSocketDataProcessor::~QWebSocketDataProcessor()
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
quint64 QWebSocketDataProcessor::maxMessageSize()
{
    return MAX_MESSAGE_SIZE_IN_BYTES;
}

/*!
    \internal
 */
quint64 QWebSocketDataProcessor::maxFrameSize()
{
    return MAX_FRAME_SIZE_IN_BYTES;
}

/*!
    \internal
 */
void QWebSocketDataProcessor::process(QIODevice *pIoDevice)
{
    bool isDone = false;

    while (!isDone)
    {
        QWebSocketFrame frame = QWebSocketFrame::readFrame(pIoDevice);
        if (frame.isValid())
        {
            if (frame.isControlFrame())
            {
                isDone = processControlFrame(frame);
            }
            else    //we have a dataframe; opcode can be OC_CONTINUE, OC_TEXT or OC_BINARY
            {
                if (!m_isFragmented && frame.isContinuationFrame())
                {
                    clear();
                    Q_EMIT errorEncountered(QWebSocketProtocol::CC_PROTOCOL_ERROR, tr("Received Continuation frame, while there is nothing to continue."));
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
void QWebSocketDataProcessor::clear()
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

/*!
    \internal
 */
bool QWebSocketDataProcessor::processControlFrame(const QWebSocketFrame &frame)
{
    bool mustStopProcessing = false;
    switch (frame.getOpCode())
    {
    case QWebSocketProtocol::OC_PING:
    {
        Q_EMIT pingReceived(frame.getPayload());
        break;
    }
    case QWebSocketProtocol::OC_PONG:
    {
        Q_EMIT pongReceived(frame.getPayload());
        break;
    }
    case QWebSocketProtocol::OC_CLOSE:
    {
        quint16 closeCode = QWebSocketProtocol::CC_NORMAL;
        QString closeReason;
        QByteArray payload = frame.getPayload();
        if (payload.size() == 1)    //size is either 0 (no close code and no reason) or >= 2 (at least a close code of 2 bytes)
        {
            closeCode = QWebSocketProtocol::CC_PROTOCOL_ERROR;
            closeReason = tr("Payload of close frame is too small.");
        }
        else if (payload.size() > 1)   //close frame can have a close code and reason
        {
            closeCode = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(payload.constData()));
            if (!QWebSocketProtocol::isCloseCodeValid(closeCode))
            {
                closeCode = QWebSocketProtocol::CC_PROTOCOL_ERROR;
                closeReason = tr("Invalid close code %1 detected.").arg(closeCode);
            }
            else
            {
                if (payload.size() > 2)
                {
                    QTextCodec *tc = QTextCodec::codecForName("UTF-8");
                    QTextCodec::ConverterState state(QTextCodec::ConvertInvalidToNull);
                    closeReason = tc->toUnicode(payload.constData() + 2, payload.size() - 2, &state);
                    bool failed = (state.invalidChars != 0) || (state.remainingChars != 0);
                    if (failed)
                    {
                        closeCode = QWebSocketProtocol::CC_WRONG_DATATYPE;
                        closeReason = tr("Invalid UTF-8 code encountered.");
                    }
                }
            }
        }
        mustStopProcessing = true;
        Q_EMIT closeReceived(static_cast<QWebSocketProtocol::CloseCode>(closeCode), closeReason);
        break;
    }
    case QWebSocketProtocol::OC_CONTINUE:
    case QWebSocketProtocol::OC_BINARY:
    case QWebSocketProtocol::OC_TEXT:
    case QWebSocketProtocol::OC_RESERVED_3:
    case QWebSocketProtocol::OC_RESERVED_4:
    case QWebSocketProtocol::OC_RESERVED_5:
    case QWebSocketProtocol::OC_RESERVED_6:
    case QWebSocketProtocol::OC_RESERVED_7:
    case QWebSocketProtocol::OC_RESERVED_C:
    case QWebSocketProtocol::OC_RESERVED_B:
    case QWebSocketProtocol::OC_RESERVED_D:
    case QWebSocketProtocol::OC_RESERVED_E:
    case QWebSocketProtocol::OC_RESERVED_F:
    {
        //do nothing
        //case added to make C++ compiler happy
        break;
    }
    default:
    {
        qDebug() << "DataProcessor::processControlFrame: Invalid opcode detected:" << static_cast<int>(frame.getOpCode());
        //Do nothing
        break;
    }
    }
    return mustStopProcessing;
}

QT_END_NAMESPACE
