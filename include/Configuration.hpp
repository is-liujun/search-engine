#ifndef __CONFIGURATION_HPP__
#define __CONFIGURATION_HPP__

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>

using std::fstream;
using std::istringstream;
using std::string;
using std::unordered_map;

namespace SearchEngine
{

class Configuration
{
public:

    //单例模式的初始化
    static Configuration *getInstence();
    //单例模式的销毁
    static void destroy();

    //获取配置文件
    unordered_map<string, string> &getConfig();

private:
    Configuration();
    ~Configuration();
    //加载配置文件
    void loadfile(const string &filename = "../conf/myconf.conf");

private:
    static Configuration *_pInstence;

    //配置文件内容
    unordered_map<string, string> _config;
};

} // namespace SearchEngine

#endif