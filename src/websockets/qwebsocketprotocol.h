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

Version versionFromString(const QString &versionString);

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
