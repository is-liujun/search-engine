#ifndef __WORDCUT_HPP__
#define __WORDCUT_HPP__

#include <vector>
#include <string>
#include <set>
#include <unordered_set>
#include <utility>
#include <unordered_map>

using std::pair;
using std::set;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

namespace SearchEngine
{

    class WordSplit;

    class WordDict
    {
    public:
        // 需要进行分词的工具。
        WordDict(WordSplit *tool);

        vector<pair<string, int>> &getDict();

        unordered_map<string, set<int>> &getIndex();
        // 获取这个字符在utf8编码下的长度
        size_t getByteNumUTF8(const char byte);

    //private:
        // 获取一个目录下的所有文件
        vector<string> getFiles(string path);

        // 读取文件名获得内容
        void buildDict(string type);

        void getStopWords();

        // 建立下标
        void buildIndex();

        void storeDict(const string &filePath);

        void storeIndex(const string &filePath);

    private:
        WordSplit *_tool;                       // 中文分词工具
        vector<string> _cnFileNames;            // 存放中文语料库的文件名
        vector<string> _enFileNames;            // 存放英文语料库的文件名
        vector<pair<string, int>> _dict;        // 字典
        unordered_map<string, set<int>> _index; // 字符出现的字典下标
        string _cnDir;                          // 存放中文路径的文件夹
        string _enDir;                          // 存放英文语料的文件夹
        unordered_set<string> _stop_word;       // 存放停止词
    };

}

#endif