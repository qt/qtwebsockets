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
****************************************************************************/

#ifndef QMASKGENERATOR_H
#define QMASKGENERATOR_H

#include <QtCore/QObject>
#include "QtWebSockets/qwebsockets_global.h"

QT_BEGIN_NAMESPACE

class Q_WEBSOCKETS_EXPORT QMaskGenerator : public QObject
{
    Q_DISABLE_COPY(QMaskGenerator)

public:
    explicit QMaskGenerator(QObject *parent = nullptr);
    ~QMaskGenerator() override;

    virtual bool seed() = 0;
    virtual quint32 nextMask() = 0;
};

QT_END_NAMESPACE

#endif // QMASKGENERATOR_H
