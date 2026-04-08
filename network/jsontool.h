#pragma once

#include<QString>
#include<QJsonObject>

class JsonTool
{
public:
    static QJsonObject make_request(const QString &type,const QString &status,const QString &message="",const QJsonObject &data= QJsonObject())
    {
        QJsonObject req;
        req["type"]=type;
        req["status"]=status;
        req["message"]=message;
        req["data"]=data;
        return req;
    }
};
