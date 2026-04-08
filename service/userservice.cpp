#include "userservice.h"

UserService::UserService() {}
UserService& UserService::instance()
{
    // C++11 局部静态变量是线程安全的
    static UserService _instance;
    return _instance;
}
