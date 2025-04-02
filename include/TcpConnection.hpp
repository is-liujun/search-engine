#ifndef __TCPCONNECTION_HPP__
#define __TCPCONNECTION_HPP__

#include "Socket.hpp"
#include "SocketIO.hpp"
#include "InetAddress.hpp"

#include <memory>
#include <functional>

using std::shared_ptr;
using std::function;

class Eventloop;

class TcpConnection
:public std::enable_shared_from_this<TcpConnection>
{
    using TcpConnectionPtr = shared_ptr<TcpConnection>;
    using TcpConnectionCallback = function<void(const TcpConnectionPtr)>;
public:
    explicit TcpConnection(int fd,Eventloop *loop);
    ~TcpConnection();

    void send(const string &msg);
    void sendToLoop(const string &msg);
    string receive();
    string toString();
    void setNewConnection(const TcpConnectionCallback &);
    void setCloseConnection(const TcpConnectionCallback &);
    void setMessage(const TcpConnectionCallback &);
    void handleNewConnection();
    void handleMessage();
    void handleclose();
private:
    InetAddress getLocalAddr();
    InetAddress getPeerAddr();
private:
    SocketIO _sockIO;
    Eventloop *_loop;
    Socket _sock;
    InetAddress _localAddr;
    InetAddress _peerAddr;

    TcpConnectionCallback  _newConnection;
    TcpConnectionCallback  _closeConnection;
    TcpConnectionCallback  _message;
};
#endif
