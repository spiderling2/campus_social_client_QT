#include"networkclient.h"
#include <QJsonDocument>
#include <QFile>
#include <QDebug>
#include"jsontool.h"
#include"../service/userservice.h"
#include <QtConcurrent/QtConcurrent>

NetworkClient::NetworkClient(QObject* parent) : QObject(parent) {
    QDir dir(QDir::currentPath());
    dir.mkdir("received_files");  // 确保目录存在
    saveDir = dir.absoluteFilePath("received_files");
    QDir().mkpath(saveDir); // 如果不存在就创建

    socket = new QTcpSocket(this);
    getEventTimer=new QTimer(this);
    getEventTimer->setInterval(30000);
    connectToServer();
}

void NetworkClient::connectToServer() {
    socket->connectToHost("127.0.0.1", 12345);
    connect(socket, &QTcpSocket::readyRead, this,&NetworkClient::onReadyRead);
    connect(getEventTimer,&QTimer::timeout,[this](){
        emit getEventsRequested(current_userName);
    });
    connect(this,&NetworkClient::getEventsRequested,this,&NetworkClient::onGetEventsRequested);
}

void NetworkClient::onGetEventsRequested(const QString& userName)
{
    QJsonObject obj;
    QJsonObject data;
    obj["type"]="get_events";
    data["username"]=userName;
    obj["data"]=data;
    sendJson(obj);
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

void NetworkClient::logout(const QString& username) {



    QJsonObject obj;
    QJsonObject data;
    obj["type"]="logout";
    data["username"]=username;
    obj["data"]=data;
    sendJson(obj);


}

void NetworkClient::createEvent(const QString& username,const QString& eventName) {


    QJsonObject obj;
    QJsonObject data;
    obj["type"]="create_event";
    data["username"]=username;
    data["eventname"]=eventName;
    obj["data"]=data;
    sendJson(obj);

}

void NetworkClient::joinEvent(const QString userName,const QString& eventName) {

    QJsonObject obj;
    QJsonObject data;
    obj["type"]="join_event";
    data["username"]=userName;
    data["eventname"]=eventName;
    obj["data"]=data;
    sendJson(obj);
}

void NetworkClient::sendFile(const QString& eventName, const QString& filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Failed to open file: " + filepath);
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    // 1️⃣ 先发送文件头 JSON (msg_type = 1)
    QJsonObject headerObj;
    headerObj["type"] = "send_file";
    QJsonObject data;
    data["username"] = UserService::instance().get_username();
    data["eventname"] = eventName;
    data["filename"] = QFileInfo(filepath).fileName();
    data["filesize"] = static_cast<qint64>(fileData.size());
    headerObj["data"] = data;

    QByteArray headerJson = QJsonDocument(headerObj).toJson(QJsonDocument::Compact);
    quint32 payload_len = static_cast<quint32>(headerJson.size());
    quint32 total_len = 1 + 4 + payload_len; // type + payload_len + payload

    QByteArray packet;
    packet.resize(4 + total_len);
    qToBigEndian(total_len, reinterpret_cast<uchar*>(packet.data())); // 总长度
    packet[4] = 1; // 文件头类型
    qToBigEndian(payload_len, reinterpret_cast<uchar*>(packet.data() + 5));
    memcpy(packet.data() + 9, headerJson.constData(), payload_len);

    socket->write(packet);
    socket->flush();

    // 2️⃣ 再发送文件内容原始字节流
    socket->write(fileData);
    socket->flush();
    qDebug() << "sendFile:" << filepath << "size:" << fileData.size();
}



#include <QtEndian>

void NetworkClient::onReadyRead()
{
    buffer.append(socket->readAll());

    while (true) {
        // =========================
        // 1. 文件接收模式（独立流）
        // =========================
        if (receivingFile.mode) {
            if (buffer.isEmpty())
                break;

            qint64 remaining = receivingFile.filesize - receivingFile.receivedBytes;
            if (remaining <= 0) {
                receivingFile.mode = false;
                break;
            }

            QByteArray chunk = buffer.left(qMin<qint64>(remaining, buffer.size()));
            buffer.remove(0, chunk.size());

            receivingFile.data.append(chunk);
            receivingFile.receivedBytes += chunk.size();

            if (receivingFile.receivedBytes >= receivingFile.filesize) {
                QString savePath = QDir(saveDir).filePath(receivingFile.filename);

                QFile file(savePath);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(receivingFile.data);
                    file.close();
                    qDebug() << "file received successfully:" << savePath;
                    emit fileReceived(receivingFile.username, receivingFile.filename, savePath);
                } else {
                    qDebug() << "error writing file:" << savePath;
                }

                // 重置状态
                receivingFile.mode = false;
                receivingFile.data.clear();
                receivingFile.filesize = 0;
                receivingFile.receivedBytes = 0;
            }
            continue;
        }

        // =========================
        // 2. 检查总长度
        // =========================
        if (buffer.size() < 4)
            break;

        quint32 total_be = 0;
        memcpy(&total_be, buffer.constData(), 4);
        quint32 total_len = qFromBigEndian(total_be);

        if (total_len < 5 + 4) { // type(1) + payload_len(4)
            buffer.remove(0, 1); // 重同步
            continue;
        }

        if (buffer.size() < total_len + 4)
            break;

        buffer.remove(0, 4); // 移除 total

        // =========================
        // 3. 读取类型和payload长度
        // =========================
        uint8_t msg_type = static_cast<uint8_t>(buffer[0]);
        buffer.remove(0, 1);

        quint32 payload_len_be = 0;
        memcpy(&payload_len_be, buffer.constData(), 4);
        quint32 payload_len = qFromBigEndian(payload_len_be);
        buffer.remove(0, 4);

        if (buffer.size() < payload_len) {
            buffer.prepend(QByteArray(1, msg_type)); // 回滚
            QByteArray pl_len_bytes(reinterpret_cast<char*>(&payload_len_be), 4);
            buffer.prepend(pl_len_bytes);
            buffer.prepend(QByteArray(4, 0)); // 回滚total? 可选
            break;
        }

        QByteArray payload = buffer.left(payload_len);
        buffer.remove(0, payload_len);

        // =========================
        // 4. 根据类型处理
        // =========================
        if (msg_type == 0) {
            // JSON 消息
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(payload, &err);
            if (err.error != QJsonParseError::NoError) {
                qDebug() << "JSON parse error:" << err.errorString();
                continue;
            }

            if (!doc.isObject())
                continue;

            QJsonObject obj = doc.object();
            QJsonObject data = obj.value("data").toObject();

            // 普通消息
            QtConcurrent::run([this, obj]() {
                QVariantMap res = handle_message(obj);
                if (!res.isEmpty())
                    emit messageReceived(res);
            });

        } else if (msg_type == 1) {
            // 文件头 JSON
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(payload, &err);
            if (err.error != QJsonParseError::NoError || !doc.isObject()) {
                qDebug() << "file header parse error";
                continue;
            }
            QJsonObject data = doc.object().value("data").toObject();
            receivingFile.username=data.value("username").toString();
            receivingFile.filename = data.value("filename").toString();
            receivingFile.filesize = data.value("filesize").toVariant().toLongLong();
            receivingFile.receivedBytes = 0;
            receivingFile.data.clear();
            receivingFile.mode = true;

            qDebug() << "开始接收文件:" << receivingFile.filename
                     << "大小:" << receivingFile.filesize;
        } else {
            qDebug() << "未知消息类型:" << msg_type;
        }
    }
}
QVariantMap NetworkClient::handle_message(const QJsonObject& msg)
{
    // 转成 JSON 文本
    QJsonDocument doc(msg);
    QString str = doc.toJson(QJsonDocument::Compact); // 紧凑格式
    qDebug() << str;

    QString type=msg.value("type").toString();
    QString status=msg.value("status").toString();
    QString message=msg.value("message").toString();
    QJsonObject data=msg.value("data").toObject();

    QVariantMap result;


    if(status=="success")
    {
        if(type=="register_response")
        {
            result["username"]=data.value("username").toString();
            result["content"]="[register]"+message;

            return result;
        }else if(type=="login_response")
        {
            UserService::instance().set_username(data.value("username").toString());
            current_userName=UserService::instance().get_username();
            emit loginSuccess();
            result["username"]=UserService::instance().get_username();
            result["content"] ="[login]"+message;
            // 启动定时器获取事件列表
            if (!getEventTimer->isActive()) {
                getEventTimer->start();
                qDebug() << "Started event refresh timer";
            }
            emit getEventsRequested(current_userName);
            return result;

        }else if(type=="logout_response")
        {
            emit logoutSuccess();
            result["username"]=UserService::instance().get_username();
            result["content"] ="[logout]"+message;
            // 停止定时器
            if (getEventTimer->isActive()) {
                getEventTimer->stop();
                qDebug() << "Stopped event refresh timer";
            }
            current_event="";
            current_userName="";
            return result;

        }
        else if(type=="create_event_response")
        {
            result["username"]=UserService::instance().get_username();
            result["content"]="create_event_response"+message;
            return result;
        }
        else if(type=="join_event_response")
        {
            result["username"]=UserService::instance().get_username();
            result["message"]= "join_event_response"+message;
            return result;
        }
        else if(type=="get_events_response")
        {
            QVariantList events;
            QJsonArray contentArray=data.value("content").toArray();
            // 转换为 QVariantList
            for (const auto& item : contentArray) {
                events.append(item.toVariant());
            }

            emit getEventsSuccess(events);
            qDebug()<<"get event successfully";
            return result;
        }
        else if(type=="message_broadcast")
        {
            if(data.value("eventname").toString()==current_event)
                {


                    result["username"]=data.value("username").toString();
                    result["content"]=data.value("content").toString();
                    return result;
                }
            else
            {
                qDebug()<<"current event is"<<data.value("eventname").toString()<<"but client is"<<current_event;
                }

            return result;
        }
    }
    else if(status=="error")
    {
        result["content"]="[error]"+message;
        result["username"]=UserService::instance().get_username();
        return result;

    }
}

#include <QtEndian>

void NetworkClient::sendJson(const QJsonObject& obj)
{
    QByteArray jsonData = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    quint32 payload_len = static_cast<quint32>(jsonData.size());

    // 总长度 = type(1) + payload_len(4) + payload
    quint32 total_len = 1 + 4 + payload_len;

    QByteArray packet;
    packet.resize(4 + total_len);

    // 写总长度（4B）
    qToBigEndian(total_len, reinterpret_cast<uchar*>(packet.data()));

    // 写类型 (1B) JSON = 0
    packet[4] = 0;

    // 写 payload_len (4B)
    qToBigEndian(payload_len, reinterpret_cast<uchar*>(packet.data() + 5));

    // 写 payload
    memcpy(packet.data() + 9, jsonData.constData(), payload_len);

    socket->write(packet);
    socket->flush();
    qDebug() << "sendJson:" << QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

void NetworkClient::set_current_event(const QString& eventname)
{
    qDebug()<<"network curretn is"<<eventname;
    current_event=eventname;
}

void NetworkClient::disconnectFromServer()
{
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        // 可选：发送 logout 消息
        QString username = UserService::instance().get_username();
        if(!username.isEmpty())
            logout(username);

        socket->flush();       // 先把缓存发出去
        socket->disconnectFromHost();
        if(socket->state() != QAbstractSocket::UnconnectedState)
            socket->waitForDisconnected(3000); // 等待最多3秒断开
    }
}



void NetworkClient::sendMessageOrFile(const QString& eventName, const QVariant& content)
{
    if (content.type() == QVariant::String) {
        // 普通文本消息，调用 sendJson
        QJsonObject obj;
        obj["type"] = "send";
        QJsonObject data;
        data["username"] = UserService::instance().get_username();
        data["eventname"] = eventName;
        data["content"] = content.toString();
        obj["data"] = data;

        sendJson(obj);

    } else if (content.type() == QVariant::StringList) {
        // 文件列表
        QStringList files = content.toStringList();
        for (const QString& filePath : files) {
            sendFile(eventName, filePath);
        }
    } else {
        qDebug() << "[sendMessageOrFile] Unsupported QVariant type:" << content.typeName();
    }
}
