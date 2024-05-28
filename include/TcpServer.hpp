#ifndef __TCPSERVER_HPP__
#define __TCPSERVER_HPP__

#include "Acceptor.hpp"
#include "EventLoop.hpp"

class TcpServer
{
public:
    TcpServer(const string & ip, unsigned short port);
    ~TcpServer();

    void start();
    void stop();
    void setAllCallBack(TcpConnectionCallback &&cb1,TcpConnectionCallback &&cb2,TcpConnectionCallback &&cb3);
private:
    Acceptor _acceptor;
    Eventloop _event;
};

#endif
