#ifndef __WEBPAGEQUERY_HPP__
#define __WEBPAGEQUERY_HPP__

#include <set>
#include <unordered_map>

#include "WordSplit.hpp"
#include "WebPage.hpp"
namespace SearchEngine
{
    using std::unordered_map;
class WebPageQuery
{
public:
    static WebPageQuery* getInstence();
    static void destroy();
    string doQuery(const string &str);
    void loadLibrary();
    vector<double> getQueryWordsWeightVector(const vector<string>&queryWords);
    bool executeQuery(const vector<string> &qeuryWords,vector<int> &resultVec);
    string createJson(vector<int> &docIdVec);
    string returnNoAnswer();
private:
    WebPageQuery();
    ~WebPageQuery();
private:
    static WebPageQuery* _pInstence;
    WordSplit *_tool;
    unordered_map<int,WebPage> _pageLib;
    unordered_map<int,std::pair<int,int>> _offsetLib;
    unordered_map<string,map<int,double>> _invertIndexTable;
};
    
} // namespace SearchEngine



#endif