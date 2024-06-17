#ifndef __DIRSCANNER_HPP__
#define __DIRSCANNER_HPP__

#include <vector>
#include <string>

namespace SearchEngine
{
    using std::string;
    using std::vector;

    class DirScanner
    {

    public:
        DirScanner();
        ~DirScanner();
        vector<string> &getFiles();
        void scan(string dir);

    private:
        vector<string> _files;
    };
}

#endif
