#include "websocketprotocol.h"
#include <QString>
#include <QSet>
#include <QtEndian>

namespace WebSocketProtocol
{
	Version versionFromString(const QString &versionString)
	{
		bool ok = false;
		Version version = V_Unknow;
		int ver = versionString.toInt(&ok);
		QSet<Version> supportedVersions;
		supportedVersions << V_0 << V_4 << V_5 << V_6 << V_7 << V_8 << V_13;
		if (ok)
		{
			if (supportedVersions.contains(static_cast<Version>(ver)))
			{
				version = static_cast<Version>(ver);
			}
		}
		return version;
	}

	void mask(QByteArray *payload, quint32 maskingKey)
	{
		quint32 *payloadData = reinterpret_cast<quint32 *>(payload->data());
		quint32 numIterations = static_cast<quint32>(payload->size()) / sizeof(quint32);
		quint32 remainder = static_cast<quint32>(payload->size()) % sizeof(quint32);
		quint32 i;
		for (i = 0; i < numIterations; ++i)
		{
			*(payloadData + i) ^= maskingKey;
		}
		if (remainder)
		{
			const quint32 offset = i * static_cast<quint32>(sizeof(quint32));
			char *payloadBytes = payload->data();
			uchar *mask = reinterpret_cast<uchar *>(&maskingKey);
			for (quint32 i = 0; i < remainder; ++i)
			{
				*(payloadBytes + offset + i) ^= static_cast<char>(mask[(i + offset) % 4]);
			}
		}
	}

	void mask(char *payload, quint64 size, quint32 maskingKey)
	{
		quint32 *payloadData = reinterpret_cast<quint32 *>(payload);
		quint32 numIterations = static_cast<quint32>(size / sizeof(quint32));
		quint32 remainder = size % sizeof(quint32);
		quint32 i;
		for (i = 0; i < numIterations; ++i)
		{
			*(payloadData + i) ^= maskingKey;
		}
		if (remainder)
		{
			const quint32 offset = i * static_cast<quint32>(sizeof(quint32));
			uchar *mask = reinterpret_cast<uchar *>(&maskingKey);
			for (quint32 i = 0; i < remainder; ++i)
			{
				*(payload + offset + i) ^= static_cast<char>(mask[(i + offset) % 4]);
			}
		}
	}
}	//end namespace WebSocketProtocol
