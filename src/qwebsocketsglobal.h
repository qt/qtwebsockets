#ifndef QWEBSOCKETSGLOBAL_H
#define QWEBSOCKETSGLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#  if defined(QT_BUILD_SERIALPORT_LIB)
#    define Q_WEBSOCKETS_EXPORT Q_DECL_EXPORT
#  else
#    define Q_WEBSOCKETS_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define Q_WEBSOCKETS_EXPORT
#endif

// The macro has been available only since Qt 5.0
#ifndef Q_DECL_OVERRIDE
#define Q_DECL_OVERRIDE
#endif

QT_END_NAMESPACE
#endif // QWEBSOCKETSGLOBAL_H
