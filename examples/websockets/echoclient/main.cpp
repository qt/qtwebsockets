// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include "echoclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("QtWebSockets example: echoclient");
    parser.addHelpOption();

    QCommandLineOption dbgOption(QStringList() << "d" << "debug",
            QCoreApplication::translate("main", "Debug output [default: off]."));
    parser.addOption(dbgOption);
    parser.process(a);
    bool debug = parser.isSet(dbgOption);

    EchoClient client(QUrl(QStringLiteral("ws://localhost:1234")), debug);
    QObject::connect(&client, &EchoClient::closed, &a, &QCoreApplication::quit);

    return a.exec();
}
