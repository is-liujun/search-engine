#include "WordDict.hpp"
#include "WordSplit_JieBa.hpp"
#include <iostream>
#include <memory>

using std::shared_ptr;

void test()
{
    using namespace SearchEngine;
    shared_ptr<WordSplit> tool(new WordSplit_JieBa());
    WordDict dict(tool.get());
}
int main()
{
    test();
    return 0;
}

