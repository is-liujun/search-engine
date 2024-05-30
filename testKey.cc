#include "KeyWord.hpp"
#include "MainServer.hpp"
#include "Configuration.hpp"
#include "CacheManager.hpp"
#include "threadPool.hpp"
#include "Task.hpp"
#include <iostream>
#include <memory>
#include <string>

using std::shared_ptr;

void ThreadPool::initCache()
{
    CacheManager<string,vector<string>>::getInstence();
    for(auto &th:_threads)
    {
        CacheManager<string,vector<string>>::getInstence()->initCache(th.get_id(),_threads.size());
    }
}

Task::Task(const string &msg, TcpConnectionPtr ptr)
    :_msg(msg)
    ,_ptr(ptr)
    {
        _msg.pop_back();
    }

void Task::process()
{
    string res;
    auto cache = CacheManager<string,vector<string>>::getInstence()->getCache(std::this_thread::get_id(),_msg);
    if(cache.empty())
    {
        cache = SearchEngine::KeyWord::getInstence()->query(_msg);
        CacheManager<string,vector<string>>::getInstence()->putCache(std::this_thread::get_id(),_msg,cache);
    }
    else
    {
        res += "[cache] ";
    }
    for(auto word:cache)
    {
        res += (word +" ");
    }
    res[res.size()-1] = '\n';
    _ptr->sendToLoop(res);
}


void test()
{
    string ip = SearchEngine::Configuration::getInstence()->getConfig()["ip"];
    int port = std::stoi(SearchEngine::Configuration::getInstence()->getConfig()["port"]);
    Server server(ip,port,4,10);
    server.setTimerCallBack(std::bind(&CacheManager<string,vector<string>>::sync,CacheManager<string,vector<string>>::getInstence()));
    server.start();
}
int main()
{
    test();
    return 0;
}

