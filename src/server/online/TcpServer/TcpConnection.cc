#include "TcpConnection.hpp"
#include "EventLoop.hpp"
#include <iostream>
#include <sstream>

using std::cout;
using std::endl;
using std::ostringstream;

TcpConnection::TcpConnection(int fd,Eventloop *loop)
    : _sockIO(fd)
    , _loop(loop)
    , _sock(fd), _localAddr(getLocalAddr()), _peerAddr(getPeerAddr())
{
}

TcpConnection::~TcpConnection() {}

void TcpConnection::send(const string &msg)
{
    _sockIO.writen(msg.c_str(), msg.length());
}

void TcpConnection::sendToLoop(const string &msg)
{
    if(_loop)
    {
        _loop->sendToLoop(std::bind(&TcpConnection::send,this,msg));
        
    }
}

string TcpConnection::receive()
{
    char buf[1024] = {0};
    _sockIO.readLine(buf, sizeof(buf));
    return string(buf);
}

string TcpConnection::toString()
{
    ostringstream oss;
    oss << _localAddr.ip() << ":"
        << _localAddr.port() << "----->"
        << _peerAddr.ip() << ":"
        << _peerAddr.port();
    return oss.str();
}

void TcpConnection::setNewConnection(const TcpConnectionCallback &cb)
{
    _newConnection = cb;
}

void TcpConnection::setCloseConnection(const TcpConnectionCallback &cb)
{
    _closeConnection = cb;
}

void TcpConnection::setMessage(const TcpConnectionCallback &cb)
{
    _message = cb;
}

void TcpConnection::handleNewConnection()
{
    if (_newConnection)
    {
        _newConnection(shared_from_this());
    }
    else
    {
        cout << "_newConnection == nullptr" << endl;
    }
}

void TcpConnection::handleMessage()
{
    if (_message)
    {
        _message(shared_from_this());
    }
    else
    {
        cout << "_newConnection == nullptr" << endl;
    }
}

void TcpConnection::handleclose()
{

    if (_closeConnection)
    {
        _closeConnection(shared_from_this());
    }
    else
    {
        cout << "_newConnection == nullptr" << endl;
    }
}

InetAddress TcpConnection::getLocalAddr()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr);

    int ret = getsockname(_sock.fd(), (struct sockaddr *)&addr, &len);
    if (-1 == ret)
    {
        std::cerr << "getsockname" << endl;
    }
    return InetAddress(addr);
}

InetAddress TcpConnection::getPeerAddr()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr);
    int ret = getpeername(_sock.fd(), (struct sockaddr *)&addr, &len);
    if (-1 == ret)
    {
        std::cerr << "getpeername" << endl;
    }
    return InetAddress(addr);
}
