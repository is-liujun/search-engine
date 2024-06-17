#include "WordDict.hpp"

#include <dirent.h>
#include <sys/types.h>

#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>

#include "WordSplit.hpp"
#include "Configuration.hpp"

using std::back_insert_iterator;
using std::find;
using std::ifstream;
using std::istringstream;

namespace SearchEngine
{
    WordDict::WordDict(WordSplit *tool)
        : _tool(tool)
    {
        string storeDictPath = Configuration::getInstence()->getConfig()["Dict"];
        string storeIndexPath = Configuration::getInstence()->getConfig()["DictIndex"];
        _cnDir = Configuration::getInstence()->getConfig()["cnDir"];
        _enDir = Configuration::getInstence()->getConfig()["enDir"];
        _cnFileNames = getFiles(_cnDir);
        _enFileNames = getFiles(_enDir);
        getStopWords();
        buildDict("cn");
        buildDict("en");
        buildIndex();
        storeDict(storeDictPath);
        storeIndex(storeIndexPath);
    }

    vector<pair<string,int>> &WordDict::getDict()
    {
        return _dict;
    }

    unordered_map<string,set<int>> &WordDict::getIndex()
    {
        return _index;
    }

    vector<string> WordDict::getFiles(string path)
    {
        vector<string> tmp;

        DIR *pDir = opendir(path.c_str());
        struct dirent *file;
        while ((file = readdir(pDir)) != nullptr)
        {
            if (file->d_name[0] == '.')
            {
                continue;
            }
            tmp.push_back(path + '/' + file->d_name);
        }
        closedir(pDir);
        return tmp;
    }

    void WordDict::buildDict(string type)
    {
        unordered_map<string, int> tmpDict;
        if (type == "en")
        {
            for (auto &file : _enFileNames)
            {
                ifstream ifs(file);
                string line;
                while (getline(ifs, line))
                {
                    string word;
                    istringstream iss(line);
                    while (iss >> word)
                    {
                        string deal_word;
                        deal_word.reserve(word.size());
                        for (char &c : word)
                        {
                            if (isupper(c))
                            {
                                c += 32;
                            }
                            if (isalpha(c)||'-'==c)
                            {
                                deal_word.push_back(c);
                            }
                            else
                            {
                                continue;
                            }
                        }
                        if (deal_word.back() == '-')
                        {
                            deal_word.pop_back();
                        }
                        if (deal_word != " " && (_stop_word.count(deal_word) == 0))
                        {
                            ++tmpDict[deal_word];
                        }
                    }
                }
            }
        }
        else if (type == "cn")
        {
            for (auto &file : _cnFileNames)
            {
                ifstream ifs(file);
                string line;
                while (getline(ifs, line))
                {
                    vector<string> list = _tool->cut(line);
                    for (size_t idx = 0;idx<list.size();++idx)
                    {
                        string word = list[idx];
                        if (word != " " && (_stop_word.find(word) == _stop_word.end()))
                        {
                            ++tmpDict[word];
                        }
                    }
                }
            }
        }
        std::copy(tmpDict.begin(), tmpDict.end(), std::back_inserter(_dict));
    }

    void WordDict::getStopWords()
    {
        string cn_stop_file = Configuration::getInstence()->getConfig()["cnStopFile"];
        string en_stop_file = Configuration::getInstence()->getConfig()["enStopFile"];

        ifstream cn_ifs(cn_stop_file);
        ifstream en_ifs(en_stop_file);
        string word;
        while (cn_ifs >> word)
        {
            _stop_word.insert(word);
            word.clear();
        }
        while (en_ifs >> word)
        {
            _stop_word.insert(word);
            word.clear();
        }
    }

    void WordDict::buildIndex()
    {
        int i = 0;
        for (auto elem : _dict)
        {
            string word = elem.first;
            size_t charNums = word.size() / getByteNumUTF8(word[0]);
            for (size_t idx = 0, n = 0; n != charNums; ++idx, ++n)
            {
                size_t charLen = getByteNumUTF8(word[idx]);
                string subWord = word.substr(idx, charLen);
                _index[subWord].insert(i);
                idx += (charLen - 1);
            }
            ++i;
        }
    }

    size_t WordDict::getByteNumUTF8(const char byte)
    {
        int byteNum = 0;
        for (size_t idx = 0; idx < 6; ++idx)
        {
            if (byte & (1 << (7 - idx)))
            {
                ++byteNum;
            }
            else
            {
                break;
            }
        }
        return byteNum == 0 ? 1 : byteNum;
    }

    void WordDict::storeDict(const string &filePath)
    {
        std::ofstream ofs(filePath);
        for(auto & elem:_dict)
        {
            ofs << elem.first << " " << elem.second << std::endl;
        }
    }

    void WordDict::storeIndex(const string &filePath)
    {
        std::ofstream ofs(filePath);
        for(auto &elem : _index)
        {
            ofs << elem.first << " ";
            for(auto num : elem.second)
            {
                ofs << num << " ";
            }
            ofs << std::endl;
        }
    }
}