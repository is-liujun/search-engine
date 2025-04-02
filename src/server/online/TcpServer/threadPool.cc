#include "threadPool.hpp"
#include <unistd.h>
#include <iostream>
ThreadPool::ThreadPool(size_t threadNum, size_t queSize)
    : _threadNum(threadNum), _queSize(queSize), _que(_queSize), _isExit(0)
{//初始化线程数、任务队列大小、初始化任务队列、推出标志；
    _threads.reserve(_threadNum); //线程池大小；
}

ThreadPool::~ThreadPool() {}

void ThreadPool::start()
{
    for (size_t idx = 0; idx < _threadNum; ++idx) //遍历线程数，创建指定数量的线程，并将线程的执行函数定义为doTask;
    {
        _threads.push_back(thread(&ThreadPool::doTask, this));  //thread是线程创建函数，封装了pthread_create 系统调用Api。。
    }
    initCache(); //初始化对应线程的存储Cache
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

void ThreadPool::doTask() //新线程开始执行指定的函数；
{                          
    while (!_isExit)  //线程池中的每个线程，不断从任务队列中获取任务并执行；
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