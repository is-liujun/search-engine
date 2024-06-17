#include "WebPageQuery.hpp"
#include "KeyWord.hpp"
#include "MainServer.hpp"
#include "Configuration.hpp"
#include "CacheManager.hpp"
#include "threadPool.hpp"
#include "Task.hpp"
#include "TransProtocol.hpp"
#include "json.hpp"

#include <iostream>

void ThreadPool::initCache()
{
    CacheManager<string, vector<string>>::getInstence();
    CacheManager<string, string>::getInstence();
    for (auto &th : _threads)
    {
        CacheManager<string, vector<string>>::getInstence()->initCache(th.get_id(), _threads.size());
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
    if (recv.getTransId() == 1)
    {
        auto cache = CacheManager<string, vector<string>>::getInstence()->getCache(std::this_thread::get_id(), _msg);
        if (cache.empty())
        {
            cache = SearchEngine::KeyWord::getInstence()->query(_msg);
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
        res.pop_back();
        std::cout << "query Result = " << res << '\n';
        nlohmann::json jsonObject;
        jsonObject["res"] = res;
        TransProtocol message(100, jsonObject.dump());
        _ptr->sendToLoop(message.toString());
    }
    else if (recv.getTransId() == 2)
    {
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
        TransProtocol message(200, res);
        _ptr->sendToLoop(message.toString());
    }
}

struct Sync
{
    Sync()
        : _sync(&Sync::threadFunc,this)
    {
    }

    ~Sync()
    {
        _sync.join();
    }

    void setCallBack(function<void()> func1, function<void()> func2)
    {
        _functor1 = func1;
        _functor2 = func2;
    }

    void isTime()
    {
        _istime.notify_all();
    }

    void threadFunc()
    {
        std::unique_lock<mutex> um(_nothing);
        while (1)
        {
            _istime.wait(um);
            _functor1();
            _functor2();
        }
    }
    function<void()> _functor1;
    function<void()> _functor2;
    thread _sync;
    mutex _nothing;
    condition_variable _istime;
};

void test()
{
    string ip = SearchEngine::Configuration::getInstence()->getConfig()["ip"];
    int port = std::stoi(SearchEngine::Configuration::getInstence()->getConfig()["port"]);
    SearchEngine::WebPageQuery::getInstence()->loadLibrary();
    SearchEngine::KeyWord::getInstence()->loadFile();
    Sync sync;
    sync.setCallBack(std::bind(&CacheManager<string, vector<string>>::sync, CacheManager<string, vector<string>>::getInstence()), std::bind(&CacheManager<string, string>::sync, CacheManager<string, string>::getInstence()));
    Server server(ip, port, 4, 10);
    server.setTimerCallBack(std::bind(&Sync::isTime, &sync));
    server.start();
}
int main()
{
    test();
    return 0;
}
