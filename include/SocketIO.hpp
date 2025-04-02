#ifndef __SOCKETIO_HPP__
#define __SOCKETIO_HPP__

#include <string>

class SocketIO
{
public:
    explicit SocketIO(int fd);
    ~SocketIO();

    std::string readn();
    int writen(std::string);
private:
    int _fd;
};
#endif
