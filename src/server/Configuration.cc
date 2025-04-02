#include "Configuration.hpp"

namespace SearchEngine
{

Configuration* Configuration::_pInstence = Configuration::getInstence();

Configuration* Configuration::getInstence()
{
    if(_pInstence == nullptr)
    {
        _pInstence = new Configuration();
        atexit(destroy);
    }
    return _pInstence;
}

void Configuration::destroy()
{
    if(_pInstence)
    {
        delete _pInstence;
    }
}

void Configuration::loadfile(const string &filename)
{
    std::ifstream ifs(filename);
    
    string line;

    while(getline(ifs,line)){

        if(line.empty()||line[0]=='#'){
            continue;
        }

        istringstream iss(line);

        string key;
        string value;
        iss >> key;
        iss >> value;
        _config[key] = value;
    }
}

unordered_map<string,string> &Configuration::getConfig()
{
    return _config;
}

Configuration::Configuration()
{
    loadfile();
}

Configuration::~Configuration()
{}

}