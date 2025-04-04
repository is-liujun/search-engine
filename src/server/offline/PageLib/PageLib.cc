#include "PageLib.hpp"

#include <iostream>
#include <map>
#include <fstream>
#include <regex>

#include "MyLog.hpp"
#include "Configuration.hpp"
#include "tinyxml2.hpp"

namespace SearchEngine
{
    using std::ifstream;
    using std::map;
    using std::ofstream;
    using std::regex;

    PageLib::PageLib() {}

    PageLib::~PageLib() {}

    /// @brief 判断一段文本是否全部有空白组成
    /// @param str 待检测短语
    /// @return true代表全部由空字符组成，false代表不全为空
    static bool isSpace(string &str)
    {
        for(auto &ch : str)
        {
            if(!isspace(ch))
            {
                return false;
            }
        }
        return true;
    }

    /// @brief 去掉一段文本中的全部html标签
    /// @param text 待去标签文本
    /// @return 返回去除文本后的标签
    static string removeLabel(string text)
    {
        string patt = "<([^/][\u4e00-\u9fa5\\w\\n ,!()+：@#;%?\":/.=-]*|/[\\w]*)>";
        regex reg(patt);
        std::sregex_iterator pos(text.begin(), text.end(), reg);
        std::sregex_iterator end;
        map<int, std::pair<string, int>> label;
        for (; pos != end; ++pos)
        {
            string value = pos->str();
            string label_name;
            int idx = 1;
            while (value[idx] != ' ')
            {
                if (value[idx] == '>')
                    break;
                label_name += value[idx];
                idx++;
            }
            label[pos->position()] = {label_name, value.size()};
        }
        if(label.empty())
        {
            return text;
        }
        char *content = new char[text.size()]();
        size_t idx = 0, idx_c = 0;
        auto it = label.begin();
        while (idx < text.size()) // 逐个遍历 content 字符串
        {
            if (label.count(idx)) // 如果这个位置的字符是标签的起始位置，
            {
                ++it; // 指向下个标签的起始位置
                if (it == label.end())
                    break;
                idx += label[idx].second; // 跳转到标签的结束位置再进行判断
            }
            else // 如果这个字符不是标签的起始位置，
            {
                idx_c += text.copy(&content[idx_c], it->first - idx, idx); // 将这段一直到下个标签复制到另一个字符串中。
                idx = it->first;                                           // 跳转到下个标签的起始位置。
            }
        }
        string ret(content); // 将复制的非标签文本转为字符串。
        delete[] content;
        return ret;
    }

    /// @brief 从磁盘加载未处理的xml格式的文档并进行格式化
    void PageLib::create()
    {
        string xmlDir = Configuration::getInstence()->getConfig()["XmlFile"];
        _dirScanner.scan(xmlDir);
        vector<string> fileList = _dirScanner.getFiles(); //获取../data/xmlFile目录下所有的绝对路径文件名；
        for (string filename : fileList)
        {
            MyLog::LogInfo("SearchEngine::PageLib::create(): open filename = %s\n",filename.c_str());
            using namespace tinyxml2; //利用第三方库，读取xml格式信息；；；
            XMLDocument doc;
            doc.LoadFile(filename.c_str());
            XMLNode *node = doc.FirstChild()->NextSibling()->FirstChild()->FirstChild();
            while (node) //不断的在xml文件中，找<item>标签，提取出title、link、description信息组成一个RssItem网页对象实例；
            {
                string name = node->Value();
                if ("item" == name)
                {
                    RssItem item;
                    for (XMLNode *elem = node->FirstChild(); elem; elem = elem->NextSibling())
                    {
                        string label = elem->Value();
                        if ("title" == label)
                        {
                            string text = elem->ToElement()->GetText();
                            item.title = text;
                        }
                        if ("link" == label)
                        {
                            item.link = elem->ToElement()->GetText();
                        }
                        if ("content" == label || "description" == label) //有的标签是content、有的是description
                        {
                            item.content = removeLabel(elem->ToElement()->GetText());
                        }
                    }
                    _items.push_back(item);
                    if(isSpace(_items.back().content)) //如果没有content或者description的，把这个网页实例删掉；
                    {
                        _items.pop_back();
                    }
                }
                node = node->NextSibling();
            }
        }
    }

    /// 将内存中的_items数组，重新序列化为XML格式并存储到PageLib文件中；同时记录每一个网页的起始位置和大小信息；
    void PageLib::store()
    {
        MyLog::LogInfo("SearchEngine::PageLib::store start\n");
        using namespace tinyxml2; //使用 tinyxml2 库动态创建XML节点
        ofstream ofsPage(Configuration::getInstence()->getConfig()["PageLib"]); //打开文件流；
        ofstream ofsOffset(Configuration::getInstence()->getConfig()["OffSetLib"]);

        for (size_t i = 0; i < _items.size(); ++i) //遍历所有的vector<RssItem> _items;网页实例
        {
            XMLDocument newDoc;
            XMLNode *root = newDoc.InsertEndChild(newDoc.NewElement("doc"));
            XMLNode *docid = root->InsertEndChild(newDoc.NewElement("id"));
            docid->ToElement()->SetText(i + 1);
            XMLNode *link = root->InsertEndChild(newDoc.NewElement("url"));
            link->ToElement()->SetText(_items[i].link.c_str());
            XMLNode *title = root->InsertEndChild(newDoc.NewElement("title"));
            title->ToElement()->SetText(_items[i].title.c_str());
            XMLNode *content = root->InsertEndChild(newDoc.NewElement("content"));
            content->ToElement()->SetText(_items[i].content.c_str());
            XMLPrinter printer;
            newDoc.Print(&printer);
            string text = printer.CStr(); //把XML文档转换成string
            _pages.push_back(text); //遍历每一个RssItem网页实例，再放入vector<string> _pages;

            size_t pos = ofsPage.tellp();
            ofsPage << text ; //把新形成的text写入pageLib.dat中；
            size_t newPos = ofsPage.tellp();
            _offsetLib[i + 1] = {pos, newPos-pos}; //计算网页实例id 和对应的起始位置，文件长度；map<int, pair<int, int>> _offsetLib; 
            ofsOffset << i + 1 << ' ' << _offsetLib[i + 1].first << ' ' << _offsetLib[i + 1].second << '\n';
        }
    }
} // namespace SearchEngine
