#include "PageLib.hpp"

#include <iostream>
#include <map>
#include <fstream>
#include <regex>

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

    void PageLib::create()
    {
        string xmlDir = Configuration::getInstence()->getConfig()["XmlFile"];
        _dirScanner.scan(xmlDir);
        vector<string> fileList = _dirScanner.getFiles();
        for (string filename : fileList)
        {
            std::cout << filename << '\n';
            using namespace tinyxml2;
            XMLDocument doc;
            doc.LoadFile(filename.c_str());
            XMLNode *node = doc.FirstChild()->NextSibling()->FirstChild()->FirstChild();
            while (node)
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
                        if ("description" == label)
                        {
                            item.content = removeLabel(elem->ToElement()->GetText());
                        }
                    }
                    _items.push_back(item);
                    if(isSpace(_items.back().content))
                    {
                        _items.pop_back();
                    }
                }
                node = node->NextSibling();
            }
        }
    }
    void PageLib::store()
    {
        using namespace tinyxml2;
        ofstream ofsPage(Configuration::getInstence()->getConfig()["PageLib"]);
        ofstream ofsOffset(Configuration::getInstence()->getConfig()["OffSetLib"]);

        for (size_t i = 0; i < _items.size(); ++i)
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
            string text = printer.CStr();
            _pages.push_back(text);
            size_t pos = ofsPage.tellp();
            
            ofsPage << text ;
            size_t newPos = ofsPage.tellp();
            _offsetLib[i + 1] = {pos, newPos-pos};
            ofsOffset << i + 1 << ' ' << _offsetLib[i + 1].first << ' ' << _offsetLib[i + 1].second << '\n';
        }
    }
} // namespace SearchEngine
