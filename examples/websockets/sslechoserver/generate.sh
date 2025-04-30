#!/usr/bin/sh
# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

openssl req -x509 -nodes -newkey rsa:4096 -keyout localhost.key -out localhost.cert -sha512 -days 365 -subj "/C=NO/ST=Oslo/L=Oslo/O=The Qt Project/OU=WebSockets/CN=localhost"
