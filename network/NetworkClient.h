#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include<QString>

class NetworkClient : public QObject {
    Q_OBJECT
public:
    explicit NetworkClient(QObject* parent = nullptr);

    void login(const QString& username, const QString& password);
    void registerUser(const QString& username, const QString& password);
    void logout();
    void createEvent(const QString& eventName);
    void joinEvent(const QString& eventName);
    void sendMessage(const QString& eventName, const QString& message);
    void sendFile(const QString& eventName, const QString& filepath);
    void onReadyRead();
    QString handle_message(const QJsonObject& doc);
    void sendJson(const QJsonObject& obj);

signals:
    void messageReceived(const QString& msg);
    void errorOccurred(const QString& error);

    void loginSuccess();
    void logoutSuccess();

private:
    QTcpSocket* socket;
    QByteArray buffer;      // 累积收到的数据
    qint32 expectedSize;    // 当前消息长度

    void connectToServer();
};
