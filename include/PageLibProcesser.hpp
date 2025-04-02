#ifndef __PAGELIBPROCESSER_HPP__
#define __PAGELIBPROCESSER_HPP__

#include <unordered_map>

#include "WordSplit_JieBa.hpp"
#include "WebPage.hpp"

namespace SearchEngine
{
    using std::unordered_map;
    using std::pair;
    class PageLibProcesser
    {
    public:
        PageLibProcesser();
        ~PageLibProcesser();
        void doProcess();
        void readInfoFromfile();
        void curRedundantPages();
        void buildinvertIndexTable();
        void storeOnDisk();
    private:
        WordSplit *_tool;
        vector<WebPage> _pageLib;
        unordered_set<string> _stopWords;
        unordered_map<int,pair<int,int>> _offsetLib;
        unordered_map<string,vector<pair<int,double>>> _invertIndexTable;
    };
}
#endif
