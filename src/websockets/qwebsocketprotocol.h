/****************************************************************************
**
** Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QWEBSOCKETPROTOCOL_H
#define QWEBSOCKETPROTOCOL_H

#include <QtCore/qglobal.h>
#include "QtWebSockets/qwebsockets_global.h"

QT_BEGIN_NAMESPACE

class QString;

namespace QWebSocketProtocol
{
enum Version
{
    VersionUnknown = -1,
    Version0 = 0,
    //hybi-01, hybi-02 and hybi-03 not supported
    Version4 = 4,
    Version5 = 5,
    Version6 = 6,
    Version7 = 7,
    Version8 = 8,
    Version13 = 13,
    VersionLatest = Version13
};

enum CloseCode
{
    CloseCodeNormal                 = 1000,
    CloseCodeGoingAway              = 1001,
    CloseCodeProtocolError          = 1002,
    CloseCodeDatatypeNotSupported   = 1003,
    CloseCodeReserved1004           = 1004,
    CloseCodeMissingStatusCode      = 1005,
    CloseCodeAbnormalDisconnection  = 1006,
    CloseCodeWrongDatatype          = 1007,
    CloseCodePolicyViolated         = 1008,
    CloseCodeTooMuchData            = 1009,
    CloseCodeMissingExtension       = 1010,
    CloseCodeBadOperation           = 1011,
    CloseCodeTlsHandshakeFailed     = 1015
};

}	//end namespace QWebSocketProtocol

QT_END_NAMESPACE

#endif // QWEBSOCKETPROTOCOL_H
