# db_connection_pool

mysql数据库连接池

# 使用方式

运行autobuild.sh,生成的静态库文件在lib目录下，生成的test文件在bin目录下
注意：可执行文件目录下需要放置my.conf配置文件，用来配置数据库连接池模块选项。

# 测试结果

| 数据量 | 不使用连接池 | 使用连接池 |
| ------ | ------------ | ---------- |
| 1000   | 1550ms       | 50ms       |
| 5000   | 8920ms       | 170ms      |
| 10000  | 17200ms      | 370ms      |
![image text](https://github.com/xiaoyou137/groceries-repo/blob/main/pics/mysql-test.png)

# 功能设计实现

- connectionpool.cpp和connectionpool.hpp：连接池代码实现
- connection.cpp和connection.hpp：数据库操作代码

## 连接池主要包含以下功能点：

- 连接池只需要一个实例，所以使用线程安全的懒汉模式单例。
- 空闲连接全部全部维护在一个线程安全的队列中，使用互斥锁保护线程队列安全，并使用原子类型atomic_int的变量来记录当前线程池中的连接数量。
- 消费和生产连接的接口中，使用了unique_lock和condition_variable进行线程间通信。
- 队列中空闲时间超过maxIdleTime的连接会被释放，该操作是由一个独立线程定时扫描队列完成。
- 生产连接也是通过一个独立线程完成，当队列中的连接数量大于初始连接数，并且队列中有连接超过最大空闲时间，就释放空闲连接。
- 用户获取的连接用shared_ptr来管理，并且用lambda函数自定义删除器，在shared_ptr析构时返还连接回连接池。

# 连接池提高连接数据库效率的原理

TCP三次握手，四次挥手，mysql连接认证，mysql关闭连接回收所耗费的性能时间相当可观，引入连接池，即可大量节省这部分的时间开销。

# 关键技术点

mysql数据库编程、单例模式、C++11多线程编程、线程互斥、线程同步通信、基于CAS的原子类型、智能指针shared_ptr、lambda表达式、queue队列容器、生产者消费者模型
