#ifndef DATAPROCESSOR_P_H
#define DATAPROCESSOR_P_H

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

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QTextCodec>
#include "qwebsocketprotocol.h"

QT_BEGIN_NAMESPACE

class QTcpSocket;

/**
 * @internal
 * @brief The DataProcessor class
 */
class DataProcessor: public QObject
{
	Q_OBJECT
public:
	explicit DataProcessor(QObject *parent = 0);
	virtual ~DataProcessor();

Q_SIGNALS:
	void controlFrameReceived(QWebSocketProtocol::OpCode opCode, QByteArray frame);
	void textFrameReceived(QString frame, bool lastFrame);
	void binaryFrameReceived(QByteArray frame, bool lastFrame);
	void textMessageReceived(QString message);
	void binaryMessageReceived(QByteArray message);
	void errorEncountered(QWebSocketProtocol::CloseCode code, QString description);

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
};

QT_END_NAMESPACE

#endif // DATAPROCESSOR_P_H
