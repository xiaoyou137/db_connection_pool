#include "connectionpool.hpp"
#include "connection.hpp"
#include <ctime>

using namespace std;

void test_connections_without_pool(int count)
{
    clock_t begin = clock();
    for (int i = 0; i < count; i++)
    {
        Connection *p = new Connection();
        p->connect("127.0.0.1", 3306, "root", "123456", "chat");
        p->update("insert into test(name, age, sex) values('zhangsan', 20, 'male')");
        delete p;
    }
    clock_t end = clock();
    cout << count << "次连接，用时: " << (end - begin) * 1000 / CLOCKS_PER_SEC << "ms!" << endl;
}

void test_connections_with_pool(int count)
{
    clock_t begin = clock();
    for (int i = 0; i < count; i++)
    {
        ConnectionPool *pool = ConnectionPool::getInstance();
        auto p = pool->getConnection();
        p->update("insert into test(name, age, sex) values('zhangsan', 20, 'male')");
    }
    clock_t end = clock();
    cout << count << "次连接，用时: " << (end - begin) * 1000 / CLOCKS_PER_SEC << "ms!" << endl;
}

void clearTestTable()
{
    Connection *p = new Connection();
    p->connect("127.0.0.1", 3306, "root", "123456", "chat");
    p->update("delete from test");
    delete p;
}

int main()
{
    cout << "不使用连接池，";
    test_connections_without_pool(1000);

    cout << "使用连接池， ";
    test_connections_with_pool(1000);

    //clearTestTable();

    cout << "不使用连接池，";
    test_connections_without_pool(5000);

    cout << "使用连接池， ";
    test_connections_with_pool(5000);

    //clearTestTable();

    cout << "不使用连接池，";
    test_connections_without_pool(10000);

    cout << "使用连接池， ";
    test_connections_with_pool(10000);
    //clearTestTable();
}