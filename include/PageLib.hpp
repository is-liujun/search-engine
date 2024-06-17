#ifndef __PAGELIB_HPP__
#define __PAGELIB_HPP__

#include <map>
// #include <utility>

#include "DirScanner.hpp"
namespace SearchEngine
{
    using std::map;
    using std::pair;
    class PageLib
    {
    public:
        PageLib();
        ~PageLib();
        void create();
        void store();

    private:
        struct RssItem
        {
            string title;
            string link;
            string content;
        };
        DirScanner _dirScanner;
        vector<string> _pages;
        vector<RssItem> _items;
        map<int, pair<int, int>> _offsetLib;
    };
}
#endif
