#ifndef WEBSOCKETPROTOCOL_H
#define WEBSOCKETPROTOCOL_H

class QString;
class QByteArray;
#include <qglobal.h>

namespace WebSocketProtocol
{
	/**
	 * @brief The Version enum
	 * For an overview of the differences between the different protocols, see
	 * http://code.google.com/p/pywebsocket/wiki/WebSocketProtocolSpec
	 */
	enum Version
	{
		V_Unknow = -1,
		/*
		 * hixie76: http://tools.ietf.org/html/draft-hixie-thewebsocketprotocol-76
		 * hybi-00: http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-00
		 * Works with key1, key2 and a key in the payload
		 * Attribute: Sec-WebSocket-Draft value 0
		 **/
		V_0 = 0,
		//hybi-01, hybi-02 and hybi-03 not supported
		/*
		 * hybi-04: http://tools.ietf.org/id/draft-ietf-hybi-thewebsocketprotocol-04.txt
		 * Changed handshake: key1, key2, key3 ==> Sec-WebSocket-Key, Sec-WebSocket-Nonce, Sec-WebSocket-Accept
		 * Sec-WebSocket-Draft renamed to Sec-WebSocket-Version
		 * Sec-WebSocket-Version = 4
		 **/
		V_4 = 4,
		/*
		 * hybi-05: http://tools.ietf.org/id/draft-ietf-hybi-thewebsocketprotocol-05.txt
		 * Sec-WebSocket-Version = 5
		 * Removed Sec-WebSocket-Nonce
		 * Added Sec-WebSocket-Accept
		 */
		V_5 = 5,
		/*
		 * Sec-WebSocket-Version = 6
		 */
		V_6 = 6,
		/*
		 * hybi-07: http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-07
		 * Sec-WebSocket-Version = 7
		 */
		V_7 = 7,
		/*
		 * hybi-8, hybi-9, hybi-10, hybi-11 and hybi-12
		 * Status codes 1005 and 1006 are added and all codes are now unsigned
		 * Internal error results in 1006
		 */
		V_8 = 8,
		/*
		 * hybi-13, hybi14, hybi-15, hybi-16, hybi-17 and RFC 6455
		 * Sec-WebSocket-Version = 13
		 * Status code 1004 is now reserved
		 * Added 1008, 1009 and 1010
		 * Must support TLS
		 * Clarify multiple version support
		 */
		V_13 = 13,
		V_LATEST = V_13
	};

	Version versionFromString(const QString &versionString);

	enum CloseCode
	{
		CC_NORMAL					= 1000,		//normal closure
		CC_GOING_AWAY				= 1001,		//going away
		CC_PROTOCOL_ERROR			= 1002,		//protocol error
		CC_DATATYPE_NOT_SUPPORTED	= 1003,		//unsupported data
		CC_RESERVED_1004			= 1004,		//---Reserved----
		CC_MISSING_STATUS_CODE		= 1005,		//no status received
		CC_ABNORMAL_DISCONNECTION	= 1006,		//abnormal closure
		CC_WRONG_DATATYPE			= 1007,		//Invalid frame payload data
		CC_POLICY_VIOLATED			= 1008,		//Policy violation
		CC_TOO_MUCH_DATA			= 1009,		//Message too big
		CC_MISSING_EXTENSION		= 1010,		//Mandatory extension missing
		CC_BAD_OPERATION			= 1011,		//Internal server error
        CC_TLS_HANDSHAKE_FAILED		= 1015		//TLS handshake failed
	};

	enum OpCode
	{
		OC_CONTINUE		= 0x0,
		OC_TEXT			= 0x1,
		OC_BINARY		= 0x2,
		OC_RESERVED_3	= 0x3,
		OC_RESERVED_4	= 0x4,
		OC_RESERVED_5	= 0x5,
		OC_RESERVED_6	= 0x6,
		OC_RESERVED_7	= 0x7,
		OC_CLOSE		= 0x8,
		OC_PING			= 0x9,
		OC_PONG			= 0xA,
		OC_RESERVED_B	= 0xB,
		OC_RESERVED_V	= 0xC,
		OC_RESERVED_D	= 0xD,
		OC_RESERVED_E	= 0xE,
        OC_RESERVED_F	= 0xF
	};


    inline bool isOpCodeReserved(OpCode code)
    {
        return ((code > OC_BINARY) && (code < OC_CLOSE)) || (code > OC_PONG);
    }
    inline bool isCloseCodeValid(int closeCode)
    {
        return  (closeCode > 999) && (closeCode < 5000) &&
                (closeCode != CC_RESERVED_1004) &&          //see RFC6455 7.4.1
                (closeCode != CC_MISSING_STATUS_CODE) &&
                (closeCode != CC_ABNORMAL_DISCONNECTION) &&
                ((closeCode >= 3000) || (closeCode < 1012));
    }

    void mask(QByteArray *payload, quint32 maskingKey);
	void mask(char *payload, quint64 size, quint32 maskingKey);

}	//end namespace WebSocketProtocol

#endif // WEBSOCKETPROTOCOL_H
