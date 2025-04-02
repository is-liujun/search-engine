#ifndef __ECHOSERVER_HPP__
#define __ECHOSERVER_HPP__

#include "TcpServer.hpp"
#include "threadPool.hpp"

class Server
{
public:
    Server(const string ip,unsigned short port,size_t threadNum,size_t queSize);
    ~Server();
    void start();
    void stop();
    void onConnection(const TcpConnectionPtr &ptr);
    void onMessage(const TcpConnectionPtr &ptr);
    void onClose(const TcpConnectionPtr &ptr);
    void setTimerCallBack(function<void()> &&cb);
private:
    TcpServer _server;
    ThreadPool _pool;
};

#endif