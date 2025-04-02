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
        _notFull.wait(um); //等待pop唤醒
    }
    _que.push(std::move(cb));
    _notEmpty.notify_one();
}

//_notEmpty和_notFull两个条件变量cond进行控制；
Elemtype TaskQueue::pop() //利用了mutex枷锁，多线程访问，当任务队列_que为空时——》wait；否则取出任务；
{
    std::unique_lock<mutex> um(_mutex);
    while(empty()&&_flag)
    {
        _notEmpty.wait(um); //任务队列为空——_notEmpty进入wait，等待push
    }
    if(!_flag){
        return nullptr;
    }

    auto tmp = _que.front();
    _que.pop();
    _notFull.notify_one(); //任务队列取出一个——_notFull进行唤醒；
    return tmp;
}

void TaskQueue::wakeup(){
    _flag = 0;
    _notEmpty.notify_all();
}
