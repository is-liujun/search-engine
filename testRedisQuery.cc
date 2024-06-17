#include "WebPageQuery.hpp"
#include "KeyWord.hpp"
#include "MainServer.hpp"
#include "RedisCache.hpp"
#include "Configuration.hpp"
#include "threadPool.hpp"
#include "Task.hpp"
#include "TransProtocol.hpp"
#include "json.hpp"

#include <iostream>

RedisCache *redisCache[2];
void releaseRedis()
{
    delete redisCache[0];
    delete redisCache[1];
}
void ThreadPool::initCache()
{
    string ip = SearchEngine::Configuration::getInstence()->getConfig()["redis"];
    redisCache[0] = new RedisCache(ip);
    std::cout << "redisStart ip: "<< ip << '\n';
    redisCache[1] = new RedisCache(ip + '1');
    std::cout << "redisStart ip: "<< ip+'1' << '\n';
    atexit(releaseRedis);
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
        auto cache = redisCache[0]->get(_msg);
        if (cache.empty())
        {
            vector<string> queryRes = SearchEngine::KeyWord::getInstence()->query(_msg);
            for (auto &word : queryRes)
            {
                cache += (word + " ");
            }
            cache.pop_back();
            redisCache[0]->put(_msg, cache);
        }
        else
        {
            res+= "[cache] ";
        }
        res+=cache;
        
        std::cout << "query Result = " << res << '\n';
        nlohmann::json jsonObject;
        jsonObject["res"] = res;
        TransProtocol message(100, jsonObject.dump());
        _ptr->sendToLoop(message.toString());
    }
    else if (recv.getTransId() == 2)
    {
        auto cache = redisCache[1]->get(_msg);
        if (cache.empty())
        {
            cache = SearchEngine::WebPageQuery::getInstence()->doQuery(_msg);
            redisCache[1]->put(_msg, cache);
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

void placeHolder()
{
    std::cout << "Sync do Nothing\n";
}

void test()
{
    string ip = SearchEngine::Configuration::getInstence()->getConfig()["ip"];
    int port = std::stoi(SearchEngine::Configuration::getInstence()->getConfig()["port"]);
    SearchEngine::WebPageQuery::getInstence()->loadLibrary();
    SearchEngine::KeyWord::getInstence()->loadFile();
    Server server(ip, port, 4, 10);
    server.setTimerCallBack(placeHolder);
    server.start();
}
int main()
{
    test();
    return 0;
}
