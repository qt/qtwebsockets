/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
    V_Unknow = -1,
    V_0 = 0,
    //hybi-01, hybi-02 and hybi-03 not supported
    V_4 = 4,
    V_5 = 5,
    V_6 = 6,
    V_7 = 7,
    V_8 = 8,
    V_13 = 13,
    V_LATEST = V_13
};

Version versionFromString(const QString &versionString);

enum CloseCode
{
    CC_NORMAL					= 1000,
    CC_GOING_AWAY				= 1001,
    CC_PROTOCOL_ERROR			= 1002,
    CC_DATATYPE_NOT_SUPPORTED	= 1003,
    CC_RESERVED_1004			= 1004,
    CC_MISSING_STATUS_CODE		= 1005,
    CC_ABNORMAL_DISCONNECTION	= 1006,
    CC_WRONG_DATATYPE			= 1007,
    CC_POLICY_VIOLATED			= 1008,
    CC_TOO_MUCH_DATA			= 1009,
    CC_MISSING_EXTENSION		= 1010,
    CC_BAD_OPERATION			= 1011,
    CC_TLS_HANDSHAKE_FAILED		= 1015
};

inline Version currentVersion() { return V_LATEST; }

}	//end namespace QWebSocketProtocol

QT_END_NAMESPACE

#endif // QWEBSOCKETPROTOCOL_H
