#include "DirScanner.hpp"
#include <dirent.h>
#include <sys/types.h>
namespace SearchEngine
{
    DirScanner::DirScanner()
    {
    }

    DirScanner::~DirScanner()
    {
    }

    void DirScanner::scan(string dir)
    {
        DIR *pDir = opendir(dir.c_str());
        struct dirent *file;
        while ((file = readdir(pDir)) != nullptr)
        {
            if (file->d_name[0] == '.')
            {
                continue;
            }
            _files.push_back(dir + '/' + file->d_name);
        }
        closedir(pDir);
    }

    vector<string> &DirScanner::getFiles()
    {
        return _files;
    }
}