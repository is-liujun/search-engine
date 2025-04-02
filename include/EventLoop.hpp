#ifndef __EVENTLOOP_HPP__
#define __EVENTLOOP_HPP__

#include <sys/epoll.h>

#include <map>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>

using std::function;
using std::map;
using std::mutex;
using std::shared_ptr;
using std::vector;

class Acceptor;
class TcpConnection;

using TcpConnectionPtr = shared_ptr<TcpConnection>;
using TcpConnectionCallback = function<void(const TcpConnectionPtr &)>;

class Eventloop
{
    using Functor = function<void()>;

public:
    Eventloop(Acceptor &acceptor);
    ~Eventloop();
    void loop();
    void unloop();
    void setNewConnection(TcpConnectionCallback &&);
    void setCloseConnection(TcpConnectionCallback &&);
    void setMessage(TcpConnectionCallback &&);
    void setTimerCallBack(Functor&&);
    void setTimer(int ,int);
    void sendToLoop(Functor &&cb);

private:
    void waitEpollFd();
    void handleNewConnection();
    void handleMessage(int fd);
    int createEpollfd();
    int createEventfd();
    int createTimerfd();
    void wakeup();
    void handleRead();
    void handleReadTime();
    void addEpollReadFd(int fd);
    void delEpollReadFd(int fd);
    void doPendings();

private:
    int _epfd;
    int _evtfd;
    int _timerfd;
    bool _isloop;
    vector<struct epoll_event> _evtlist;
    Acceptor &_acceptor;
    map<int, TcpConnectionPtr> _conns; //存储当前所有的client连接，<connfd，shared_ptr<TcpConnection>>
    TcpConnectionCallback _newConnection;
    TcpConnectionCallback _closeConnection;
    TcpConnectionCallback _message;
    Functor _timerfdCallBack;
    vector<Functor> _pendings;
    mutex _mutex;
};

#endif
