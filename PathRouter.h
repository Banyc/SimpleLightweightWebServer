#pragma once

#include <string>

#define PREFIX "/myapp"

class PathRouter
{
public:
    static std::string Route(std::string queryPath)
    {
        std::string realPath;
        if (queryPath.find(PREFIX) != std::string::npos && queryPath.find(PREFIX) == 0)
        {
            realPath = queryPath.substr(std::string(PREFIX).length());
        }
        // else
        // {
        //     realPath = "/html/404.html";
        // }
        return realPath;
    }
};
