#ifndef HANDSHAKERESPONSE_P_H
#define HANDSHAKERESPONSE_P_H

#include <QObject>
#include "qwebsocketprotocol.h"

class HandshakeRequest;
class QString;
class QTextStream;

class HandshakeResponse:public QObject
{
	Q_OBJECT
public:
	HandshakeResponse(const HandshakeRequest &request,
					  const QString &serverName,
					  bool isOriginAllowed,
					  const QList<QWebSocketProtocol::Version> &supportedVersions,
					  const QList<QString> &supportedProtocols,
					  const QList<QString> &supportedExtensions);

	virtual ~HandshakeResponse();

	bool isValid() const;
	bool canUpgrade() const;
	QString getAcceptedProtocol() const;
	QString getAcceptedExtension() const;
	QWebSocketProtocol::Version getAcceptedVersion() const;

public Q_SLOTS:

Q_SIGNALS:

private:
	Q_DISABLE_COPY(HandshakeResponse)
	bool m_isValid;
	bool m_canUpgrade;
	QString m_response;
	QString m_acceptedProtocol;
	QString m_acceptedExtension;
	QWebSocketProtocol::Version m_acceptedVersion;

	QString calculateAcceptKey(const QString &key) const;
	QString getHandshakeResponse(const HandshakeRequest &request,
								 const QString &serverName,
								 bool isOriginAllowed,
								 const QList<QWebSocketProtocol::Version> &supportedVersions,
								 const QList<QString> &supportedProtocols,
								 const QList<QString> &supportedExtensions);

	QTextStream &writeToStream(QTextStream &textStream) const;
	friend QTextStream &operator <<(QTextStream &stream, const HandshakeResponse &response);
};

#endif // HANDSHAKERESPONSE_P_H
