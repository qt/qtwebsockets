// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QSSLSERVER_P_H
#define QSSLSERVER_P_H

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

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QSslError>
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslPreSharedKeyAuthenticator>
#include <QtCore/QList>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QSslSocket;

class QSslServer : public QTcpServer
{
    Q_OBJECT
    Q_DISABLE_COPY(QSslServer)

public:
    explicit QSslServer(QObject *parent = nullptr);
    ~QSslServer() override;

    void setSslConfiguration(const QSslConfiguration &sslConfiguration);
    QSslConfiguration sslConfiguration() const;

Q_SIGNALS:
    void sslErrors(const QList<QSslError> &errors);
    void peerVerifyError(const QSslError &error);
    void newEncryptedConnection();
    void preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *authenticator);
    void alertSent(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description);
    void alertReceived(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description);
    void handshakeInterruptedOnError(const QSslError &error);
    void startedEncryptionHandshake(QSslSocket *socket);

protected:
    void incomingConnection(qintptr socket) override;

private slots:
    void socketEncrypted();

private:
    QSslConfiguration m_sslConfiguration;
};

QT_END_NAMESPACE

#endif // QSSLSERVER_P_H
