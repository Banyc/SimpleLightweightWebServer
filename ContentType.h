#pragma once

#include <string>

enum ContentType
{
    text_html,  //  HTML格式
    text_plain,  // 纯文本格式
    text_xml,  //  XML格式
    image_gif,  // gif图片格式
    image_jpeg,  // jpg图片格式
    image_png,  // png图片格式
    application_octet_stream  // default
};

class ContentTypeToString
{
public:
    static std::string ToString(enum ContentType type)
    {
        std::string typeStr;
        switch (type)
        {
        case text_html:
            typeStr += std::string("text/html; charset=gb2312");
            break;
        case text_plain:
            typeStr += std::string("text/plain; charset=gb2312");
            break;
        case text_xml:
            typeStr += std::string("text/xml");
            break;
        case image_gif:
            typeStr += std::string("image/gif");
            break;
        case image_jpeg:
            typeStr += std::string("image/jpeg");
            break;
        case image_png:
            typeStr += std::string("image/png");
            break;
        case application_octet_stream:
            typeStr += std::string("application/octet-stream");
            break;

        default:
            typeStr += std::string("application/octet-stream");
            break;
        }
        return typeStr;
    }
};
