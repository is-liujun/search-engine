#ifndef __CACHEMANAGER_HPP__
#define __CACHEMANAGER_HPP__

#include "LRUCache.hpp"
#include <unordered_map>
#include <thread>
#include <iostream>
using std::unordered_map;


//缓存管理器，基于LRU和线程本地缓存，通过模板类支持任意类型的键值对，并且为每个线程维护独立的缓存实例，
template <class Key, class Value> //模板类，存储key,value信息；
class CacheManager
{
    using thread_id = std::thread::id;

public:
    static CacheManager *getInstence();
    static void destroy();
    void initCache(thread_id, size_t);
    Value getCache(thread_id, Key);
    void putCache(thread_id, Key, Value);
    void sync();

private:
    CacheManager();

    ~CacheManager();

private:
    static CacheManager *_pInstence;
    unordered_map<thread_id, Cache<Key,Value>> _caches;
    bool _isSync;
};

template <class Key, class Value>
CacheManager<Key, Value> *CacheManager<Key, Value>::getInstence()
{
    if (_pInstence == nullptr)
    {
        _pInstence = new CacheManager<Key, Value>();
        atexit(destroy);
    }
    return _pInstence;
}

template <class Key, class Value>
CacheManager<Key, Value> *CacheManager<Key, Value>::_pInstence = nullptr;

template <class Key, class Value>
void CacheManager<Key, Value>::destroy()
{
    if (_pInstence)
    {
        delete _pInstence;
        _pInstence = nullptr;
    }
}

template <class Key, class Value>
CacheManager<Key, Value>::CacheManager()
{
}

template <class Key, class Value>
CacheManager<Key, Value>::~CacheManager()
{
}

template <class Key, class Value>
void CacheManager<Key, Value>::initCache(thread_id id, size_t capacity)
{
    _caches.insert({id, Cache<Key, Value>(capacity)});
}

template <class Key, class Value>
Value CacheManager<Key, Value>::getCache(thread_id id, Key key)
{
    auto &cache = _caches.at(id);
    return cache.get(key);
}

template <class Key, class Value>
void CacheManager<Key, Value>::putCache(thread_id id, Key key, Value value)
{
    auto &cache = _caches.at(id);
    cache.put(key, value);
}

template <class Key, class Value>
void CacheManager<Key, Value>::sync()
{
    std::cout << "start to sync" << std::endl;
    // 选择第一个线程的缓存作为主缓存；
    auto mainCache = _caches.begin();

    //将主缓存的修改，复制到同步缓存中；
    mainCache->second.copyCache();
    auto &syncCache = mainCache->second.getSyncCache();

    // 遍历其他线程的缓存；将其修改都同步到主缓存的同步缓存中
    for (auto &cache : _caches)// pair<thread_id,Cache>
    {
        //跳过遍历主键
        if(cache.first == mainCache->first)
        {
            continue;
        }
        auto pendingList = cache.second.getList(); //list<pair<Key, Value>>
        for(auto &item:pendingList)
        {
            std::cout<<"sync cache first = "<<item.first<< '\n';
            syncCache.push_back({item.first,item.second});
        }
    }

    // 将主缓存的同步缓存同步到其他线程的缓存中；
    for (auto &cache : _caches)
    {
        if(cache.first == mainCache->first)
        {
            continue;
        }
        auto &otherSyncCache = cache.second.getSyncCache();
        otherSyncCache = syncCache;
        cache.second.swapCache();
    }
    mainCache->second.swapCache();
}

#endif