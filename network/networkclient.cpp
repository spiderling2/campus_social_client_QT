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

    while (true) {
        // === 1. 如果正在接收文件，优先接收文件内容 ===
        if (receivingFile.mode) {
            qint64 remaining = receivingFile.filesize - receivingFile.receivedBytes;
            if (buffer.isEmpty()) break; // 没数据就等下一次 readyRead

            QByteArray chunk = buffer.left(remaining);
            buffer.remove(0, chunk.size());

            receivingFile.data.append(chunk);
            receivingFile.receivedBytes += chunk.size();

            if (receivingFile.receivedBytes >= receivingFile.filesize) {
                // 文件接收完成
                QDir dir(saveDir);
                QString savePath = dir.filePath(receivingFile.filename);
                QFile file(savePath);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(receivingFile.data);
                    file.close();
                    qDebug() << "file recived successfully:" << savePath;
                    // 发信号通知 UI 显示
                    emit fileReceived(current_event, receivingFile.filename, savePath);
                } else {
                    qDebug() << "error:" << savePath;
                }

                // 重置状态
                receivingFile.mode = false;
                receivingFile.data.clear();
                receivingFile.filesize = 0;
                receivingFile.receivedBytes = 0;
            }

            continue; // 继续处理缓冲区
        }

        // === 2. 如果还没在接收文件，先读取 JSON header ===
        if (expectedSize == 0) {
            if (buffer.size() < 4) break; // header 长度不够
            expectedSize = qFromBigEndian<qint32>(
                reinterpret_cast<const uchar*>(buffer.constData()));
            buffer.remove(0, 4);
        }

        if (buffer.size() < expectedSize) break; // JSON 数据未到齐

        QByteArray jsonData = buffer.left(expectedSize);
        buffer.remove(0, expectedSize);
        expectedSize = 0;

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject obj = doc.object();
            QJsonObject data = obj.value("data").toObject();

            if (data.contains("filename") && data.contains("filesize")) {
                // 开始接收文件
                receivingFile.filename = data.value("filename").toString();
                receivingFile.filesize = data.value("filesize").toVariant().toLongLong();
                receivingFile.receivedBytes = 0;
                receivingFile.data.clear();
                receivingFile.mode = true;

                qDebug() << "开始接收文件:" << receivingFile.filename
                         << "大小:" << receivingFile.filesize;
            } else {
                // 普通消息
                QtConcurrent::run([this, obj]() {
                    QVariantMap res = handle_message(obj);
                    if (!res.isEmpty())
                        emit messageReceived(res);
                });
            }
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

#include <QtEndian>  // 用于 qToBigEndian

// 私有方法，发送长度 + JSON 数据
void NetworkClient::sendJson(const QJsonObject& obj)
{
    QByteArray jsonData = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    qint32 len = jsonData.size();

    QByteArray header;
    header.resize(4);
    qToBigEndian(len, reinterpret_cast<uchar*>(header.data()));
    QJsonDocument doc(obj);

    // 转成字符串
    QString jsonStr = doc.toJson(QJsonDocument::Compact); // 或 QJsonDocument::Indented 美化输出

    // 输出到控制台
    qDebug() << jsonStr;

    socket->write(header);
    socket->write(jsonData);
    socket->flush();  // 可选：强制立即发送
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


void NetworkClient::sendMessageOrFile(const QString& eventName, const QVariant& content) {
    if (content.type() == QVariant::String) {
        // 普通文本
        QJsonObject obj;
        obj["type"] = "send";
        QJsonObject data;
        data["username"] = UserService::instance().get_username();
        data["eventname"] = eventName;
        data["content"] = content.toString();
        obj["data"] = data;
        sendJson(obj); // 直接发送 JSON
    } else if (content.type() == QVariant::StringList) {
        // 假设是文件路径列表
        QStringList files = content.toStringList();
        for (const QString& filePath : files) {
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly)) {
                emit errorOccurred(QString("无法打开文件: %1").arg(filePath));
                continue;
            }

            QByteArray fileData = file.readAll();
            file.close();

            // 构建 JSON 元信息
            QJsonObject obj;
            obj["type"] = "send_file";
            QJsonObject data;
            data["username"] = UserService::instance().get_username();
            data["eventname"] = eventName;
            data["filename"] = QFileInfo(filePath).fileName();
            data["filesize"] = static_cast<qint64>(fileData.size());
            obj["data"] = data;

            // 发送长度 + JSON
            QByteArray jsonData = QJsonDocument(obj).toJson(QJsonDocument::Compact);
            qint32 len = jsonData.size();
            QByteArray header;
            header.resize(4);
            qToBigEndian(len, reinterpret_cast<uchar*>(header.data()));
            QJsonDocument doc(obj);

            // 转成字符串
            QString jsonStr = doc.toJson(QJsonDocument::Compact); // 或 QJsonDocument::Indented 美化输出

            // 输出到控制台
            qDebug() << jsonStr;
            socket->write(header);
            socket->write(jsonData);
            socket->flush();

            // 然后发送文件内容
            socket->write(fileData);
            socket->flush();

        }
    }
}


