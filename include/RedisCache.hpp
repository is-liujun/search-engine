#ifndef __REDISCACHE_HPP__
#define __REDISCACHE_HPP__

#include <sw/redis++/redis++.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

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
    redisStruct.set(key, value, std::chrono::seconds(30));
}

string RedisCache::get(string key)
{
    auto res = redisStruct.get(key);
    if (res.has_value())
        return res.value();
    else
        return string();
}

#endif
