#ifndef __CACHEMANAGER_HPP__
#define __CACHEMANAGER_HPP__

#include "LRUCache.hpp"
#include <unordered_map>
#include <thread>
#include <iostream>
using std::unordered_map;

template <class Key, class Value>
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
    unordered_map<thread_id, Cache<Key, Value>> _caches;
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
    Cache<Key, Value> tmp(_caches.size());
    for (auto &cache : _caches)
    {
        if (!cache.second.empty())
        {
            auto &first = cache.second.get();
            for (auto &pair : first)
            {
                std::cout << "cache.first() = " << pair.first << std::endl;
                tmp.put(pair.first, pair.second);
            }
        }
    }

    for (auto &cache : _caches)
    {
        cache.second.startSync();
        cache.second = tmp;
        cache.second.endSync();
    }
}

#endif