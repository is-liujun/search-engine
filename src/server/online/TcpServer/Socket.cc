#include "Socket.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>

Socket::Socket(){
    _fd = ::socket(AF_INET,SOCK_STREAM,0);
    if(_fd<0){
        std::cerr << "socket" << std::endl;
        return;
    }
}

Socket::Socket(int fd)
:_fd(fd)
{}

Socket::~Socket(){
    close(_fd);
}

int Socket::fd() const{
    return _fd;
}

void Socket::shutDownWrite(){
    if(shutdown(_fd,SHUT_WR)){
        std::cerr << "shutdown" << std::endl;
    }
}