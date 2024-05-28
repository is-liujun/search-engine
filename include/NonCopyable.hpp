#ifndef __NONCOPYABLE_HPP__
#define __NONCOPYABLE_HPP__

class NonCopyable
{
protected:
    NonCopyable() {};
    ~NonCopyable() {};
private:
    NonCopyable(const NonCopyable &rhs) = delete;
    NonCopyable& operator=(const NonCopyable &rhs) = delete;
};
#endif
