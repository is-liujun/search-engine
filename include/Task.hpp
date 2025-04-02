#ifndef __TASK_HPP__
#define __TASK_HPP__

#include "TcpConnection.hpp"

#include <string>
#include <memory>

using std::string;
using std::shared_ptr;

using TcpConnectionPtr = shared_ptr<TcpConnection>;

class Task
{
public:
    Task(const string &msg, TcpConnectionPtr ptr);
    void process();
private:
    string _msg;
    TcpConnectionPtr _ptr;
};

#endif