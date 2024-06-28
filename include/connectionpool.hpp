#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include "common.hpp"
#include "connection.hpp"
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>

using std::atomic_int;
using std::mutex;
using std::queue;
using std::string;
using std::condition_variable;
using std::shared_ptr;

// 数据库连接池
class ConnectionPool
{
public:
    // 线程安全的懒汉单例
    static ConnectionPool *getInstance();

    // 获取连接
    shared_ptr<Connection> getConnection();

    // 扫描连接队列，释放掉超过最大空闲时间的多余连接
    void scanIdleConnection();

private:
    // 单例模式，构造函数私有化
    ConnectionPool();
    // 读取配置项
    bool loadConfig();
    // 连接生产者,运行在独立的线程中
    void produceConnectionTask();

    string _ip;             // mysql的ip地址
    unsigned short _port;   // mysql的port
    string _username;       // 连接mysql用的用户名
    string _password;       // 连接mysql时的密码
    string _dbname;         // 连接mysql哪个db
    int _initSize;          // 连接池初始连接数
    int _maxSize;           // 连接池最大连接数
    int _maxIdleTime;       // 连接池最大空闲时间
    int _connectionTimeout; // 连接池获取连接的超时时间

    queue<Connection *> _connectionQue; // 存储mysql连接的队列
    mutex _queueMutex;                  // 维护连接队列线程安全的mutex
    atomic_int _connectionCount; // 当前连接池中的连接数量
    condition_variable _cv; // 条件变量，用于线程间通信
};

#endif
