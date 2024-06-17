#ifndef __TRANSPROTOCOL_HPP__
#define __TRANSPROTOCOL_HPP__

#include <string>

class TransProtocol
{
public:
    TransProtocol(const char *str);
    TransProtocol(int transId,std::string message);
    std::string toString();
    int getTransId();
    std::string getMessage();
private:
    int _size;
    int _transId;
    std::string _message;
};

#endif