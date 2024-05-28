#include "EventLoop.hpp"
#include "TcpConnection.hpp"
#include "Acceptor.hpp"

#include <sys/eventfd.h>
#include <unistd.h>
#include <iostream>

Eventloop::Eventloop(Acceptor &acceptor)
:_epfd(createEpollfd())
,_evtfd(createEventfd())
, _isloop(false)
, _evtlist(1024)
, _acceptor(acceptor)
{
    addEpollReadFd(acceptor.fd());
    addEpollReadFd(_evtfd);
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
        nready = ::epoll_wait(_epfd, &_evtlist[0], _evtlist.size(), 3000);
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
        if (nready == _evtlist.size())
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
            else if(fd == _evtfd)
            {
                handleRead();
                doPendings();
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

    _conns.insert({connfd, TcpPtr});

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
    int evtfd = ::eventfd(0,0);
    if(evtfd<0)
    {
        std::cerr << "eventfd" << std::endl;
        return -1;
    }
    return evtfd;
}

void Eventloop::wakeup()
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
    ssize_t ret = read(_evtfd,&num,sizeof(num));
    if(ret!=sizeof(num)){
        std::cerr << "write error" << std::endl;
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

void Eventloop::doPendings()
{
    vector<Functor> tmp;
    std::unique_lock<mutex> ul(_mutex);
    tmp.swap(_pendings);
    ul.unlock();

    for(auto &cb :tmp)
    {
        cb();
    }
}

void Eventloop::sendToLoop(Functor &&cb)
{
    std::unique_lock<mutex> ul(_mutex);

    _pendings.push_back(std::move(cb));
    ul.unlock();

    wakeup();
}