#include "Acceptor.hpp"
#include <iostream>

Acceptor::Acceptor(const string& ip,const unsigned short port) 
:_sock()
,_addr(ip,port)
{}

Acceptor::~Acceptor()
{}

void Acceptor::ready() {
    setReuseAddr();
    setReusePort();
    bind();
    listen();
}

void Acceptor::setReuseAddr(){
    int on = 1;
    if(-1 == setsockopt(_sock.fd(), SOL_SOCKET, SO_REUSEADDR, &on,sizeof(on))){
        std::cerr << "setsockopt" << std::endl;
        return;
    }
}

void Acceptor::setReusePort(){
    int on = 1;
    if(-1 == setsockopt(_sock.fd(),SOL_SOCKET, SO_REUSEPORT,&on,sizeof(on))){
        std::cerr << "setsockopt" << std::endl;
        return;
    }
}

void Acceptor::bind(){
    int ret = ::bind(_sock.fd()
    ,(struct sockaddr *)_addr.getInetAddressPtr()
    ,sizeof(struct sockaddr));
    if(-1 == ret){
        std::cerr << "bind" << std::endl;
        return;
    }
}

void Acceptor::listen(){
    int ret = ::listen(_sock.fd(),128);
    if(-1 == ret) {
        std::cerr << "listen" << std::endl;
        return;
    }
}

int Acceptor::accept(){
    int connfd = ::accept(_sock.fd(),nullptr,nullptr);
    if(-1 == connfd){
        std::cerr << "accept" << std::endl;
        return -1;
    } 
    return connfd;
}

int Acceptor::fd() const{
    return _sock.fd();
}