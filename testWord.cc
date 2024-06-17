#include "WordDict.hpp"
#include "WordSplit_JieBa.hpp"
#include <iostream>

void test()
{
    using namespace SearchEngine;
    WordSplit_JieBa tool;
    WordDict dict(&tool);
}
int main()
{
    test();
    return 0;
}
