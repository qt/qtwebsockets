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

#include "qsslserver_p.h"

#include <QtNetwork/QSslSocket>
#include <QtNetwork/QSslCipher>

QT_BEGIN_NAMESPACE

QSslServer::QSslServer(QObject *parent) :
    QTcpServer(parent),
    m_sslConfiguration(QSslConfiguration::defaultConfiguration())
{
}

QSslServer::~QSslServer()
{
}

void QSslServer::setSslConfiguration(const QSslConfiguration &sslConfiguration)
{
    m_sslConfiguration = sslConfiguration;
}

QSslConfiguration QSslServer::sslConfiguration() const
{
    return m_sslConfiguration;
}

void QSslServer::incomingConnection(qintptr socket)
{
    QSslSocket *pSslSocket = new QSslSocket();

    if (Q_LIKELY(pSslSocket)) {
        pSslSocket->setSslConfiguration(m_sslConfiguration);

        if (Q_LIKELY(pSslSocket->setSocketDescriptor(socket))) {
            connect(pSslSocket, SIGNAL(peerVerifyError(QSslError)), this, SIGNAL(peerVerifyError(QSslError)));
            connect(pSslSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SIGNAL(sslErrors(QList<QSslError>)));
            connect(pSslSocket, SIGNAL(encrypted()), this, SIGNAL(newEncryptedConnection()));

            addPendingConnection(pSslSocket);

            pSslSocket->startServerEncryption();
        } else {
           delete pSslSocket;
        }
    }
}

QT_END_NAMESPACE
