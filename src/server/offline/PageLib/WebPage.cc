#include "WebPage.hpp"

#include <queue>

#include "Configuration.hpp"
#include "tinyxml2.hpp"

namespace SearchEngine
{
    using std::priority_queue;
    WebPage::WebPage(const string &doc, WordSplit *tool, unordered_set<string> &_stop_word)
    {
        _docText = doc;
        tinyxml2::XMLDocument xml;
        auto result = xml.Parse(doc.c_str(), doc.size());
        if (result != tinyxml2::XML_SUCCESS)
        {
            std::cerr << result << '\n';
        }
        tinyxml2::XMLNode *node = xml.FirstChild()->FirstChild();
        while (node)
        {
            string label = node->Value();
            if (label == "id")
            {
                _docId = std::stoi(node->ToElement()->GetText());
            }
            else if (label == "url")
            {
                _docUrl = node->ToElement()->GetText();
            }
            else if (label == "title")
            {
                _docTitle = node->ToElement()->GetText();
            }
            else if (label == "content")
            {
                _docContent = node->ToElement()->GetText();
            }
            node = node->NextSibling();
        }

        vector<string> res = tool->cut(_docContent);
        for (auto word : res)
        {
            if (word != " " && _stop_word.count(word) == 0)
            {
                ++_wordsMap[word];
            }
        }
    }

    int WebPage::getDocId()
    {
        return _docId;
    }

    string WebPage::getTitle()
    {
        return _docTitle;
    }
    string WebPage::getContent()
    {
        return _docContent;
    }
    string WebPage::getText()
    {
        return _docText;
    }

    map<string, int> &WebPage::getWordsMap()
    {
        return _wordsMap;
    }
    struct Less
    {
        bool operator()(const std::pair<string, int> &lhs, const std::pair<string, int> &rhs)
        {
            return lhs.second < rhs.second;
        }
    };

    void WebPage::calcTopK()
    {
        priority_queue<std::pair<string, int>, vector<std::pair<string, int>>, Less> top;
        for (auto words : _wordsMap)
        {
            top.push(words);
        }
        for (int i = 0; i < TOPK_NUMBER; i++)
        {
            if (!top.empty())
            {
                _topWords.push_back(top.top().first);
                top.pop();
            }
        }
    }

    bool operator==(const WebPage &lhs, const WebPage &rhs)
    {
        set<string> left(lhs._topWords.begin(), lhs._topWords.end());
        set<string> right(rhs._topWords.begin(), rhs._topWords.end());
        left.insert(right.begin(), right.end());
        return left.size() <= WebPage::TOPK_NUMBER + 2;
    }

    bool operator<(const WebPage &lhs, const WebPage &rhs)
    {
        if (lhs._docContent.size() != rhs._docContent.size())
        {
            return lhs._docContent.size() < rhs._docContent.size();
        }
        return lhs._docId < rhs._docId;
    }
}