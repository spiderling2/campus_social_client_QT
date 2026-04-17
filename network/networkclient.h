#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include<QString>
#include <QTimer>
struct ReceivingFile {
    QString username;
    QString eventName;
    QString filename;      // 文件名
    qint64 filesize = 0;   // 文件总大小
    qint64 receivedBytes = 0;  // 已接收字节数
    QByteArray data;       // 缓存文件数据
    bool mode = false;     // 是否正在接收文件
};
class NetworkClient : public QObject {
    Q_OBJECT
public:
    explicit NetworkClient(QObject* parent = nullptr);

    void login(const QString& username, const QString& password);
    void registerUser(const QString& username, const QString& password);
    void logout(const QString& username);
    void createEvent(const QString& username,const QString& eventName);
    void joinEvent(const QString userName,const QString& eventName);
    //void sendMessage(const QString& userName,const QString& eventName, const QString& message);
    void sendMessageOrFile(const QString& eventName, const QVariant& content);
    void sendFile(const QString& eventName, const QString& filepath);
    void onReadyRead();
    QVariantMap handle_message(const QJsonObject& doc);
    void sendJson(const QJsonObject& obj);
    void set_current_event(const QString&eventName);
    void disconnectFromServer();
    void getevents(const QString& username);





signals:
    void messageReceived(const QVariantMap& msg);
    void errorOccurred(const QString& error);

    void loginSuccess();
    void logoutSuccess();
    void getEventsSuccess(const QVariantList& events);
    void getEventsRequested(const QString& userName);  // 新增：刷新事件信号

private slots:
    void onGetEventsRequested(const QString& userName);  // 定时刷新槽函数


private:
    QTcpSocket* socket;
    QByteArray buffer;      // 累积收到的数据
    qint32 expectedSize;    // 当前消息长度

    QString current_event;//当前选择事件
    QString current_userName;
    ReceivingFile receivingFile;

    QString saveDir;

    QTimer* getEventTimer;  // 定时器





    void connectToServer();
};
