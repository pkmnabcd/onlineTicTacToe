#include "config.hpp"

#include "ConfigData.hpp"

#include <fstream>
#include <print>
#include <string>

ConfigData readConfig()
{
    ConfigData config;

    std::ifstream file("server_config.txt");
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }
        // Get rid of \r if needed
        if (line.back() == '\r')
        {
            line.pop_back();
        }
        if (line.substr(0, 9) == "serv_port")
        {
            std::string subs = line.substr(10); // get str starting at index 10
            std::erase(subs, ' ');
            std::erase(subs, '\t');
            subs.push_back('\0');
            config.m_serv_port = subs;
        }
        else
        {
            std::print("Warning: invalid config file parameter: '{}'\n", line.substr(0, 9));
        }
    }
    return config;
}
