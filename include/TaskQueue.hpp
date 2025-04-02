#ifndef __TASKQUEUE_HPP__
#define __TASKQUEUE_HPP__

#include <queue>
#include <memory>
#include <functional>
#include <mutex>
#include <condition_variable>

using std::condition_variable;
using std::function;
using std::mutex;
using std::queue;
using std::shared_ptr;


using Elemtype = function<void()>;

class TaskQueue
{

public:
    TaskQueue(size_t queSize);
    ~TaskQueue();
    bool empty() const;
    bool full() const;
    void push(Elemtype &&cb);
    Elemtype pop();
    void wakeup();
private:
    size_t _queSize;
    queue<Elemtype> _que;
    mutex _mutex;
    condition_variable _notFull;
    condition_variable _notEmpty;
    bool _flag;
};

#endif