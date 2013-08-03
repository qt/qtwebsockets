#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QByteArray>
#include "websocketprotocol.h"

class QTcpSocket;

class DataProcessor: public QObject
{
	Q_OBJECT
public:
	explicit DataProcessor(QObject *parent = 0);
	virtual ~DataProcessor();

Q_SIGNALS:
	void frameReceived(WebSocketProtocol::OpCode opCode, QByteArray frame, bool lastFrame);
    void errorEncountered(WebSocketProtocol::CloseCode code, QString description);

public Q_SLOTS:
	void process(QTcpSocket *pSocket);
	void clear();

private:
	Q_DISABLE_COPY(DataProcessor)
	enum
	{
		PS_READ_HEADER,
		PS_READ_PAYLOAD_LENGTH,
		PS_READ_BIG_PAYLOAD_LENGTH,
		PS_READ_MASK,
		PS_READ_PAYLOAD,
		PS_DISPATCH_RESULT
	} m_processingState;

	bool m_isFinalFragment;
    bool m_isFragmented;
	WebSocketProtocol::OpCode m_opCode;
    bool m_isControlFrame;
	bool m_hasMask;
	quint32 m_mask;
	QByteArray m_frame;
	quint64 m_payloadLength;
};

#endif // DATAPROCESSOR_H
