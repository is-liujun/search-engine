#include "WebPageQuery.hpp"
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
    CacheManager<string, string>::getInstence();
    for (auto &th : _threads)
    {
        CacheManager<string, string>::getInstence()->initCache(th.get_id(), _threads.size());
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
    std::cout << "Task::process _msg = [" << _msg << "]\n";
    auto cache = CacheManager<string, string>::getInstence()->getCache(std::this_thread::get_id(), _msg);
    if (cache.empty())
    {
        cache = SearchEngine::WebPageQuery::getInstence()->doQuery(_msg);
        CacheManager<string, string>::getInstence()->putCache(std::this_thread::get_id(), _msg, cache);
    }
    else
    {
        res += "[cache] ";
    }
    res += cache;
    std::cout << "query Result = " << res << '\n';
    TransProtocol message(100, res);
    _ptr->sendToLoop(message.toString());
}

void test()
{
    string ip = SearchEngine::Configuration::getInstence()->getConfig()["ip"];
    int port = std::stoi(SearchEngine::Configuration::getInstence()->getConfig()["port"]);
    SearchEngine::WebPageQuery::getInstence()->loadLibrary();
    Server server(ip, port, 4, 10);
    server.setTimerCallBack(std::bind(&CacheManager<string, string>::sync, CacheManager<string, string>::getInstence()));
    server.start();
}
int main()
{
    test();
    return 0;
}
