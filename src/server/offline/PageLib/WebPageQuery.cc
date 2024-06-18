#include "WebPageQuery.hpp"

#include <math.h>

#include <iostream>
#include <iterator>
#include <algorithm>
#include <fstream>
#include <sw/redis++/redis++.h>
#include "json.hpp"
#include "MyLog.hpp"
#include "Configuration.hpp"
#include "WordSplit_JieBa.hpp"

namespace SearchEngine
{
    using std::ifstream;
    using std::multimap;
    using std::pair;
    using std::set_intersection;

    WebPageQuery *WebPageQuery::_pInstence = WebPageQuery::getInstence();

    WebPageQuery::WebPageQuery()
    :_tool(new WordSplit_JieBa)
    {
    }

    WebPageQuery::~WebPageQuery()
    {
        if (_tool)
        {
            delete _tool;
        }
    }

    WebPageQuery *WebPageQuery::getInstence()
    {
        if (_pInstence == nullptr)
        {
            _pInstence = new WebPageQuery;
            atexit(destroy);
        }
        return _pInstence;
    }

    void WebPageQuery::destroy()
    {
        if (_pInstence)
        {
            delete _pInstence;
        }
    }

    /// @brief 对传入语句进行网页搜索并返回json格式化字符串
    /// @param str 待查询短语
    /// @return json格式化后的网页查询结果
    string WebPageQuery::doQuery(const string &str)
    {
        MyLog::LogInfo("SearchEngine::WebPageQuery::doQuery: queryWord is [%s]\n",str.c_str());
        auto queryWords = _tool->cut(str);
        for(size_t idx = 0;idx<queryWords.size();++idx)
        {
            std::cout << queryWords[idx] << ' ';
        }
        std::cout << '\n';
        vector<int> resultVec;
        bool res = executeQuery(queryWords,resultVec);
        MyLog::LogDebug("query result size is %d\n",resultVec.size());
        return res?createJson(resultVec):returnNoAnswer();
    }

    /// @brief 从硬盘中加载处理好的网页库以及倒排索引库
    void WebPageQuery::loadLibrary()
    {
        ifstream pageStream(Configuration::getInstence()->getConfig()["NewPageLib"]);
        ifstream offsetStream(Configuration::getInstence()->getConfig()["NewOffSetLib"]);
        ifstream invertStream(Configuration::getInstence()->getConfig()["invertIndex"]);
        ifstream cnStopStream(Configuration::getInstence()->getConfig()["cnStopFile"]);
        ifstream enStopStream(Configuration::getInstence()->getConfig()["ebStopFile"]);
        unordered_set<string> stopWords;
        string word;
        while (cnStopStream >> word)
        {
            stopWords.insert(word);
            word.clear();
        }
        while (enStopStream >> word)
        {
            stopWords.insert(word);
            word.clear();
        }
        word.clear();

        using namespace sw::redis;
        string redisIP = Configuration::getInstence()->getConfig()["redis"];
        Redis redisStruct(redisIP+'2');
        MyLog::LogInfo("SearchEngine::WebPageQuery::loadLibrary create RedisConnection: %s\n",(redisIP+'2').c_str());
        vector<string> keys;
        redisStruct.keys("*",std::back_inserter(keys));
        std::cout << "keys.size() = " << keys.size()<< '\n';
        for(auto &key:keys)
        {
            pair<int,pair<int,int>> tmp;
            tmp.first = std::atoi(key.c_str());
            tmp.second.first = std::atoi(redisStruct.hget(key,"start").value().c_str());
            tmp.second.second = std::atoi(redisStruct.hget(key,"length").value().c_str());
            
            _offsetLib.insert(tmp);
        }
        std::cout << "offsetLib.size = " << _offsetLib.size() << '\n';
        // while (getline(offsetStream, word))
        // {
        //     pair<int, pair<int, int>> tmp;
        //     istringstream iss(word);
        //     iss >> tmp.first;
        //     iss >> tmp.second.first;
        //     iss >> tmp.second.second;
        //     _offsetLib.insert(tmp);
        // }
        _pageLib.reserve(_offsetLib.size());

        for (auto &item : _offsetLib)
        {
            pageStream.seekg(item.second.first);
            string doc;
            doc.resize(item.second.second);
            pageStream.read(&doc[0], item.second.second);
            WebPage page(string(doc), _tool, stopWords);
            page.calcTopK();

            _pageLib.insert({page.getDocId(), std::move(page)});
        }

        while(getline(invertStream,word))
        {
            istringstream iss(word);
            pair<string,map<int,double>> tmp;
            iss >> tmp.first;
            int docId = 0;
            double weight = 0;
            while(iss >> docId >> weight)
            {
                tmp.second.insert({docId,weight});
            }
            _invertIndexTable.insert(tmp);
        }
        MyLog::LogDebug("_pageLib.size = %d , _offSetLib.size = %d , _invertIndexTable.size = %d\n",_pageLib.size(),_offsetLib.size(),_invertIndexTable.size());
    }

    /// @brief 将传入的queryWords生成对应本篇文章的权重值
    /// @param queryWords 需要生成权重的文本数组
    /// @return 单个词语对本句话的权重值数组
    vector<double> WebPageQuery::getQueryWordsWeightVector(const vector<string> &queryWords)
    {
        // 对于单独一篇文章，tf-idf算法退化为 -1*TF
        map<string, int> wordFrequency;
        for (auto &word : queryWords)
        {
            ++wordFrequency[word];
        }
        // 进行归一化处理
        double denominator = 0;
        for (auto &item : wordFrequency)
        {
            denominator += (item.second * item.second);
        }
        vector<double> res;
        for (size_t idx = 0; idx < queryWords.size(); ++idx)
        {
            res.push_back(wordFrequency[queryWords[idx]] / denominator);
        }
        return res;
    }
    /// @brief 生成两个向量的余弦值
    /// @param lhs 一个向量值
    /// @param rhs 一个向量值
    /// @return 两个向量的余弦值
    static double getCosValue(const vector<double> &lhs, const vector<double> &rhs)
    {
        double lhsDenominator = 0;
        double rhsDenominator = 0;
        double mutiply = 0;
        for (size_t i = 0; i < lhs.size(); ++i)
        {
            lhsDenominator += lhs[i] * lhs[i];
            rhsDenominator += rhs[i] * rhs[i];
            mutiply += lhs[i] * rhs[i];
        }
        return mutiply / (sqrt(lhsDenominator) * sqrt(rhsDenominator));
    }


    /// @brief 传入查询单词数组并将获得的文章id存入数组中
    /// @param qeuryWords 待查询单词数组
    /// @param resultVec 查询结果集
    /// @return 是否有结果
    bool WebPageQuery::executeQuery(const vector<string> &qeuryWords, vector<int> &resultVec)
    {
        /*遍历所有文章列表，一边在这个文章的词频中查找查询词一边构建这个文章所对应的向量
        ，如果有一个单词没有找到就放弃这篇文章。*/
        map<int, vector<double>> fileSet;
        for(auto &item:_pageLib)
        {
            auto &wordMap = item.second.getWordsMap();
            vector<double> temp;
            for(auto &words:qeuryWords)
            {
                if(wordMap.count(words)==0)
                {
                    break;
                }
                temp.push_back(_invertIndexTable.at(words).at(item.second.getDocId()));
            }
            if(temp.size()==qeuryWords.size())
            {
                fileSet.insert({item.second.getDocId(),temp});
            }
        }
        MyLog::LogDebug("executeQuery: fileSet.size = %d\n",fileSet.size());
        if (fileSet.empty())
        {
            return false;
        }
        // // 获取基准向量
        // //auto baseQueryWordWights = getQueryWordsWeightVector(qeuryWords);
        multimap<double, pair<int, vector<double>>, std::greater<double>> compareResult;
        // 计算文章所对应的BM25值并使用map排序
        auto sum =  [&fileSet](int idx)->double{
            double res = 0;
            for(auto value:fileSet.at(idx))
            {
                res+=value;
            }
            return res;
        };

        for (auto &item : fileSet)
        {
            compareResult.insert({sum(item.first), item});
        }
        // 按照降序输出
        for (auto &item : compareResult)
        {
            resultVec.push_back(item.second.first);
            MyLog::LogDebug("executeQuery: docId = %d , BM25 = %.3f\n",item.second.first,item.first);
        }
        return true;
    }

    /// @brief 按传入的文章id顺序将文章的标题和内容封装成json格式字符串
    /// @param docIdVec 文章id的集合
    /// @return json格式的字符串
    string WebPageQuery::createJson(vector<int> &docIdVec)
    {
        nlohmann::json jsonObject;
        for(auto id:docIdVec)
        {
            jsonObject[std::to_string(id)]["title"] = _pageLib.at(id).getTitle();
            jsonObject[std::to_string(id)]["content"] = '['+_pageLib.at(id).getContent()+']';
        }
        return jsonObject.dump();
    }

    /// @brief 查询没有结果时返回的内容
    /// @return 返回无结果的json格式字符串
    string WebPageQuery::returnNoAnswer()
    {
        nlohmann::json jsonObject;
        jsonObject["res"] = "404 Not Found";
        return jsonObject.dump();
    }
}