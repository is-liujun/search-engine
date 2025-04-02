#include "EventLoop.hpp"
#include "TcpConnection.hpp"
#include "Acceptor.hpp"

#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <iostream>

Eventloop::Eventloop(Acceptor &acceptor)  //初始化基于Epoll事件驱动的网络时间循环，配置监听新连接fd、线程间事件通知、定时任务；
:_epfd(createEpollfd())  //创建Epoll实例，用于监听，比select、poll性能高；
,_evtfd(createEventfd())  //创建事件文件描述符（线程间通知）？——当其他线程向_evtfd写入数据时（线程间触发任务），事件循环被唤醒；
,_timerfd(createTimerfd()) //创建定时器文件描述符？？——定时器超时，_timerfd被触发；
, _isloop(false)
, _evtlist(1024) //分配Epoll列表大小，用于epoll_wait返回的事件列表；
, _acceptor(acceptor) //接收器，处理新的连接。。。（socket+bind+listen）
{
    addEpollReadFd(acceptor.fd()); //监听新连接、事件通知、定时器
    addEpollReadFd(_evtfd);
    addEpollReadFd(_timerfd);
    setTimer(5,5);// 设置定时器（初始 5 秒，间隔 5 秒）
}

Eventloop::~Eventloop()
{
    ::close(_epfd);
    ::close(_evtfd);
}

void Eventloop::loop()
{
    _isloop = true;
    while (_isloop)
    {
        waitEpollFd();
    }
}

void Eventloop::unloop()
{
    _isloop = false;
}

void Eventloop::setNewConnection(TcpConnectionCallback &&con)
{
    _newConnection = std::move(con);
}

void Eventloop::setCloseConnection(TcpConnectionCallback &&con)
{
    _closeConnection = std::move(con);
}

void Eventloop::setMessage(TcpConnectionCallback &&con)
{
    _message = std::move(con);
}

void Eventloop::waitEpollFd()
{
    int nready = 0;
    do
    {
        nready = ::epoll_wait(_epfd, &_evtlist[0], _evtlist.size(), 3000); //最后的3000是epoll_wait的等待超时时间；nready是就绪的fd个数
    } while (nready == -1 && errno == EINTR);

    if (-1 == nready)
    {
        std::cerr << "epoll_wait" << std::endl;
        return;
    }
    else if (0 == nready)
    {
        std::cerr << "epoll_wait timeout!" << std::endl;
    }
    else
    {
        if (nready == (int)_evtlist.size()) //当就绪个数与当前监听事件列表大小相同，则扩大监听vector
        {
            _evtlist.resize(2 * nready);
        }
        for (int idx = 0; idx < nready; ++idx)
        {
            int fd = _evtlist[idx].data.fd;
            if (fd == _acceptor.fd())
            {
                handleNewConnection();
            }
            else if(fd == _evtfd) //线程间事件通知描述符；当其他线程向_evtfd中写数据时，事件循环从epoll_wait中唤醒；
            {
                handleRead();
                doPendings(); //处理挂起的任务，并不是CS连接相关的；
            }
            else if(fd == _timerfd)
            {
                handleReadTime();
                _timerfdCallBack();
            }
            else
            {
                handleMessage(fd);
            }
        }
    }
}

void Eventloop::handleNewConnection()
{
    int connfd = _acceptor.accept();
    if (connfd < 0)
    {
        std::cerr << "handleNewConnection" << std::endl;
        return;
    }

    addEpollReadFd(connfd);

    shared_ptr<TcpConnection> TcpPtr(new TcpConnection(connfd,this));

    TcpPtr->setNewConnection(_newConnection);
    TcpPtr->setCloseConnection(_closeConnection);
    TcpPtr->setMessage(_message);

    _conns.insert({connfd, TcpPtr}); //执行容器插入时，shared_ptr会进行拷贝操作，引用计数++

    TcpPtr->handleNewConnection();
}

void Eventloop::handleMessage(int fd)
{
    if (0 == _conns.count(fd))
    {
        std::cerr << "illegal file desciptor" << std::endl;
        return;
    }
    auto Tcp = _conns[fd];

    char buf[10] = {0};
    int ret = ::recv(fd, buf, sizeof(buf), MSG_PEEK);

    if (ret == 0)
    {
        Tcp->handleclose();
        delEpollReadFd(fd);
        _conns.erase(fd);
    }
    else
    {
        Tcp->handleMessage();
    }
}

int Eventloop::createEpollfd()
{
    int fd = ::epoll_create(1);
    if (fd < 0)
    {
        std::cerr << "epoll_create" << std::endl;
        return -1;
    }
    return fd;
}

int Eventloop::createEventfd()
{
    int evtfd = ::eventfd(0,0); //系统调用函数；
    if(evtfd<0)
    {
        std::cerr << "eventfd" << std::endl;
        return -1;
    }
    return evtfd;
}

int Eventloop::createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_REALTIME,0);
    if(timerfd<0)
    {
        std::cerr << "timerfd" << std::endl;
        return -1;
    }
    return timerfd;
}

void Eventloop::setTimerCallBack(Functor && cb)
{
    _timerfdCallBack = std::move(cb);
}

void Eventloop::setTimer(int initSec,int peridoSec)
{
    struct itimerspec newValue;
    newValue.it_value.tv_sec = initSec;
    newValue.it_value.tv_nsec = 0;
    newValue.it_interval.tv_sec = peridoSec;
    newValue.it_interval.tv_nsec = 0;
    int ret = timerfd_settime(_timerfd,0,&newValue,nullptr);
    if(ret)
    {
        std::cerr << "set timerfd error" << std::endl;
    }
}

void Eventloop::wakeup()  //通过写入_evtfd，可以从epoll_wait 中进行唤醒，处理任务；
{
    uint64_t num = 1;
    ssize_t ret = write(_evtfd,&num,sizeof(num));
    if(ret!=sizeof(num)){
        std::cerr << "write error" << std::endl;
    }
}

void Eventloop::handleRead()
{
    uint64_t num = 0;
    ssize_t ret = read(_evtfd,&num,sizeof(num)); //从_evtfd中，把数据读出，读取message的长度
    if(ret!=sizeof(num)){
        std::cerr << "read evtfd error" << std::endl;
    }
}

void Eventloop::handleReadTime()
{
    uint64_t num = 0;
    ssize_t ret = read(_timerfd,&num,sizeof(num));
    if(ret!=sizeof(num))
    {
        std::cerr << "read timerfd error" << std::endl;
    }
}

void Eventloop::addEpollReadFd(int fd)
{
    struct epoll_event evt;
    evt.events = EPOLLIN;
    evt.data.fd = fd;
    int ret = ::epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &evt);
    if (ret < 0)
    {
        std::cerr << "epoll_ctl add" << std::endl;
    }
}

void Eventloop::delEpollReadFd(int fd)
{
    struct epoll_event evt;
    int ret = ::epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, &evt);
    if (ret < 0)
    {
        std::cerr << "epoll_ctl add" << std::endl;
    }
}

void Eventloop::doPendings() //作用是：执行事件循环中挂起的任务；
{
    vector<Functor> tmp;
    std::unique_lock<mutex> ul(_mutex); //枷锁，避免多线程竞争；
    tmp.swap(_pendings); //将pendings中的任务移动到局部变量中；
    ul.unlock();

    for(auto &cb :tmp) //执行每个挂起的任务
    {
        cb();
    }
}

void Eventloop::sendToLoop(Functor &&cb) //有线程通过sendToLoop提交任务到_pendings；然后调用wakeup，向_evtfd中写入信息，唤醒epoll_wait;
{
    std::unique_lock<mutex> ul(_mutex);

    _pendings.push_back(std::move(cb));
    ul.unlock();

    wakeup();
}
