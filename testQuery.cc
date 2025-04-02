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

void ThreadPool::initCache() //初始化各个线程的缓存
{
    CacheManager<string, vector<string>>::getInstence(); //词语Cache
    CacheManager<string, string>::getInstence(); //文章Cache
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
    if (recv.getTransId() == 1) //通过id来识别是查询词语（1），还是文章(2)
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
            res += (word + " "); //查询到的单词放到string中
        }
        res.pop_back(); //把最后一个" " 删除
        std::cout << "query Result = " << res << '\n';

        nlohmann::json jsonObject;
        jsonObject["res"] = res;
        TransProtocol message(100, jsonObject.dump()); //转成json，发给client
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
        : _sync(&Sync::threadFunc,this) //会创建单独的线程进行sync Cache
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

    void isTime() //当被唤醒时，执行那两个Cache更新（词语、文章）
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
    SearchEngine::WebPageQuery::getInstence()->loadLibrary(); //加载NewPageLib.dat、NewOffSetLib.dat（换成从redis中读取了）、invertIndex.dat、中英文停止词；
    SearchEngine::KeyWord::getInstence()->loadFile(); //加载字典Dict和字符出现的字典下标：vector<pair<string, int>> _dict unordered_map<string, set<int>> _index;
    
    Sync sync; //同时绑定两个定时更新任务，一个是更新词语推荐的Cache<string, vector<string>>，一个是文章的Cache<string, string>
    sync.setCallBack(std::bind(&CacheManager<string, vector<string>>::sync, CacheManager<string, vector<string>>::getInstence()), std::bind(&CacheManager<string, string>::sync, CacheManager<string, string>::getInstence()));
    
    Server server(ip, port, 4, 10);

    server.setTimerCallBack(std::bind(&Sync::isTime, &sync)); //定时函数，触发Cache更新
    server.start();
}
int main()
{
    test();
    return 0;
}
