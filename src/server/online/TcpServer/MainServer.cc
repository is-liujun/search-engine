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
    _pool.start(); //启动线程池，创建指定数量的工作线程；
    using namespace std::placeholders; // 为了简化占位符的使用 或者用占位符时：std::placeholders::_1
    _server.setAllCallBack(std::bind(&Server::onConnection,this,_1)  //设置回调函数：onConnection当新连接建立时调用；
                            ,std::bind(&Server::onMessage,this,_1) //onMessage 当收到客户端消息时调用
                            ,std::bind(&Server::onClose,this,_1));  //onClose 当连接关闭时调用；
    _server.start(); //_serverTCP服务器对象，封装了网络通信逻辑（如 socket、bind、listen、accept) 
    //start(): 启动服务器，开始监听客户端连接并处理事件。
    
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
    std::cout << "Server receive: " << msg.size() << std::endl;
    Task task(msg,ptr);

    //task中包含消息信息控制process里面执行流程。。。
    _pool.addTask(std::bind(&Task::process,task)); //有消息进来，把任务函数加入任务队列；
}

void Server::setTimerCallBack(function<void()> &&cb) //右值接收函数，避免拷贝，并且bind（）绑定后是一个临时function对象——除非显示存储到变量中；
{
    _server.setTimerCallBack(std::move(cb)); //将cb函数利用move移动过去，避免拷贝，同理函数的接收参数也是右值引用；；
}