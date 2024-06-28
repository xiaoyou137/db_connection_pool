#include "connection.hpp"
#include "common.hpp"

// 初始化数据库连接
Connection::Connection()
{
    _conn = mysql_init(nullptr);
}

// 释放连接资源
Connection::~Connection()
{
    if (_conn != nullptr)
    {
        mysql_close(_conn);
    }
}

// 连接数据库
bool Connection::connect(string ip, unsigned short port, string username, string password, string dbname)
{
    MYSQL *p = mysql_real_connect(_conn, ip.c_str(), username.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0);
    if (p != nullptr)
    {
        // C++默认字符编码是ASCII，如果不修改则中文乱码
        mysql_query(_conn, "set name gbk");
    }
    return p;
}

// 更新数据库
bool Connection::update(std::string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG(sql + "更新失败");
        return false;
    }
    return true;
}

// 查询数据库
MYSQL_RES *Connection::query(std::string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG(sql + "查询失败");
        return nullptr;
    }

    return mysql_use_result(_conn);
}

// 获取数据库连接
MYSQL *Connection::getConnection()
{
    return _conn;
}
