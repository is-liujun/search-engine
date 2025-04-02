#ifndef __WORDQUERY_HPP__
#define __WORDQUERY_HPP__

#include "WordDict.hpp"

#include <memory>

using std::shared_ptr;

namespace SearchEngine
{
    class KeyWord
    {
    public:
        static KeyWord* getInstence();
        static void destroy();
        void loadFile();
        vector<string> query(string word);

    private:
        KeyWord();
        struct Cmp
        {
            string _key;
            int _dictIdx;
            string _value;
            int _frequency;
            int _editDistence;

            Cmp(string word, int idx, string value, int frequency);
            bool operator<(const Cmp &rhs)const;
            int editDistence(const string &word, const string &dictWord);
        };
        struct Less
        {
            bool operator()(const Cmp &lhs, const Cmp &rhs)
            {
                return lhs < rhs;
            }
        };

    private:
        vector<pair<string, int>> _dict;
        unordered_map<string, set<int>> _index;
        static KeyWord* _pInstence;
    };
}
#endif