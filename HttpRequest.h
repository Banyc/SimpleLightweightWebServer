#pragma once

#include "HttpMethods.h"

class HttpRequest
{
public:
    bool IsEmpty = true;
    HttpMethods Method = UnknownHttpMethod;
    std::string Path = std::string("/");  // remember to prepend a dot
    std::string Protocol = std::string("HTTP/1.1");

    int ContentLength = 0;

    std::string Content;
};
