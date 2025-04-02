#ifndef __LRUCACHE_HPP__
#define __LRUCACHE_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <utility>
#include <unordered_map>

using std::list;
using std::pair;
using std::string;
using std::unordered_map;
using std::vector;

template <class Key, class Value>
class Cache
{
public:
    Cache(size_t cacheSize);
    Value get(Key key);
    void put(Key key, Value value);
    const list<pair<Key, Value>> getList();
    bool empty();
    void startSync();
    void endSync();
    void swapCache();
    list<pair<Key, Value>> &getSyncCache();
    void copyCache();

private:
    size_t _capacity;
    bool _isSync;
    list<pair<Key, Value>> _cache;
    list<pair<Key, Value>> _cacheSync;
    list<pair<Key, Value>> _pendingList;
    unordered_map<Key, typename list<pair<Key, Value>>::iterator> _idx;
};

template <class Key, class Value>
Cache<Key, Value>::Cache(size_t cacheSize)
    : _capacity(cacheSize), _isSync(0)
{
}

template <class Key, class Value>
Value Cache<Key, Value>::get(Key key)
{
    auto it = _idx.find(key);
    if (it != _idx.end())
    {
        auto &it_cache = it->second;
        _cache.splice(_cache.begin(), _cache, it_cache); //没有对_idx进行更新？——前面it_cache是&，是更新了
        return it_cache->second;
    }
    return Value();
}

template <class Key, class Value>
const list<pair<Key, Value>> Cache<Key, Value>::getList()
{
    auto tmp = _pendingList;
    _pendingList.clear();
    return tmp;
}

template <class Key, class Value>
void Cache<Key, Value>::put(Key key, Value value)
{
    if (_isSync)
    {
        return;
    }
    _pendingList.push_back({key, value});
    auto it = _idx.find(key);
    if (it != _idx.end())
    {
        it->second->second = value;
        _cache.splice(_cache.begin(), _cache, it->second);
    }
    else
    {
        if (_cache.size() < _capacity)
        {
            _cache.push_front({key, value});
            _idx[key] = _cache.begin();
        }
        else
        {
            auto del = _cache.back();
            _cache.pop_back();
            _cache.push_front({key, value});
            _idx.erase(del.first);
            _idx[key] = _cache.begin();
        }
    }
}

template <class Key, class Value>
bool Cache<Key, Value>::empty()
{
    return _cache.empty();
}

template <class Key, class Value>
void Cache<Key, Value>::startSync()
{
    _isSync = true;
}

template <class Key, class Value>
void Cache<Key, Value>::endSync()
{
    _isSync = false;
}

template <class Key, class Value>
void Cache<Key, Value>::copyCache()
{
    _isSync = 1;
    _cacheSync = _cache;
    _isSync = 0;
}

template <class Key, class Value>
list<pair<Key, Value>> &Cache<Key, Value>::getSyncCache()
{
    return _cacheSync;
}

template <class Key, class Value>
void Cache<Key, Value>::swapCache()  //将同步的缓存放到_cache 和 unmape中，以便进行LRU
{
    _isSync = 1;
    std::swap(_cache, _cacheSync);  //std的交换函数
    _idx.clear();
    for (auto it = _cache.begin(); it != _cache.end(); ++it)  //list<pair<Key, Value>> _cache;
    {
        _idx[(*it).first] = it;
    }
    _isSync = 0;
}

#endif