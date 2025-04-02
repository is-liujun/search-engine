#include "KeyWord.hpp"
#include "MainServer.hpp"
#include "Configuration.hpp"
#include "CacheManager.hpp"
#include "threadPool.hpp"
#include "Task.hpp"
#include "TransProtocol.hpp"
#include <iostream>
#include <memory>
#include <string>

using std::shared_ptr;

void ThreadPool::initCache()
{
    CacheManager<string, vector<string>>::getInstence();
    for (auto &th : _threads)
    {
        CacheManager<string, vector<string>>::getInstence()->initCache(th.get_id(), _threads.size()); //管理的缓存容量不应该是线程的数量，不太合理。。。
    }
}

Task::Task(const string &msg, TcpConnectionPtr ptr)
    : _msg(msg), _ptr(ptr)
{
    _msg.pop_back();
}

void Task::process()
{
    string res;
    TransProtocol recv(_msg.c_str());
    _msg = recv.getMessage();
    auto cache = CacheManager<string, vector<string>>::getInstence()->getCache(std::this_thread::get_id(), _msg);
    if (cache.empty()) //当前词语没有在缓存中，则进行查询最相近的词汇；
    {
        cache = SearchEngine::KeyWord::getInstence()->query(_msg);
        //把查询到的词组，放入当前线程的缓存中；
        CacheManager<string, vector<string>>::getInstence()->putCache(std::this_thread::get_id(), _msg, cache);
    }
    else
    {
        res += "[cache] ";
    }
    for (auto word : cache)
    {
        res += (word + " ");
    }
    res[res.size() - 1] = '\n';
    std::cout << "query Result = "<<res << '\n';
    TransProtocol message(100,res); 
    _ptr->sendToLoop(message.toString()); //得到处理结果后，把发送信息的任务放到_peendings中挂起，然后向_evtfd中写，触发epoll_wait，让其他线程去完成发送任务；
}

void test()
{
    string ip = SearchEngine::Configuration::getInstence()->getConfig()["ip"];
    int port = std::stoi(SearchEngine::Configuration::getInstence()->getConfig()["port"]);

    //使用Epoll高效监听多个文件描述符、eventfd实现线程间通信、timerfd实现定时任务（初始时间和超时时间是5，5)；
    //初始化线程池4和任务队列大小10
    Server server(ip, port, 4, 10);

    //注册定时任务，周期性调用缓存管理器的 sync() 缓存同步；——触发间隔：
    //CacheManager：模板类，管理键为 string、值为 vector<string> 的缓存数据。
    //std::bind：将成员函数 sync 绑定到单例对象，生成可调用对象。。
    server.setTimerCallBack(std::bind(&CacheManager<string, vector<string>>::sync, CacheManager<string, vector<string>>::getInstence()));
    //这个bind操作，可拆解成：
        //auto& cache = CacheManager<string, vector<string>>::getInstence();
        //server.setTimerCallBack([&cache]() { cache.sync(); });
    
    server.start();
}
int main()
{
    test();
    return 0;
}
