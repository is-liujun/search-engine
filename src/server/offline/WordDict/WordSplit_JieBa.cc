#include "WordSplit_JieBa.hpp"

namespace SearchEngine
{

WordSplit_JieBa::WordSplit_JieBa()
:_jieba(dict_path,model_path,user_dict_path,idf_dict_path,stop_words_path)
{}

std::vector<std::string> WordSplit_JieBa::cut(const std::string &word)
{
    std::vector<std::string> res;
    _jieba.Cut(word,res);
    return res;
}
}