#include "SocketIO.hpp"
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <fstream>
#include <sstream>

SocketIO::SocketIO(int fd){
    _fd = fd;
}

SocketIO::~SocketIO(){
    close(_fd);
}

std::string SocketIO::readn()
{
    std::cout << "SocketIO::readn" << '\n';
    int size = 0;
    ::recv(_fd,&size,sizeof(int),MSG_WAITALL);
    std::cout << "SocketIO::readn recv size = " << size << '\n';
    char str[size+1];
    str[size] = '\0';
    std::stringstream sstream;
    
    ::recv(_fd,str,size,MSG_WAITALL);
    sstream.write(str,sizeof(str));
    std::cout << "SocketIO::readn recv " << &(str[4]) << '\n';
    return sstream.str();
}


int SocketIO::writen(std::string msg) {
    return ::send(_fd,msg.c_str(),msg.size(),MSG_NOSIGNAL);
}