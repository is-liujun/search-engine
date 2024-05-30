#ifndef __LRUCACHE_HPP__
#define __LRUCACHE_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <utility>
#include <unordered_map>

using std::string;
using std::vector;
using std::list;
using std::pair;
using std::unordered_map;

template<class Key,class Value>
class Cache
{
public:
    Cache(size_t cacheSize);
    Value get(Key key);
    void put(Key key,Value value);
    const list<pair<Key,Value>> &get();
    bool empty();
    void startSync();
    void endSync();
    void operator=(Cache &);
private:
    size_t _capacity;
    bool _isSync;
    bool _isChange;
    list<pair<Key,Value>> _cache;
    unordered_map<Key,typename list<pair<Key,Value>>::iterator> _idx;
};

template <class Key, class Value>
Cache<Key, Value>::Cache(size_t cacheSize)
    : _capacity(cacheSize),_isSync(0),_isChange(0)
{
}

template <class Key, class Value>
Value Cache<Key, Value>::get(Key key)
{
    auto it = _idx.find(key);
    if (it != _idx.end())
    {
        auto &it_cache = it->second;
        _cache.splice(_cache.begin(), _cache, it_cache);
        _isChange = true;
        return it_cache->second;
    }
    return Value();

}

template<class Key,class Value>
const list<pair<Key,Value>> & Cache<Key,Value>::get()
{
    return _cache;
}

template <class Key, class Value>
void Cache<Key, Value>::put(Key key, Value value)
{
    if(_isSync)
    {
        return;
    }
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
    _isChange = 1;
}

template<class Key,class Value>
bool Cache<Key,Value>::empty()
{
    return _cache.empty();
}

template<class Key,class Value>
void Cache<Key,Value>::startSync()
{
    _isSync = true;
}

template<class Key,class Value>
void Cache<Key,Value>::endSync()
{
    _isSync = false;
    _isChange = 0;
}

template<class Key,class Value>
void Cache<Key,Value>::operator=(Cache<Key,Value> & rhs)
{
    _capacity = rhs._capacity;
    _cache = rhs._cache;
    _idx.clear();
    for(auto it = _cache.begin();it!=_cache.end();++it)
    {
        _idx[(*it).first] = it;
    }
}

#endif