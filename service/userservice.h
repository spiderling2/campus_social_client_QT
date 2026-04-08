#pragma once
#include<QString>

class UserService
{
public:
    UserService();
    static UserService& instance();
    void set_username(const QString &u_name){
        username=u_name;
    }
QString get_username()
    {
    return username;
    }


private:
    QString username;


};


