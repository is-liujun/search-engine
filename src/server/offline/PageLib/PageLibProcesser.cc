#include "PageLibProcesser.hpp"

#include <math.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <sw/redis++/redis++.h>
#include "MyLog.hpp"
#include "Configuration.hpp"

namespace SearchEngine
{
    using std::ifstream;
    using std::ofstream;
    using std::istringstream;
    PageLibProcesser::PageLibProcesser()
        : _tool(new WordSplit_JieBa) //new WordSplit_JieBa 和 new WordSplit_JieBa()都是动态分配对象，但是不带()不会初始化成员变量；
    {
    }

    PageLibProcesser::~PageLibProcesser()
    {
        delete _tool;
    }

    /// @brief 加载格式化的文档进行去重并建立倒排索引
    void PageLibProcesser::doProcess()
    {
        readInfoFromfile(); //从硬盘加载数据
        curRedundantPages(); //对网页进行去重
        buildinvertIndexTable(); //构建倒排索引表
        storeOnDisk(); //将结果保存到磁盘
    }

    /// @brief 从硬盘中加载文件——读取网页库、偏移库、和停用词文件
    //将停用词存入 _stopWords（一个 set<string> 容器）
    //将偏移库文件 存入 _offsetLib（一个 map<int, pair<int, int>> 容器）。
    //将网页库文件 读取每篇文档构建webPage对象，存入_pageLib （vector<WebPage>容器）
    void PageLibProcesser::readInfoFromfile()
    {
        MyLog::LogInfo("SearchEngine::PageLibProcesser::readInfoFromfile start\n");
        string pageLib = Configuration::getInstence()->getConfig()["PageLib"];
        string offsetLib = Configuration::getInstence()->getConfig()["OffSetLib"];
        string cnstopFile = Configuration::getInstence()->getConfig()["cnStopFile"];
        string enstopFile = Configuration::getInstence()->getConfig()["ebStopFile"];

        ifstream cnStopStream(cnstopFile);
        ifstream enStopStream(enstopFile);

        string word;
        while (cnStopStream >> word) //把中文停止词和英文停止词都放入_stopWords中
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
        while (getline(offsetStream, word)) //把网页偏移数据存放到 _offsetLib中；
        {
            pair<int, pair<int, int>> tmp;
            istringstream iss(word);
            iss >> tmp.first;
            iss >> tmp.second.first;
            iss >> tmp.second.second;
            _offsetLib.insert(tmp); 
        }

        ifstream pageStream(pageLib, std::ios_base::binary); //把网页.dat读取到_pageLib中
        _pageLib.reserve(_offsetLib.size()); 

        for (auto &item : _offsetLib)
        {
            pageStream.seekg(item.second.first); //先利用seek定位到网页文件每一个xml数据的起始位置，截取这个xml网页信息
            string doc;
            doc.resize(item.second.second);
            pageStream.read(&doc[0], item.second.second); 
            _pageLib.push_back(WebPage(string(doc), _tool, _stopWords)); //这个_tool已经被初始化成了jieba分词工具
            _pageLib.back().calcTopK(); //把频率最高的10个单词存到当前WebPage对象的_topWords中；
        }
    }

    /// @brief 按照WebPage的相等以及比较算法对加载的文章进行去重
    //WebPage类需要实现operator< 和 operator==
    void PageLibProcesser::curRedundantPages()
    {
        MyLog::LogInfo("SearchEngine::PageLibProcesser::curRedundantPages start\n");
        set<WebPage> unmuti; //利用set进行去重；
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

    /// @brief 按照BM25算法生成每个词语在文档中的权重 并 生成倒排索引；
    void PageLibProcesser::buildinvertIndexTable()
    {
        MyLog::LogInfo("SearchEngine::PageLibProcesser::buildinvertIndexTable start\n");
        vector<map<string, int>> termFrequency; //存储每篇WebPage中词语和对应的词频；
        map<string, int> documentFrequency; //存储每个词语在整个文档集合中的频率
        int N = _pageLib.size();
        double docAvgLen = 0; //计算平均的文档长度
        // 构建文档频率
        for (int i = 0; i < N; ++i)
        {
            auto wordsMap = _pageLib[i].getWordsMap(); //获取WepPage的词语+词频
            docAvgLen += _pageLib[i].getContent().size(); 
            termFrequency.push_back(wordsMap);
            for (auto &word : wordsMap)
            {
                ++documentFrequency[word.first]; 
            }
        }
        docAvgLen /=N;
        int k1 = 1.5;
        int b = 0.75; //k1和b 是 BM25算法的参数
        for (int i = 0; i < N; ++i)  //遍历所有文档
        {
            auto &wordsMap = termFrequency[i];
            int id = _pageLib[i].getDocId();
            for(auto &words:wordsMap)
            {
                int tf = words.second; //词语在当前文档中的词频；
                int df = documentFrequency[words.first]; //词语在文档集合中的词频；
                double idf = log2((N-df+0.5)/(df+0.5)); //IDF是？
                //计算BM25
                double bm25 = idf*(tf*(k1+1))/(tf+k1*(1-b+b*(_pageLib[i].getContent().size()/docAvgLen)));
                _invertIndexTable[words.first].push_back({id,bm25}); //存储每个词语对应的文档ID和权重w
            }
        }
    }

    /// @brief 将生成的网页库以及倒排索引库保存到硬盘中
    void PageLibProcesser::storeOnDisk()
    {
        MyLog::LogInfo("SearchEngine::PageLibProcesser::storeOnDisk start\n");
        ofstream pageStream(Configuration::getInstence()->getConfig()["NewPageLib"]);
        ofstream invertIndex(Configuration::getInstence()->getConfig()["invertIndex"]);

        string redisIp = Configuration::getInstence()->getConfig()["redis"];
        using namespace sw::redis;
        Redis redisStruct(redisIp+'2'); //为何+2?
        MyLog::LogInfo("SearchEngine::PageLibProcesser::readInfoFromfile create RedisConnection: %s\n",(redisIp+'2').c_str());
        redisStruct.flushdb(); //清空当前redis数据库；
        for(size_t i=0;i<_pageLib.size();++i)
        {
            int docId = _pageLib[i].getDocId();
            int start = pageStream.tellp(); //获取输出文件最后位置；
            pageStream << _pageLib[i].getText(); //getText是返回文档的字符串格式（包含title、url、content）
            int end = pageStream.tellp();
            vector<pair<string,int>> insertvalue({{"start",start},{"length",end-start}});
            redisStruct.hset(std::to_string(docId),insertvalue.begin(),insertvalue.end()); //向redis中记录每篇文档ID和起始位置，长度；
        }
        for(auto &item : _invertIndexTable) //unordered_map<string,vector<pair<int,double>>> 
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
