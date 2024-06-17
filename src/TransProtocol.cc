#include "TransProtocol.hpp"

#include <sstream>
#include <string.h>

TransProtocol::TransProtocol(const char *str)
{
    _transId = *(reinterpret_cast<const int*>(str));
    _message = str+sizeof(int);
    _size = _message.size()+sizeof(int);
}

TransProtocol::TransProtocol(int transId, std::string message)
    : _size(message.size()+sizeof(int)), _transId(transId), _message(message)
{
}

std::string TransProtocol::toString()
{
    std::ostringstream oss;
    oss.write(reinterpret_cast<char*>(&_size),sizeof(int));
    oss.write(reinterpret_cast<char*>(&_transId),sizeof(int));
    oss << _message;
    return oss.str();
}

int TransProtocol::getTransId()
{
    return _transId;
}

std::string TransProtocol::getMessage()
{
    return _message;
}
