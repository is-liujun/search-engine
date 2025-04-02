#include "PageLib.hpp"
#include "PageLibProcesser.hpp"
#include <iostream>
void test()
{
    SearchEngine::PageLib pagelib;
    std::cout << "start create" << '\n';
    pagelib.create(); //遍历xml格式的文件，把所有的xml文件都读取一遍，然后把网页实例存储在 vector<RssItem> _items 中；
    std::cout << "start store" << '\n';
    pagelib.store(); //遍历vector<RssItem> _items，存储到文件（网页.dat、偏移.dat）

    SearchEngine::PageLibProcesser processer;
    processer.doProcess(); //生成倒排索引库，存储每个单词和含有的文章id、权重w（TF-IDF算法算出w权重值）
}

int main()
{
    test();
    return 0;
}