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
        DirScanner _dirScanner; //指定目录，获取网页文件路径；
        vector<string> _pages;  //存储网页的原始内容
        vector<RssItem> _items; //存储解析后的网页数据；
        map<int, pair<int, int>> _offsetLib; //记录每个网页在文件中的偏移量（起始位置，长度）
    };
}
#endif
