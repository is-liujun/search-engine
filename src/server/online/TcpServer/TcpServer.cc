#include "TcpServer.hpp"

TcpServer::TcpServer(const string & ip, unsigned short port)
:_acceptor(ip,port)
,_event(_acceptor)
{

}

TcpServer::~TcpServer(){}

void TcpServer::start(){
    _acceptor.ready();
    _event.loop();
}
void TcpServer::stop(){
    _event.unloop();
}
void TcpServer::setAllCallBack(TcpConnectionCallback &&cb1,TcpConnectionCallback &&cb2,TcpConnectionCallback &&cb3){
    _event.setNewConnection(std::move(cb1));
    _event.setMessage(std::move(cb2));
    _event.setCloseConnection(std::move(cb3));
}

void TcpServer::setTimerCallBack(function<void()> &&cb)
{
    _event.setTimerCallBack(std::move(cb));
}