#include "KeyWord.hpp"

#include <queue>
#include <fstream>
#include <sstream>
#include <functional>

#include "MyLog.hpp"
#include "Configuration.hpp"

using std::ifstream;
using std::istringstream;
using std::priority_queue;
using std::reverse;

namespace SearchEngine
{
    KeyWord::Cmp::Cmp(string word, int idx, string value, int frequency)
        : _key(word), _dictIdx(idx), _value(value), _frequency(frequency), _editDistence(editDistence(word, _value))
    {
    }

    bool KeyWord::Cmp::operator<(const Cmp &rhs) const
    {
        if (this->_editDistence != rhs._editDistence)
        {
            return this->_editDistence < rhs._editDistence;
        }
        if (this->_frequency != rhs._frequency)
        {
            return this->_frequency < rhs._frequency;
        }
        return this->_value < rhs._value;
    }


    /// @brief 该函数是为了区分单字节的英文字符和多字节的utf-8汉字字符
    /// @param ch 英文字符或者汉字的首个字节
    /// @return 返回值为utf-8或者英文字符的所占字节数
    static size_t charSize(const char ch)
    {
        int byteNum = 0;
        for (size_t i = 0; i < 6; ++i)
        {
            if (ch & (1 << (7 - i)))
                ++byteNum;
            else
                break;
        }
        return byteNum == 0 ? 1 : byteNum;
    }

    /// @brief 该函数是为了将一段中英文混合的string短语转化为单个汉字或者单个字母的string数组
    /// @param word 需要进行分割的string短语
    /// @return 由单字或者字母组成的string数组
    static vector<string> trans(const string &word)
    {
        vector<string> res;

        for (size_t i = 0; i < word.size(); ++i)
        {
            size_t nBytes = charSize(word[i]);
            res.push_back(word.substr(i, nBytes));
            i += (nBytes - 1);
        }
        return res;
    }

    /// @brief 该函数是为了求出从word转化为dictword所需要的最小编辑距离
    /// @param word 待转化的短语
    /// @param dictWord 目标短语
    /// @return 返回最小的编辑距离
    int KeyWord::Cmp::editDistence(const string &word, const string &dictWord)
    {
        vector<string> word1 = trans(word);
        vector<string> word2 = trans(dictWord);

        int len1 = word1.size();
        int len2 = word2.size();
        // return process(word1,word2,len1-1,len2-1);
        int dp[len1][len2];
        dp[0][0] = word1[0] == word2[0] ? 0 : 1;
        for (int i = 1; i < len1; ++i)
        {
            if (word1[i] == word2[0])
            {
                dp[i][0] = i;
            }
            else
            {
                dp[i][0] = dp[i - 1][0] > i ? i + 1 : dp[i - 1][0] + 1;
            }
        }
        for (int j = 1; j < len2; ++j)
        {
            if (word1[0] == word2[j])
            {
                dp[0][j] = j;
            }
            else
            {
                dp[0][j] = dp[0][j - 1] > j ? j + 1 : dp[0][j - 1] + 1;
            }
        }
        for (int i = 1; i < len1; ++i)
        {
            for (int j = 1; j < len2; ++j)
            {
                if (word1[i] == word2[j])
                {
                    dp[i][j] = dp[i - 1][j - 1];
                }
                else
                {
                    int p1 = dp[i][j - 1];
                    int p2 = dp[i - 1][j - 1];
                    int p3 = dp[i - 1][j];
                    dp[i][j] = 1 + (p1 > p2 ? (p2 > p3 ? p3 : p2) : (p1 > p3 ? p3 : p1));
                }
            }
        }
        return dp[len1 - 1][len2 - 1];
    }

    KeyWord *KeyWord::_pInstence = KeyWord::getInstence();

    KeyWord *KeyWord::getInstence()
    {
        if (_pInstence == nullptr)
        {
            _pInstence = new KeyWord();
            atexit(destroy);
        }
        return _pInstence;
    }

    void KeyWord::destroy()
    {
        if (_pInstence)
        {
            delete _pInstence;
            _pInstence = nullptr;
        }
    }

    KeyWord::KeyWord()
    {
    }

    /// @brief 从磁盘文件中读取目标文件内容加载到内存中
    void KeyWord::loadFile()
    {
        string dictPath = Configuration::getInstence()->getConfig()["Dict"];
        string dictIndexPath = Configuration::getInstence()->getConfig()["DictIndex"];

        ifstream dict(dictPath);
        ifstream dictIndex(dictIndexPath);

        string line;
        string word;
        int frequency;
        while (getline(dict, line))
        {
            istringstream iss(line);
            iss >> word >> frequency;
            _dict.push_back({word, frequency});
        }
        while (getline(dictIndex, line))
        {
            istringstream iss(line);
            iss >> word;
            while (iss >> frequency)
            {
                _index[word].insert(frequency);
            }
        }
    }

    /// @brief 进行关键词推荐查询
    /// @param word 需要进行查询的关键词
    /// @return 与关键词最接近的几个词语
    vector<string> KeyWord::query(string word)
    {
        vector<string> cutWord = trans(word);
        MyLog::LogInfo("SearchEngine::KeyWord::query word = %s\n",word.c_str());
        priority_queue<Cmp, vector<Cmp>, Less> cnQue;
        priority_queue<Cmp, vector<Cmp>, Less> enQue;
        set<int> cnIndexList;
        set<int> enIndexList;
        for (auto &it : cutWord)
        {

            if (_index.count(it) > 0)
            {
                if (charSize(it[0]) == 1)
                {
                    enIndexList.insert(_index[it].begin(), _index[it].end());
                }
                else
                {
                    cnIndexList.insert(_index[it].begin(), _index[it].end());
                }
            }
        }

        for (int idx : cnIndexList)
        {
            cnQue.push(Cmp(word, idx, _dict[idx].first, _dict[idx].second));
            if (cnQue.size() > 5)
            {
                cnQue.pop();
            }
        }
        for (int idx : enIndexList)
        {
            enQue.push(Cmp(word, idx, _dict[idx].first, _dict[idx].second));
            if (enQue.size() > 5)
            {
                enQue.pop();
            }
        }
        size_t cnQueSize = cnQue.size();
        size_t enQueSize = enQue.size();
        cutWord.clear();
        if (cnQueSize && enQueSize)
        {
            for (size_t idx = 0; idx < enQueSize; ++idx)
            {
                if (idx > enQueSize / 2)
                {
                    cutWord.push_back(enQue.top()._value);
                }
                enQue.pop();
            }
            for (size_t idx = 0; idx < cnQueSize; ++idx)
            {
                if (idx > cnQueSize / 2)
                {
                    cutWord.push_back(cnQue.top()._value);
                }
                cnQue.pop();
            }
        }
        else if (cnQueSize)
        {
            for (size_t idx = 0; idx < cnQueSize; ++idx)
            {
                cutWord.push_back(cnQue.top()._value);
                cnQue.pop();
            }
        }
        else if (enQueSize)
        {
            for (size_t idx = 0; idx < enQueSize; ++idx)
            {
                cutWord.push_back(enQue.top()._value);
                enQue.pop();
            }
        }
        reverse(cutWord.begin(), cutWord.end());
        MyLog::LogInfo("SearchEngine::KeyWord::query word result num = %d\n",cutWord.size());
        return cutWord;
    }

}