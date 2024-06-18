#include "MyLog.hpp"

namespace MyLog
{
Mylogger::Mylogger()
:_root(log4cpp::Category::getInstance("myLog"))
{
    using namespace log4cpp;
    // 格式
    /* BasicLayout *pbl = new BasicLayout(); */
    /* SimpleLayout *psl = new SimpleLayout(); */
    PatternLayout *pplo = new PatternLayout();
    pplo->setConversionPattern("%c: %d: [%p] %m%n");

    PatternLayout *ppl1 = new PatternLayout();
    ppl1->setConversionPattern("%c: %d: [%p] %m%n");

    // 目的地
    OstreamAppender *poa = new OstreamAppender("a",&cout);
    poa->setLayout(pplo);
    RollingFileAppender *pfa = new RollingFileAppender("RollingFileAppender","../log/mylog.log",1024*1024*10,3);
    pfa->setLayout(ppl1);

    // 记录器
    _root.addAppender(poa);
    _root.addAppender(pfa);

    // 过滤器
    _root.setPriority(Priority::DEBUG);
};

Mylogger::~Mylogger(){
    _root.shutdown();
};

void Mylogger::warn(const char *msg){
    _root.warn(msg);
}

void Mylogger::error(const char *msg){
    _root.error(msg);
}

void Mylogger::debug(const char *msg){
    _root.debug(msg);
}

void Mylogger::info(const char *msg){
    _root.info(msg);
}

Mylogger* Mylogger::_mlog = Mylogger::getInstance();

Mylogger* Mylogger::getInstance()
{
    if( _mlog==nullptr )
    {
        _mlog = new Mylogger();
        atexit(destroy);
    }
    return _mlog;
}

void Mylogger::destroy()
{
    if( _mlog != nullptr )
    {
        delete _mlog;
        _mlog = nullptr;
    }
}

}