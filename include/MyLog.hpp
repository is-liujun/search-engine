#ifndef __MYLOG_H__
#define __MYLOG_H__

#include <iostream>
#include <stdio.h>
#include <string>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/SimpleLayout.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/Category.hh>
#include <log4cpp/Priority.hh>
 
using std::cout;
using std::endl;
using std::string;
using std::to_string;

namespace MyLog
{

class Mylogger
{
public:
    template<class... Args>
    void warn(const char *msg,Args... args){
        _root.warn(msg,args...);
    }

    template<class ...Args>
    void error(const char *msg,Args ...args){
        _root.error(msg,args...);
    }

    template<class ...Args>
    void debug(const char *msg,Args ...args){
        _root.debug(msg,args...);
    }

    template<class ...Args>
    void info(const char *msg,Args ...args){
        _root.info(msg,args...);
    }

    void warn(const char *msg); //MyLog.cc中实现了这4个函数，上面的四个好像没有用到；；；；
    void error(const char *msg);
    void debug(const char *msg);
    void info(const char *msg);

    static Mylogger* getInstance();
    static void destroy();
private:
    Mylogger();
    ~Mylogger();
    
private:
    static Mylogger* _mlog;
    log4cpp::Category& _root;
};

//日志的宏定义：INFO(msg)，用于生成包含——[文件名：函数名：行号] + msg的日志信息；
#define INFO(msg) string("[")\
    .append(__FILE__).append(": ")\
    .append(__FUNCTION__).append(": ")\
    .append(to_string(__LINE__)).append("] ")\
    .append(msg).c_str()


//最终调用形式——日志宏：支持可变参数形式
//举例，LogDebug("User %s logged in", "Alice");——》mylog：2023-10-10 12:34:56 [DEBUG] [main.cpp: main: 8] User Alice logged in
#define LogInfo(msg,...) Mylogger::getInstance()->info(INFO(msg),##__VA_ARGS__);
#define LogWarn(msg,...) Mylogger::getInstance()->warn(INFO(msg),##__VA_ARGS__);
#define LogDebug(msg,...) Mylogger::getInstance()->debug(INFO(msg),##__VA_ARGS__);
#define LogError(msg,...) Mylogger::getInstance()->error(INFO(msg),##__VA_ARGS__);

} // namespace log


#endif