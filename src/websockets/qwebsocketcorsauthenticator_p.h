#ifndef QWEBSOCKETCORSAUTHENTICATOR_P_H
#define QWEBSOCKETCORSAUTHENTICATOR_P_H

#include <qglobal.h>    //for QT_BEGIN_NAMESPACE
#include <QString>

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
QT_BEGIN_NAMESPACE

class QWebSocketCorsAuthenticatorPrivate
{
public:
    QWebSocketCorsAuthenticatorPrivate(const QString &origin, bool allowed);
    ~QWebSocketCorsAuthenticatorPrivate();

    QString m_origin;
    bool m_isAllowed;
};

#endif // QWEBSOCKETCORSAUTHENTICATOR_P_H
