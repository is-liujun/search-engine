#ifndef __WEBPAGE_HPP__
#define __WEBPAGE_HPP__

#include <map>
#include <set>
#include <unordered_set>
#include <string>
#include <vector>
#include "WordSplit.hpp"

namespace SearchEngine
{
    using std::map;
    using std::string;
    using std::set;
    using std::vector;
    using std::unordered_set;
    class WebPage
    {
        friend bool operator==(const WebPage&lhs,const WebPage &rhs);
        friend bool operator<(const WebPage&lhs,const WebPage &rhs);
    public:
        WebPage(const string &doc,WordSplit *tool,unordered_set<string> &_stop_words);
        int getDocId();
        string getTitle();
        string getContent();
        string getText();
        map<string,int>& getWordsMap();
        void calcTopK();
    private:
        const static int TOPK_NUMBER = 10;
        int _docId;
        string _docTitle;
        string _docUrl;
        string _docContent;
        string _docText;
        vector<string> _topWords;
        map<string,int> _wordsMap;
    };
} // namespace SearchEngine


#endif