#ifndef __THREADPOOL_HPP__
#define __THREADPOOL_HPP__

#include "TaskQueue.hpp"

#include <vector>
#include <thread>  //C++11引入的标准库类；
using std::vector;
using std::thread;

class ThreadPool
{
public:
    ThreadPool(size_t threadNum, size_t queSize);
    ~ThreadPool();
    void start();
    void stop();
    void addTask(Elemtype &&cb);
    void doTask();
    Elemtype getTask();
    void initCache();
private:
    size_t _threadNum;
    vector<thread> _threads;
    size_t _queSize;
    TaskQueue _que;
    bool _isExit;
};

#endif