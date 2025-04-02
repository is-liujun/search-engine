#include "include/MainServer.hpp"
#include "include/Configuration.hpp"
#include "include/CacheManager.hpp"
#include "include/threadPool.hpp"
#include "include/Task.hpp"
#include "include/TransProtocol.hpp"
#include "include/WebPageQuery.hpp"
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

void Task::process() //查询网页的Task
{
    string res;
    TransProtocol recv(_msg.c_str()); //进行解析接受的_msg
    _msg = recv.getMessage();
    std::cout << "Task::process _msg = [" << _msg << "]\n";
    auto cache = CacheManager<string, string>::getInstence()->getCache(std::this_thread::get_id(), _msg);
    //这个cache是json格式的string（里面包含查询到的所有网页信息）
    if (cache.empty())
    {
        cache = SearchEngine::WebPageQuery::getInstence()->doQuery(_msg);
        //把查询到的文章，存入缓存中；
        CacheManager<string, string>::getInstence()->putCache(std::this_thread::get_id(), _msg, cache);
    }
    else
    {
        res += "[cache] ";
    }
    res += cache;
    std::cout << "query Result = " << res << '\n';
    TransProtocol message(100, res); //转成传输协议格式（总size(_message.size()+sizeof(int))、type(int)、_message）
    _ptr->sendToLoop(message.toString());
}

void test()
{
    string ip = SearchEngine::Configuration::getInstence()->getConfig()["ip"];
    int port = std::stoi(SearchEngine::Configuration::getInstence()->getConfig()["port"]);
    SearchEngine::WebPageQuery::getInstence()->loadLibrary(); //加载NewPageLib.dat、NewOffSetLib.dat（换成从redis中读取了）、invertIndex.dat、中英文停止词；
    Server server(ip, port, 4, 10);
    server.setTimerCallBack(std::bind(&CacheManager<string, string>::sync, CacheManager<string, string>::getInstence()));
    server.start();
}
int main()
{
    test();
    return 0;
}
