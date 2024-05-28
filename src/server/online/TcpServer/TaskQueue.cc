#include "TaskQueue.hpp"

TaskQueue::TaskQueue(size_t queSize)
:_queSize(queSize)
,_flag(1)
{}

TaskQueue::~TaskQueue(){}

bool TaskQueue::empty()const
{
    return _que.empty();
}

bool TaskQueue::full()const
{
    return _que.size()==_queSize;
}

void TaskQueue::push(Elemtype &&cb)
{
    std::unique_lock<mutex> um(_mutex);

    while(full()){
        _notFull.wait(um);
    }
    _que.push(std::move(cb));
    _notEmpty.notify_one();
}

Elemtype TaskQueue::pop()
{
    std::unique_lock um(_mutex);
    while(empty()&&_flag)
    {
        _notEmpty.wait(um);
    }
    if(!_flag){
        return nullptr;
    }

    auto tmp = _que.front();
    _que.pop();
    _notFull.notify_one();
    return tmp;
}

void TaskQueue::wakeup(){
    _flag = 0;
    _notEmpty.notify_all();
}