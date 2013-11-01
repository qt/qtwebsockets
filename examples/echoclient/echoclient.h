#ifndef ECHOCLIENT_H
#define ECHOCLIENT_H

#include <QObject>
#include <QtWebSockets/QWebSocket>

class EchoClient : public QObject
{
    Q_OBJECT
public:
    explicit EchoClient(const QUrl &url, QObject *parent = Q_NULLPTR);

Q_SIGNALS:

public Q_SLOTS:

private Q_SLOTS:
    void onConnected();
    void onTextMessageReceived(QString message);

private:
    QWebSocket m_webSocket;
};

#endif // ECHOCLIENT_H
