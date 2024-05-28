#include "threadPool.hpp"
#include <unistd.h>
#include <iostream>
ThreadPool::ThreadPool(size_t threadNum, size_t queSize)
    : _threadNum(threadNum), _queSize(queSize), _que(_queSize), _isExit(0)
{
    _threads.reserve(_threadNum);
}

ThreadPool::~ThreadPool() {}

void ThreadPool::start()
{
    for (size_t idx = 0; idx < _threadNum; ++idx)
    {
        _threads.push_back(thread(&ThreadPool::doTask, this));
    }
}

void ThreadPool::stop()
{
    while (!_que.empty())
    {
        sleep(1);
    }

    _isExit = true;
    _que.wakeup();
    for (auto &it : _threads)
    {
        it.join();
    }
}

void ThreadPool::addTask(Elemtype &&cb)
{
    _que.push(std::move(cb));
}

Elemtype ThreadPool::getTask()
{
    return _que.pop();
}

void ThreadPool::doTask()
{
    while (!_isExit)
    {
        auto task = getTask();
        if (task)
        {
            task();
        }
        else
        {
            std::cout << "task = nullptr" << std::endl;
        }
    }
}