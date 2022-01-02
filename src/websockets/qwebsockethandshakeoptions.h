/****************************************************************************
**
** Copyright (C) 2022 Menlo Systems GmbH, author Arno Rehn <a.rehn@menlosystems.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#ifndef QWEBSOCKETHANDSHAKEOPTIONS_H
#define QWEBSOCKETHANDSHAKEOPTIONS_H

#include <QtCore/QSharedDataPointer>
#include <QtCore/QStringList>

#include "QtWebSockets/qwebsockets_global.h"

QT_BEGIN_NAMESPACE

class QWebSocketHandshakeOptionsPrivate;

class Q_WEBSOCKETS_EXPORT QWebSocketHandshakeOptions
{
public:
    QWebSocketHandshakeOptions();
    QWebSocketHandshakeOptions(const QWebSocketHandshakeOptions &other);
    QWebSocketHandshakeOptions(QWebSocketHandshakeOptions &&other);
    ~QWebSocketHandshakeOptions();

    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QWebSocketHandshakeOptions)
    QWebSocketHandshakeOptions &operator=(const QWebSocketHandshakeOptions &other);

    void swap(QWebSocketHandshakeOptions &other) noexcept { qSwap(d, other.d); }

    QStringList subprotocols() const;
    void setSubprotocols(const QStringList &protocols);

private:
    bool equals(const QWebSocketHandshakeOptions &other) const;

    friend bool operator==(const QWebSocketHandshakeOptions &lhs,
                           const QWebSocketHandshakeOptions &rhs) { return lhs.equals(rhs); }
    friend bool operator!=(const QWebSocketHandshakeOptions &lhs,
                           const QWebSocketHandshakeOptions &rhs) { return !lhs.equals(rhs); }

    QSharedDataPointer<QWebSocketHandshakeOptionsPrivate> d;
    friend class QWebSocketHandshakeOptionsPrivate;
};

QT_END_NAMESPACE

#endif // QWEBSOCKETHANDSHAKEOPTIONS_H
