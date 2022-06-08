// Copyright (C) 2013 Kurt Pattyn <pattyn.kurt@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GFDL-1.3-no-invariants-only
//! [6]
QList<QSslCertificate> cert = QSslCertificate::fromPath(QLatin1String("server-certificate.pem"));
QSslError error(QSslError::SelfSignedCertificate, cert.at(0));
QList<QSslError> expectedSslErrors;
expectedSslErrors.append(error);

QWebSocket socket;
socket.ignoreSslErrors(expectedSslErrors);
socket.open(QUrl(QStringLiteral("wss://myserver.at.home")));
//! [6]
