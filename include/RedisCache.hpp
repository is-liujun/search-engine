#ifndef __REDISCACHE_HPP__
#define __REDISCACHE_HPP__

#include <sw/redis++/redis++.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

//把原来的Cache换成了RedisCache；不需要再定时更新、LRU筛选了。。。
class RedisCache
{
public:
    RedisCache(string ip)
        : redisStruct(ip) {}
    ~RedisCache() {}
    void put(string, string);
    string get(string);

private:
    sw::redis::Redis redisStruct;
};

void RedisCache::put(string key, string value)
{
    redisStruct.set(key, value, std::chrono::seconds(30)); //将键值对设存储到Redis中，设置过期时间30s
}

string RedisCache::get(string key) //Redis中取值；
{
    auto res = redisStruct.get(key);// 从Redis中获取指定键的值；
    if (res.has_value())
        return res.value();
    else
        return string();
}

#endif
