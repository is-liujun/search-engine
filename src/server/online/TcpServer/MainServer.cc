#include "MainServer.hpp"
#include "TcpConnection.hpp"
#include "Task.hpp"
#include <iostream>

Server::Server(const string ip,unsigned short port,size_t threadNum,size_t queSize)
:_server(ip,port)
,_pool(threadNum,queSize)
{
}

Server::~Server()
{}

void Server::start()
{
    _pool.start();
    using namespace std::placeholders;
    _server.setAllCallBack(std::bind(&Server::onConnection,this,_1)
                            ,std::bind(&Server::onMessage,this,_1)
                            ,std::bind(&Server::onClose,this,_1));
    _server.start();
    
}

void Server::stop()
{
    _server.stop();
    _pool.stop();
}

void Server::onConnection(const TcpConnectionPtr &ptr){
    std::cout << "new connection from" <<ptr->toString() << std::endl;
}

void Server::onClose(const TcpConnectionPtr &ptr){
    std::cout << ptr->toString() << " disconnected" << std::endl;
}

void Server::onMessage(const TcpConnectionPtr &ptr){
    string msg =  ptr->receive();
    std::cout << "Server receive: " << msg << std::endl;
    Task task(msg,ptr);

    _pool.addTask(std::bind(&Task::process,task));
}

void Server::setTimerCallBack(function<void()> &&cb)
{
    _server.setTimerCallBack(std::move(cb));
}