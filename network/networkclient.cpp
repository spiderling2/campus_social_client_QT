#include"networkclient.h"
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include"jsontool.h"
#include"../service/userservice.h"

NetworkClient::NetworkClient(QObject* parent) : QObject(parent) {
    socket = new QTcpSocket(this);
    connectToServer();
}

void NetworkClient::connectToServer() {
    socket->connectToHost("127.0.0.1", 12345);
    connect(socket, &QTcpSocket::readyRead, this,&NetworkClient::onReadyRead);
}

void NetworkClient::login(const QString& username, const QString& password) {
    QJsonObject obj;
    QJsonObject data;
    obj["type"]="login";
    data["username"]=username;
    data["password"]=password;
    obj["data"]=data;
    sendJson(obj);
}

void NetworkClient::registerUser(const QString& username, const QString& password) {
    QJsonObject obj;
    QJsonObject data;
    obj["type"]="register";
    data["username"]=username;
    data["password"]=password;
    obj["data"]=data;
     sendJson(obj);
    qDebug()<<"消息已发送";
}

void NetworkClient::logout() {
    QJsonObject obj=JsonTool::make_request("logout","success");
    sendJson(obj);


}

void NetworkClient::createEvent(const QString& eventName) {


    QJsonObject obj,data;
    data["username"]=UserService::instance().get_username();
    data["eventname"]=eventName;
    obj=JsonTool::make_request("create_event","success","",data);
    socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));

}

void NetworkClient::joinEvent(const QString& eventName) {
    QJsonObject obj {{"type","join_event"}, {"event", eventName}};
    socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void NetworkClient::sendMessage(const QString& eventName, const QString& message) {
    QJsonObject obj {{"type","send"}, {"event", eventName}, {"content", message}};
    socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void NetworkClient::sendFile(const QString& eventName, const QString& filepath) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Failed to open file");
        return;
    }
    QByteArray data = file.readAll();
    // 简单协议: type=file, event=..., content=base64
    QJsonObject obj {{"type","file"}, {"event", eventName}, {"content", QString(data.toBase64())}};
    socket->write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void NetworkClient::onReadyRead()
{
    buffer.append(socket->readAll());

    while (true)
    {
        if (expectedSize == 0)
        {
            if (buffer.size() < 4)
                break; // 长度头还没收齐
            // 取前 4 字节作为消息长度
            expectedSize = qFromBigEndian<qint32>(reinterpret_cast<const uchar*>(buffer.constData()));
            buffer.remove(0, 4);
        }

        if (buffer.size() < expectedSize)
            break; // 消息体还没收齐

        QByteArray jsonData = buffer.left(expectedSize);
        buffer.remove(0, expectedSize);
        expectedSize = 0;

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (!doc.isNull() && doc.isObject())
        {
            //处理消息
            QString msg=handle_message(doc.object());
            emit messageReceived(msg);
        }
    }
}

QString NetworkClient::handle_message(const QJsonObject& msg)
{
    // 转成 JSON 文本
    QJsonDocument doc(msg);
    QString str = doc.toJson(QJsonDocument::Compact); // 紧凑格式
    qDebug() << str;

    QString type=msg.value("type").toString();
    QString status=msg.value("status").toString();
    QString message=msg.value("message").toString();
    QJsonObject data=msg.value("data").toObject();

    if(status=="success")
    {
        if(type=="register_response")
        {
            return "[register]"+message;

        }else if(type=="login_response")
        {
            UserService::instance().set_username(data.value("username").toString());
            emit loginSuccess();
            return "[login]"+message;

        }else if(type=="logout_response")
        {
            emit logoutSuccess();
            return "[logout]"+message;

        }
    }
    else if(status=="error")
    {
        return "[error]"+message;

    }
}

#include <QtEndian>  // 用于 qToBigEndian

// 私有方法，发送长度 + JSON 数据
void NetworkClient::sendJson(const QJsonObject& obj)
{
    QByteArray jsonData = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    qint32 len = jsonData.size();

    QByteArray header;
    header.resize(4);
    qToBigEndian(len, reinterpret_cast<uchar*>(header.data()));

    socket->write(header);
    socket->write(jsonData);
    socket->flush();  // 可选：强制立即发送
}
