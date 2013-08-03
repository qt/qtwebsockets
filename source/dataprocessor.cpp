#include "dataprocessor.h"
#include "websocketprotocol.h"
#include <QTcpSocket>
#include <QtEndian>

class Fragment
{
public:
    Fragment();
    Fragment(const Fragment &other);

    const Fragment &operator =(const Fragment &other);

    WebSocketProtocol::CloseCode getCloseCode() const;
    QString getCloseReason() const;
    bool isFinalFragment() const;
    bool isControlFragment() const;
    bool isDataFragment() const;
    bool isContinuationFragment() const;
    bool hasMask() const;
    quint32 getMask() const;    //returns 0 if no mask
    int getRsv1() const;
    int getRsv2() const;
    int getRsv3() const;
    WebSocketProtocol::OpCode getOpCode() const;
    QByteArray getPayload() const;

    void clear();       //resets all member variables, and invalidates the object

    bool isValid() const;

    static Fragment readFragment(QTcpSocket *pSocket);

private:
    //header
    WebSocketProtocol::CloseCode m_closeCode;
    QString m_closeReason;
    bool m_isFinalFragment;
    quint32 m_mask;
    int m_rsv1; //reserved field 1
    int m_rsv2; //reserved field 2
    int m_rsv3; //reserved field 3
    WebSocketProtocol::OpCode m_opCode;

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

    void setError(WebSocketProtocol::CloseCode code, QString closeReason);
    bool checkValidity();
};

Fragment::Fragment() :
    m_closeCode(WebSocketProtocol::CC_NORMAL),
    m_closeReason(),
    m_isFinalFragment(true),
    m_mask(0),
    m_rsv1(0),
    m_rsv2(0),
    m_rsv3(0),
    m_opCode(WebSocketProtocol::OC_RESERVED_V),
    m_length(0),
    m_payload(),
    m_isValid(false)
{
}

Fragment::Fragment(const Fragment &other) :
    m_closeCode(other.m_closeCode),
    m_closeReason(other.m_closeReason),
    m_isFinalFragment(other.m_isFinalFragment),
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

const Fragment &Fragment::operator =(const Fragment &other)
{
    m_closeCode = other.m_closeCode;
    m_closeReason = other.m_closeReason;
    m_isFinalFragment = other.m_isFinalFragment;
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

WebSocketProtocol::CloseCode Fragment::getCloseCode() const
{
    return m_closeCode;
}

QString Fragment::getCloseReason() const
{
    return m_closeReason;
}

bool Fragment::isFinalFragment() const
{
    return m_isFinalFragment;
}

bool Fragment::isControlFragment() const
{
    return (m_opCode & 0x08) == 0x08;
}

bool Fragment::isDataFragment() const
{
    return !isControlFragment();
}

bool Fragment::isContinuationFragment() const
{
    return isDataFragment() && (m_opCode == WebSocketProtocol::OC_CONTINUE);
}

bool Fragment::hasMask() const
{
    return m_mask != 0;
}

quint32 Fragment::getMask() const
{
    return m_mask;
}

int Fragment::getRsv1() const
{
    return m_rsv1;
}

int Fragment::getRsv2() const
{
    return m_rsv2;
}

int Fragment::getRsv3() const
{
    return m_rsv3;
}

WebSocketProtocol::OpCode Fragment::getOpCode() const
{
    return m_opCode;
}

QByteArray Fragment::getPayload() const
{
    return m_payload;
}

void Fragment::clear()
{
    m_closeCode = WebSocketProtocol::CC_NORMAL;
    m_closeReason.clear();
    m_isFinalFragment = true;
    m_mask = 0;
    m_rsv1 = 0;
    m_rsv2 =0;
    m_rsv3 = 0;
    m_opCode = WebSocketProtocol::OC_RESERVED_V;
    m_length = 0;
    m_payload.clear();
    m_isValid = false;
}

bool Fragment::isValid() const
{
    return m_isValid;
}

#define WAIT_FOR_MORE_DATA(dataSizeInBytes)  { returnState = processingState; processingState = PS_WAIT_FOR_MORE_DATA; dataWaitSize = dataSizeInBytes; }

Fragment Fragment::readFragment(QTcpSocket *pSocket)
{
    bool isDone = false;
    qint64 bytesRead = 0;
    Fragment fragment;
    ProcessingState processingState = PS_READ_HEADER;
    ProcessingState returnState = PS_READ_HEADER;
    quint64 dataWaitSize = 0;
    bool hasMask = false;
    quint64 payloadLength = 0;

    while (!isDone)
    {
        switch (processingState)
        {
            case PS_WAIT_FOR_MORE_DATA:
            {
                bool result = pSocket->waitForReadyRead(1000);
                if (!result)                                      //timeout
                {
                    qDebug() << "Timeout" << dataWaitSize << "bytesAvail" << pSocket->bytesAvailable();
                    fragment.setError(WebSocketProtocol::CC_GOING_AWAY, "Timeout when reading data from socket.");
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
                if (pSocket->bytesAvailable() >= 2)
                {
                    //FIN, RSV1-3, Opcode
                    char header[2] = {0};
                    bytesRead = pSocket->read(header, 2);
                    fragment.m_isFinalFragment = (header[0] & 0x80) != 0;
                    fragment.m_rsv1 = (header[0] & 0x40);
                    fragment.m_rsv2 = (header[0] & 0x20);
                    fragment.m_rsv3 = (header[0] & 0x10);
                    fragment.m_opCode = static_cast<WebSocketProtocol::OpCode>(header[0] & 0x0F);

                    //Mask, PayloadLength
                    hasMask = (header[1] & 0x80) != 0;
                    fragment.m_length = (header[1] & 0x7F);

                    switch (fragment.m_length)
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
                            payloadLength = fragment.m_length;
                            processingState = hasMask ? PS_READ_MASK : PS_READ_PAYLOAD;
                            break;
                        }
                    }
                    if (!fragment.checkValidity())
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
                if (pSocket->bytesAvailable() >= 2)
                {
                    uchar length[2] = {0};
                     //TODO: Handle return value
                    bytesRead = pSocket->read(reinterpret_cast<char *>(length), 2);
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
                if (pSocket->bytesAvailable() >= 8)
                {
                    uchar length[8] = {0};
                    //TODO: Handle return value
                    bytesRead = pSocket->read(reinterpret_cast<char *>(length), 8);
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
                if (pSocket->bytesAvailable() >= 4)
                {
                    //TODO: Handle return value
                    bytesRead = pSocket->read(reinterpret_cast<char *>(&fragment.m_mask), sizeof(fragment.m_mask));
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
                // TODO: Handle large payloads
                if (!payloadLength)
                {
                    processingState = PS_DISPATCH_RESULT;
                }
                else
                {
                    qint64 bytesAvailable = pSocket->bytesAvailable();
                    if (bytesAvailable >= payloadLength)
                    {
                        //QByteArray payload = pSocket->read(bytesLeftToRead);
                        fragment.m_payload = pSocket->read(payloadLength);
                        if (hasMask)
                        {
                            WebSocketProtocol::mask(&fragment.m_payload, fragment.m_mask);
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
                qDebug() << "DataProcessor::process: Found invalid state. This should not happen!";
                fragment.clear();
                isDone = true;
                break;
            }
        }	//end switch
    }

    return fragment;
}

void Fragment::setError(WebSocketProtocol::CloseCode code, QString closeReason)
{
    clear();
    m_closeCode = code;
    m_closeReason = closeReason;
    m_isValid = false;
}

bool Fragment::checkValidity()
{
    if (!isValid())
    {
        if (m_rsv1 || m_rsv2 || m_rsv3)
        {
            setError(WebSocketProtocol::CC_PROTOCOL_ERROR, "Rsv field is non-zero");
        }
        else if (WebSocketProtocol::isOpCodeReserved(m_opCode))
        {
            setError(WebSocketProtocol::CC_PROTOCOL_ERROR, "Used reserved opcode");
        }
        else if (isControlFragment())
        {
            if (m_length > 125)
            {
                setError(WebSocketProtocol::CC_PROTOCOL_ERROR, "Controle frame is larger than 125 bytes");
            }
            else if (!m_isFinalFragment)
            {
                setError(WebSocketProtocol::CC_PROTOCOL_ERROR, "Controle frames cannot be fragmented");
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

DataProcessor::DataProcessor(QObject *parent) :
	QObject(parent),
    m_processingState(PS_READ_HEADER),
	m_isFinalFragment(false),
    m_isFragmented(false),
	m_opCode(WebSocketProtocol::OC_CLOSE),
    m_isControlFrame(false),
	m_hasMask(false),
	m_mask(0),
	m_frame(),
	m_payloadLength(0)
{
}

DataProcessor::~DataProcessor()
{
}

#if 1

void DataProcessor::process(QTcpSocket *pSocket)
{
    bool isDone = false;

    while (!isDone)
    {
        Fragment fragment = Fragment::readFragment(pSocket);
        if (fragment.isValid())
        {
            if (fragment.isControlFragment())
            {
                Q_EMIT frameReceived(fragment.getOpCode(), fragment.getPayload(), true);
                isDone = true;  //exit the loop after a control frame, so we can get a chance to close the socket if necessary
            }
            else    //we have a dataframe
            {
                if (!m_isFragmented && fragment.isContinuationFragment())
                {
                    clear();
                    Q_EMIT errorEncountered(WebSocketProtocol::CC_PROTOCOL_ERROR, "Received Continuation frame /*with FIN=true*/, while there is nothing to continue.");
                    return;
                }
                if (m_isFragmented && fragment.isDataFragment() && !fragment.isContinuationFragment())
                {
                    clear();
                    Q_EMIT errorEncountered(WebSocketProtocol::CC_PROTOCOL_ERROR, "All data frames after the initial data frame must have opcode 0");
                    return;
                }
                if (!fragment.isContinuationFragment())
                {
                    m_opCode = fragment.getOpCode();
                    m_isFragmented = !fragment.isFinalFragment();
                }
                m_frame.append(fragment.getPayload());
                if (fragment.isFinalFragment())
                {
                    Q_EMIT frameReceived(m_opCode, m_frame, m_isFinalFragment);
                    clear();
                    isDone = true;
                }
            }
        }
        else
        {
            Q_EMIT errorEncountered(fragment.getCloseCode(), fragment.getCloseReason());
            clear();
            isDone = true;
        }
    }
}
#else
void DataProcessor::process(QTcpSocket *pSocket)
{
	qint64 bytesRead = 0;
    Q_UNUSED(bytesRead);	//TODO: to make gcc happy; in the future we will use the bytesRead to handle errors
    bool isDone = false;
    quint64 currentFrameLength = 0;

	while (!isDone)
	{
		switch (m_processingState)
		{
			case PS_READ_HEADER:
			{
                currentFrameLength = m_frame.size();
                if (pSocket->bytesAvailable() >= 2)
				{
					//FIN, RSV1-3, Opcode
					char header[2];
					bytesRead = pSocket->read(header, 2);
                    m_isFinalFragment = (header[0] & 0x80) != 0;
                    int rsv1 = (header[0] & 0x40);
                    int rsv2 = (header[0] & 0x20);
                    int rsv3 = (header[0] & 0x10);
                    if (rsv1 || rsv2 || rsv3)
                    {
                        clear();
                        Q_EMIT errorEncountered(WebSocketProtocol::CC_PROTOCOL_ERROR, "Rsv field is non-zero");
                        return;
                    }
                    WebSocketProtocol::OpCode opCode = static_cast<WebSocketProtocol::OpCode>(header[0] & 0x0F);
                    m_isControlFrame = (opCode & 0x08) == 0x08;
                    if (WebSocketProtocol::isOpCodeReserved(opCode))
                    {
                        clear();
                        qDebug() << "Invalid opcode";
                        Q_EMIT errorEncountered(WebSocketProtocol::CC_PROTOCOL_ERROR, "Used reserved opcode");
                        return;
                    }
                    if ((opCode == WebSocketProtocol::OC_CONTINUE) && !m_isFragmented)
                    {
                        clear();
                        qDebug() << "Continuation frame while not fragmented.";
                        Q_EMIT errorEncountered(WebSocketProtocol::CC_PROTOCOL_ERROR, "Received Continuation frame /*with FIN=true*/, while there is nothing to continue.");
                        return;
                    }
                    if (!m_isControlFrame && m_isFragmented && (opCode != WebSocketProtocol::OC_CONTINUE))
                    {
                        clear();
                        qDebug() << "Continuation frame with invalid opcode.";
                        Q_EMIT errorEncountered(WebSocketProtocol::CC_PROTOCOL_ERROR, "All data frames after the initial data frame must have opcode 0");
                        return;
                    }
                    if (opCode != WebSocketProtocol::OC_CONTINUE)
                    {
                        m_opCode = opCode;
                    }
                    if (!m_isControlFrame && (opCode != WebSocketProtocol::OC_CONTINUE))
                    {
                        m_isFragmented = !m_isFinalFragment;
                    }

					//Mask, PayloadLength
					m_hasMask = (header[1] & 0x80) != 0;
					quint8 length = (header[1] & 0x7F);

                    if (m_isControlFrame)
                    {
                        if (length > 125)
                        {
                            qDebug() << "Control frame larger than 125 bytes.";
                            Q_EMIT errorEncountered(WebSocketProtocol::CC_PROTOCOL_ERROR, "Controle frame is larger than 125 bytes");
                            return;
                        }

                        if (!m_isFinalFragment)
                        {
                            qDebug() << "Control frames cannot be fragmented.";
                            Q_EMIT errorEncountered(WebSocketProtocol::CC_PROTOCOL_ERROR, "Controle frames cannot be fragmented");
                            return;
                        }
                    }

					switch (length)
					{
						case 126:
						{
							m_processingState = PS_READ_PAYLOAD_LENGTH;
							break;
						}
						case 127:
						{
							m_processingState = PS_READ_BIG_PAYLOAD_LENGTH;
							break;
						}
						default:
						{
							m_payloadLength = length;
							m_processingState = m_hasMask ? PS_READ_MASK : PS_READ_PAYLOAD;
							break;
						}
					}
				}
				else
				{
					isDone = true;
				}
				break;
			}

			case PS_READ_PAYLOAD_LENGTH:
			{
                if (pSocket->bytesAvailable() >= 2)
				{
					uchar length[2];
					 //TODO: Handle return value
					bytesRead = pSocket->read(reinterpret_cast<char *>(length), 2);
					m_payloadLength = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(length));
					m_processingState = m_hasMask ? PS_READ_MASK : PS_READ_PAYLOAD;
				}
				else
				{
					isDone = true;
				}
				break;
			}

			case PS_READ_BIG_PAYLOAD_LENGTH:
			{
                if (pSocket->bytesAvailable() >= 8)
				{
					uchar length[8];
					//TODO: Handle return value
					bytesRead = pSocket->read(reinterpret_cast<char *>(length), 8);
					//Most significant bit must be set to 0 as per http://tools.ietf.org/html/rfc6455#section-5.2
					//TODO: Do we check for that?
					m_payloadLength = qFromBigEndian<quint64>(length) & ~(1ULL << 63);
					m_processingState = m_hasMask ? PS_READ_MASK : PS_READ_PAYLOAD;
				}
				else
				{
					isDone = true;
				}

				break;
			}

			case PS_READ_MASK:
			{
                if (pSocket->bytesAvailable() >= 4)
				{
					//TODO: Handle return value
					bytesRead = pSocket->read(reinterpret_cast<char *>(&m_mask), sizeof(m_mask));
					m_processingState = PS_READ_PAYLOAD;
				}
				else
				{
					isDone = true;
				}
				break;
			}

			case PS_READ_PAYLOAD:
			{
                // TODO: Handle large payloads
                //if (pSocket->bytesAvailable() >= static_cast<qint32>(m_payloadLength))
                qint64 bytesAvailable = pSocket->bytesAvailable();
                //qint64 bytesLeftToRead = qMin(m_payloadLength - (quint64)m_frame.size() - currentFrameLength, (quint64)bytesAvailable);
                //qint64 bytesLeftToRead = qMin(m_payloadLength, (quint64)bytesAvailable);
                //if (bytesLeftToRead > 0)
                if (bytesAvailable >= m_payloadLength)
				{
                    //QByteArray payload = pSocket->read(bytesLeftToRead);
                    QByteArray payload = pSocket->read(m_payloadLength);
					if (m_hasMask)
					{
						WebSocketProtocol::mask(&payload, m_mask);
					}
					m_frame.append(payload);
                }
                else if (m_payloadLength > 0)
                {
                    //qDebug() <<  "Waiting for ready read";
                    pSocket->waitForReadyRead(1000);
                    //qDebug() <<  "Waited for ready read" << m_frame.size();
                    //isDone = true;  //wait for more to be read
                    break;
                }
                if (m_frame.size() == (m_payloadLength + currentFrameLength))
                {
                    m_processingState = m_isFinalFragment ? PS_DISPATCH_RESULT : PS_READ_HEADER;
                }
                else
				{
                    isDone = true;  //more to read
				}
                /*if ((m_frame.size() % (64 * 1000)) == 0)
                {
                    qDebug() << "framesize=" << m_frame.size() << pSocket->bytesAvailable();
                }*/
				break;
			}

			case PS_DISPATCH_RESULT:
			{
                Q_EMIT frameReceived(m_opCode, m_frame, m_isFinalFragment);
				m_processingState = PS_READ_HEADER;
				m_frame.clear();
				break;
			}

			default:
			{
				//should not come here
				qDebug() << "DataProcessor::process: Found invalid state. This should not happen!";
				isDone = true;
				break;
			}
		}	//end switch
	}	//end while
}
#endif
void DataProcessor::clear()
{
	m_processingState = PS_READ_HEADER;
	m_isFinalFragment = false;
    m_isFragmented = false;
	m_opCode = WebSocketProtocol::OC_CLOSE;
	m_hasMask = false;
	m_mask = 0;
	m_frame.clear();
	m_payloadLength = 0;
}
