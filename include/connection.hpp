#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <mysql/mysql.h>
#include <ctime>

using std::string;

/* 
数据库操作封装
*/

constexpr int SQL_LENTH = 1024;

// 数据库类，封装了数据库操作
class Connection
{
public:
    // 初始化数据库连接
    Connection();
    // 释放连接资源
    ~Connection();
    // 连接数据库
    bool connect(string ip, unsigned short port, string username, string password, string dbname);
    // 更新数据库
    bool update(std::string sql);
    // 查询数据库
    MYSQL_RES *query(std::string sql);
    // 获取数据库连接
    MYSQL *getConnection();

    // 连接入队时，刷新空闲起始时间
    void refreshIdleTime() { _idleTime = clock(); }
    // 返回当前连接的空闲时间，单位ms
    int getIdleTime() { return clock() - _idleTime; }

private:
    MYSQL *_conn;
    clock_t _idleTime;
};

#endif
