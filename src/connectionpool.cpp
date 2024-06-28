#include "connectionpool.hpp"
#include <fstream>
#include <thread>
#include <chrono>

using std::cv_status;
using std::ifstream;
using std::lock_guard;
using std::thread;
using std::unique_lock;

ConnectionPool::ConnectionPool()
    : _connectionCount{0}
{
    // 加载配置
    if (!loadConfig())
    {
        return;
    }

    // 初始化连接池
    for (int i = 0; i < _initSize; i++)
    {
        Connection *p = new Connection();
        p->connect(_ip, _port, _username, _password, _dbname);
        _connectionCount++;
        _connectionQue.push(p);
        p->refreshIdleTime();
    }

    // 启动一个线程，作为生产者线程
    thread producer(std::bind(&ConnectionPool::produceConnectionTask, this));
    producer.detach();

    // 启动一个线程，定时扫描队列，释放空闲连接
    thread scanner(std::bind(&ConnectionPool::scanIdleConnection, this));
    scanner.detach();
}

// 线程安全的懒汉单例,获取一个连接池实例对象
ConnectionPool *ConnectionPool::getInstance()
{
    static ConnectionPool pool;
    return &pool;
}

// 读取配置项
bool ConnectionPool::loadConfig()
{
    ifstream file("my.conf");
    if (!file.is_open())
    {
        LOG("open my.conf failed!");
        return false;
    }

    string line;
    while (getline(file, line))
    {
        int idx = line.find('=');
        if (idx == -1 || line[0] == '#')
        {
            // 当前行无效，继续读取下一行
            continue;
        }
        int endidx = line.find('\n', idx);

        string key = line.substr(0, idx);
        string value = line.substr(idx + 1, endidx - idx - 1);

        if (key == "ip")
        {
            _ip = value;
        }
        else if (key == "port")
        {
            _port = atoi(value.c_str());
        }
        else if (key == "username")
        {
            _username = value;
        }
        else if (key == "password")
        {
            _password = value;
        }
        else if (key == "dbname")
        {
            _dbname = value;
        }
        else if (key == "initSize")
        {
            _initSize = atoi(value.c_str());
        }
        else if (key == "maxSize")
        {
            _maxSize = atoi(value.c_str());
        }
        else if (key == "maxIdleTime")
        {
            _maxIdleTime = atoi(value.c_str());
        }
        else if (key == "connectionTimeout")
        {
            _connectionTimeout = atoi(value.c_str());
        }
    }

    file.close();
    return true;
}

// 连接生产者,运行在独立的线程中
void ConnectionPool::produceConnectionTask()
{
    for (;;)
    {
        unique_lock<mutex> lock(_queueMutex);
        while (!_connectionQue.empty())
        {
            // 连接队列不为空，生产者线程继续等待
            _cv.wait(lock);
        }

        // 连接队列空了，生产者开始生产连接,前提是没有超过最大连接数
        if (_connectionCount < _maxSize)
        {
            Connection *p = new Connection();
            p->connect(_ip, _port, _username, _password, _dbname);
            _connectionQue.push(p);
            p->refreshIdleTime();
            _connectionCount++;

            // 通知消费者可以消费了
            _cv.notify_all();
        }
        else
        {
            // 连接已达上限，睡眠3s
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }
}

// 获取连接
shared_ptr<Connection> ConnectionPool::getConnection()
{
    unique_lock<mutex> lock(_queueMutex);
    while (_connectionQue.empty())
    {
        if (cv_status::timeout == _cv.wait_for(lock, std::chrono::milliseconds(_connectionTimeout)))
        {
            LOG("connect timeout!"); // 获取连接超时
            return nullptr;
        }
    }

    // 自定义智能指针删除器，在shared_ptr析构时，归还连接回连接池
    shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection *p)
                              {
        lock_guard<mutex> lock(_queueMutex);
        _connectionQue.push(p);
        p->refreshIdleTime();
        _connectionCount++; });

    _connectionQue.pop();
    _connectionCount--;

    // 通知生产者检查连接队列，看是否需要生产新的连接
    _cv.notify_all();

    return sp;
}

// 扫描连接队列，释放掉超过最大空闲时间的多余连接
void ConnectionPool::scanIdleConnection()
{
    for(;;)
    {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        lock_guard<mutex> lock(_queueMutex);
        while(_connectionCount > _maxSize)
        {
            Connection* p = _connectionQue.front();
            if(p->getIdleTime() > (_maxIdleTime*1000))
            {
                // 超过最大空闲时间，释放连接
                _connectionQue.pop();
                _connectionCount--;
                delete p;
            }
            else
            {
                break;
            }
        }
    }
}