#include "PageLibProcesser.hpp"

#include <math.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "Configuration.hpp"

namespace SearchEngine
{
    using std::ifstream;
    using std::ofstream;
    using std::istringstream;
    PageLibProcesser::PageLibProcesser()
        : _tool(new WordSplit_JieBa)
    {
    }

    PageLibProcesser::~PageLibProcesser()
    {
        delete _tool;
    }

    void PageLibProcesser::doProcess()
    {
        readInfoFromfile();
        curRedundantPages();
        buildinvertIndexTable();
        storeOnDisk();
    }

    void PageLibProcesser::readInfoFromfile()
    {
        std::cout << "readInfoFromfile" << '\n';
        string pageLib = Configuration::getInstence()->getConfig()["PageLib"];
        string offsetLib = Configuration::getInstence()->getConfig()["OffSetLib"];
        string cnstopFile = Configuration::getInstence()->getConfig()["cnStopFile"];
        string enstopFile = Configuration::getInstence()->getConfig()["ebStopFile"];

        ifstream cnStopStream(cnstopFile);
        ifstream enStopStream(enstopFile);

        string word;
        while (cnStopStream >> word)
        {
            _stopWords.insert(word);
            word.clear();
        }
        while (enStopStream >> word)
        {
            _stopWords.insert(word);
            word.clear();
        }
        word.clear();
        ifstream offsetStream(offsetLib);
        while (getline(offsetStream, word))
        {
            pair<int, pair<int, int>> tmp;
            istringstream iss(word);
            iss >> tmp.first;
            iss >> tmp.second.first;
            iss >> tmp.second.second;
            _offsetLib.insert(tmp);
        }

        ifstream pageStream(pageLib, std::ios_base::binary);
        _pageLib.reserve(_offsetLib.size());

        for (auto &item : _offsetLib)
        {
            pageStream.seekg(item.second.first);
            string doc;
            doc.resize(item.second.second);
            pageStream.read(&doc[0], item.second.second);
            _pageLib.push_back(WebPage(string(doc), _tool, _stopWords));
            _pageLib.back().calcTopK();
        }
    }

    void PageLibProcesser::curRedundantPages()
    {
        std::cout << "curRedundantPages" << '\n';
        set<WebPage> unmuti;
        for (size_t i = 0; i < _pageLib.size(); ++i)
        {
            unmuti.insert(_pageLib[i]);
        }
        _pageLib.clear();
        for (auto &page : unmuti)
        {
            _pageLib.push_back(page);
        }
    }

    void PageLibProcesser::buildinvertIndexTable()
    {
        std::cout << "buildinvertIndexTable" << '\n';
        vector<map<string, int>> termFrequency;
        map<string, int> documentFrequency;
        int N = _pageLib.size();
        // 构建文档频率
        for (int i = 0; i < N; ++i)
        {
            auto wordsMap = _pageLib[i].getWordsMap();
            termFrequency.push_back(wordsMap);
            for (auto &word : wordsMap)
            {
                ++documentFrequency[word.first];
            }
        }
        for (int i = 0; i < N; ++i)
        {
            auto &wordsMap = termFrequency[i];
            int id = _pageLib[i].getDocId();
            map<string,double> weight;
            // 归一化处理文档频率的分母
            double denominator = 0;
            for(auto &words:wordsMap)
            {
                int tf = words.second;
                int df = documentFrequency[words.first];
                double idf = log2(N/(df+1));
                weight.insert({words.first,tf*idf});
                denominator+=(tf*idf*tf*idf);
            }
            // 归一化处理
            denominator = sqrt(denominator);
            for(auto &words:weight)
            {
                words.second/=denominator;
                _invertIndexTable[words.first].push_back({id,words.second});
            }
        }
    }

    void PageLibProcesser::storeOnDisk()
    {
        ofstream pageStream(Configuration::getInstence()->getConfig()["NewPageLib"]);
        ofstream offsetStream(Configuration::getInstence()->getConfig()["NewOffSetLib"]);
        ofstream invertIndex(Configuration::getInstence()->getConfig()["invertIndex"]);
        for(size_t i=0;i<_pageLib.size();++i)
        {
            int docId = _pageLib[i].getDocId();
            int start = pageStream.tellp();
            pageStream << _pageLib[i].getText();
            int end = pageStream.tellp();
            offsetStream << docId << ' ' << start << ' ' << end - start << '\n';
        }
        for(auto &item : _invertIndexTable)
        {
            invertIndex << item.first << ' ';
            for(size_t idx = 0;idx < item.second.size();++idx)
            {
                invertIndex << item.second[idx].first << ' ' << item.second[idx].second << ' ';
            }
            invertIndex << '\n';
        }
    }
} // namespace SearchEngine
