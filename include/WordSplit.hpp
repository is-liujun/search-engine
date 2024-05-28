#ifndef __WORDSPLIT_HPP__
#define __WORDSPLIT_HPP__


#include <iostream>
#include <vector>
#include <string>
#include <fstream>


namespace SearchEngine
{

class WordSplit
{
public:
    virtual std::vector<std::string> cut(const std::string &word) = 0;
    virtual ~WordSplit() = default;
};

}

#endif