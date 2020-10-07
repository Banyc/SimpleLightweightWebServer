#pragma once

#include <string>
#include "ContentType.h"

class HttpResponse
{
public:
    bool IsEmpty = true;
    std::string Protocol = std::string("HTTP/1.1");
    std::string StateCode = std::string("200 OK");
    std::string Server = std::string("custom/1.0");
    int ContentLength = 0;
    // std::string Connection = std::string("close");
    std::string Connection = std::string("Keep Alive");
    ContentType CnttType = text_plain;

    std::string Content;
};
