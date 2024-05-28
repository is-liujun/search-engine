#include "KeyWord.hpp"
#include "MainServer.hpp"
#include "Configuration.hpp"
#include "Task.hpp"
#include <iostream>
#include <memory>
#include <string>

using std::shared_ptr;

Task::Task(const string &msg, TcpConnectionPtr ptr)
    :_msg(msg)
    ,_ptr(ptr)
    {
    }

void Task::process()
{
    auto wordList = SearchEngine::KeyWord::getInstence()->query(_msg);
    string res;
    for(auto &it : wordList)
    {
        res += (it+" ");
    }
    res[res.size()-1] = '\n';
    _ptr->sendToLoop(res);
}


void test()
{
    string ip = SearchEngine::Configuration::getInstence()->getConfig()["ip"];
    int port = std::stoi(SearchEngine::Configuration::getInstence()->getConfig()["port"]);
    Server server(ip,port,4,10);
    server.start();
}
int main()
{
    test();
    return 0;
}

