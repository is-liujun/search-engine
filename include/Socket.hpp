#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__

#include "NonCopyable.hpp"

class Socket
:NonCopyable
{
public:
    Socket();
    explicit Socket(int fd);
    ~Socket();
    int fd() const;
    void shutDownWrite();
private:
    int _fd;
};

#endif
