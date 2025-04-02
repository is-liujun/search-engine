#ifndef __ACCEPTOR_HPP__
#define __ACCEPTOR_HPP__

#include "Socket.hpp"
#include "InetAddress.hpp"
#include <string>
using std::string;

class Acceptor
{
public:
    Acceptor(const string &ip,const unsigned short port);
    ~Acceptor();
    void ready();
private:
    void setReuseAddr();
    void setReusePort();
    void bind();
    void listen();
public:
    int accept();
    int fd() const;
private:
    Socket _sock;
    InetAddress _addr;
};
#endif
