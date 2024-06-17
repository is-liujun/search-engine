#include "WebPageQuery.hpp"

#include <math.h>

#include <iostream>
#include <iterator>
#include <algorithm>
#include <fstream>
#include "json.hpp"
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

    string WebPageQuery::doQuery(const string &str)
    {
        auto queryWords = _tool->cut(str);
        for(size_t idx = 0;idx<queryWords.size();++idx)
        {
            std::cout << queryWords[idx] << ' ';
        }
        std::cout << '\n';
        vector<int> resultVec;
        bool res = executeQuery(queryWords,resultVec);
        std::cout << "query result is " << resultVec.size() <<'\n';
        return res?createJson(resultVec):returnNoAnswer();
    }

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

        while (getline(offsetStream, word))
        {
            pair<int, pair<int, int>> tmp;
            istringstream iss(word);
            iss >> tmp.first;
            iss >> tmp.second.first;
            iss >> tmp.second.second;
            _offsetLib.insert(tmp);
        }
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
            pair<string,set<pair<int,double>>> tmp;
            iss >> tmp.first;
            int docId = 0;
            double weight = 0;
            while(iss >> docId >> weight)
            {
                tmp.second.insert({docId,weight});
            }
        }
    }

    vector<double> WebPageQuery::getQueryWordsWeightVector(const vector<string> &queryWords)
    {
        map<string, int> wordFrequency;
        for (auto &word : queryWords)
        {
            ++wordFrequency[word];
        }
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


    bool WebPageQuery::executeQuery(const vector<string> &qeuryWords, vector<int> &resultVec)
    {
        /*获取第0个单词的所有文章列表
        ，一边在这个文章的词频中查找剩余的词一边构建这个文章所对应的向量
        ，如果有一个没找到就放弃这篇文章。*/
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
                temp.push_back(wordMap[words]);
            }
            if(temp.size()==qeuryWords.size())
            {
                fileSet.insert({item.second.getDocId(),temp});
            }
        }
        if (fileSet.empty())
        {
            return false;
        }
        // 获取基准向量
        auto baseQueryWordWights = getQueryWordsWeightVector(qeuryWords);
        multimap<double, pair<int, vector<double>>, std::greater<double>> compareResult;
        // 计算余弦值并使用map排序
        for (auto &item : fileSet)
        {
            compareResult.insert({getCosValue(baseQueryWordWights, item.second), item});
        }
        // 按照降序输出
        for (auto &item : compareResult)
        {
            resultVec.push_back(item.second.first);
        }
        return true;
    }

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

    string WebPageQuery::returnNoAnswer()
    {
        nlohmann::json jsonObject;
        jsonObject["res"] = "404 Not Found";
        return jsonObject.dump();
    }
}