#include "PageLib.hpp"
#include "PageLibProcesser.hpp"
#include <iostream>
void test()
{
    SearchEngine::PageLib pagelib;
    std::cout << "start create" << '\n';
    pagelib.create();
    std::cout << "start store" << '\n';
    pagelib.store();

    SearchEngine::PageLibProcesser processer;
    processer.doProcess();
}

int main()
{
    test();
    return 0;
}