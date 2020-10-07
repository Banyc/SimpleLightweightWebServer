#pragma once

#include "ContentType.h"
#include "HttpMethods.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "LoginForm.h"
#include <string>

// __Message Format__
// follows http protocol

class HttpParser
{
    std::string _unsolvedMsg;

    static std::string TrimHeadBlanks(std::string str)
    {
        while (str.find(' ') == 0 || str.find('\n') == 0)
        {
            str = str.substr(1);
        }
        return str;
    }

    // if param < 0, it will be considered as max
    // return -1 if no positive min
    static int Min(int n1, int n2, int n3)
    {
        int min = -1;
        if (n1 >= 0)
        {
            min = n1;
        }
        if ((min > n2 || min == -1) && n2 >= 0)
        {
            min = n2;
        }
        if ((min > n3 || min == -1) && n3 >= 0)
        {
            min = n3;
        }
        return min;
    }

    // lineIndex from 0
    static std::string GetWord(std::string source, int lineIndex, int wordIndex)
    {
        std::string targetWord;
        std::string remaining = source;
        int returnPos; // in `remaining`
        int i;
        // locate line
        for (i = 0; i < lineIndex; i++)
        {
            returnPos = remaining.find('\n');
            if (returnPos == std::string::npos /* && i != lineIndex - 1 */) // if lineIndex is more than it has
            {
                return std::string();
            }
            remaining = remaining.substr(returnPos + 1);
        }

        // trim the first blanks
        remaining = TrimHeadBlanks(remaining);

        // locate word
        int blankPos;
        int endOfLine;
        for (i = 0; i < wordIndex; i++)
        {
            // find out the upper bound of column search
            endOfLine = remaining.find('\n');
            if (endOfLine == std::string::npos)
            {
                endOfLine = remaining.length();
            }

            // locate to the next word
            int spacePos = remaining.find(' ');
            if (spacePos == std::string::npos)
                spacePos = -1;
            int returnPos = remaining.find('\n');
            if (returnPos == std::string::npos)
                returnPos = -1;
            blankPos = Min(spacePos, returnPos, remaining.length());
            if (endOfLine == remaining.length() // remaining has reached to the EOF
                || blankPos >= endOfLine)       // remaining has reached to next row
            {
                return std::string();
            }
            remaining = remaining.substr(blankPos + 1);

            // trim the first blanks
            remaining = TrimHeadBlanks(remaining);
        }

        // get the first word in remaining
        int spacePos = remaining.find(' ');
        if (spacePos == std::string::npos)
            spacePos = -1;
        int returnPos2 = remaining.find('\n');
        if (returnPos2 == std::string::npos)
            returnPos2 = -1;
        blankPos = Min(spacePos, returnPos2, remaining.length());
        targetWord = remaining.substr(0, blankPos);

        return targetWord;
    }

    static bool IsRequestValid(HttpRequest request)
    {
        if (
            request.Method != UnknownHttpMethod)
            return true;
        else
            return false;
    }

    // CAUTION: use after Content is determined
    static bool IsRequestFull(HttpRequest request, bool DoesHeaderEnd)
    {
        // if (!DoesHeaderEnd)
        //     return false;
        // else
        if (request.Content.length() >= request.ContentLength)
            return true;
        else
            return false;
    }

    static void ReplaceAll(std::string &str, const std::string &from, const std::string &to)
    {
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

public:
    static std::string Serialize(HttpResponse response)
    {
        std::string serialized;

        serialized += response.Protocol + std::string(" ") + response.StateCode + std::string("\n");
        serialized += std::string("Server: ") + response.Server + std::string("\n");
        // if (response.StateCode == "200 OK")
        // {
            serialized += std::string("Content-Length: ") + std::to_string(response.ContentLength) + std::string("\n");
        // }
        if (response.StateCode == "404 Not Found")
        {
            response.Connection = "close";
        }
        serialized += std::string("Connection: ") + response.Connection + std::string("\n");
        // if (response.StateCode != "404 Not Found")
        // {
            serialized += std::string("Content-Type: ") + ContentTypeToString::ToString(response.CnttType) + std::string("\n");
        // }
        // if (response.StateCode == "200 OK")
        // {
            serialized += std::string("\n");
            serialized += response.Content;
        // }

        return serialized;
    }

    // loop it until all requests are popped out. CAUTION: DONT put the same buffer twice
    HttpRequest Deserialize(std::string serialized)
    {
        HttpRequest request;
        ReplaceAll(serialized, "\r\n", "\n");
        _unsolvedMsg += serialized;
        _unsolvedMsg = TrimHeadBlanks(_unsolvedMsg);

        std::string method = GetWord(_unsolvedMsg, 0, 0);
        std::string path = GetWord(_unsolvedMsg, 0, 1);
        std::string protocol = GetWord(_unsolvedMsg, 0, 2);

        // if (method.find("GET") >= 0)  // CAUTION: find() does NOT always return -1
        int getPos = method.find("GET");
        if (getPos != std::string::npos)
        {
            request.Method = GET;
        }
        else
        {
            int postPos = method.find("POST");
            if (postPos != std::string::npos)
            {
                request.Method = POST;
            }
            else
            {
                // this is not the new request
                // this should be appended to the previous
                request.Method = UnknownHttpMethod;
            }
        }
        // path
        request.Path = path;
        // content length
        int rowPos;
        rowPos = _unsolvedMsg.find("Content-Length");
        if (rowPos != std::string::npos)
        {
            std::string contentLenStr = GetWord(_unsolvedMsg.substr(rowPos), 0, 1);
            request.ContentLength = std::stoi(contentLenStr);
        }
        // std::cout << _unsolvedMsg << "<EOF>\n\n";  // debug
        // content
        int doubleReturnPos = _unsolvedMsg.find("\n\n");
        if (request.ContentLength > 0 && doubleReturnPos != std::string::npos && doubleReturnPos + 2 + request.ContentLength >= _unsolvedMsg.length()) // found double returns
        {
            // std::string contentAfterDoubleReturns
            request.Content = _unsolvedMsg.substr(doubleReturnPos + 2, request.ContentLength);
        }
        else
        {
            request.Content = std::string();
        }

        // update _unsolvedMsg
        if (IsRequestFull(request, doubleReturnPos != std::string::npos)) // next request, delete this request
        {
            if (doubleReturnPos != std::string::npos)
                _unsolvedMsg = _unsolvedMsg.substr(doubleReturnPos + 2 + request.ContentLength);
            else
                _unsolvedMsg.clear();
        }
        // if no full request is detected, leave it to concat other incoming buffer

        // decide returning
        HttpRequest empty;
        empty.IsEmpty = true;
        if (!IsRequestValid(request))
        {
            return empty;
        }
        else if (IsRequestFull(request, doubleReturnPos != std::string::npos))
        {
            request.IsEmpty = false;
            return request;
        }
        else
        {
            return empty;
        }
    }

    static LoginForm DeserializeLoginForm(std::string content)
    {
        std::string remaining = content;
        LoginForm form;
        std::string key;
        std::string value;
        int borderPos;
        int equalPos;

        remaining = TrimHeadBlanks(remaining);
        int partitionPos = remaining.find('&');
        if (partitionPos == std::string::npos)
            partitionPos = -1;
        borderPos = Min(partitionPos, remaining.length(), -1);
        equalPos = remaining.find("="); // WORKAROUND: assume it exists
        while (equalPos > 0)
        {
            key = remaining.substr(0, equalPos);
            value = remaining.substr(equalPos + 1, borderPos - equalPos - 1);
            if (key.compare("login") == 0)
            {
                form.Login = value;
            }
            else if (key.compare("pass") == 0)
            {
                form.Pass = value;
            }
            if (borderPos == remaining.length())
                break;
            remaining = remaining.substr(borderPos + 1);
            remaining = TrimHeadBlanks(remaining);
            int partitionPos = remaining.find('&');
            if (partitionPos == std::string::npos)
                partitionPos = -1;
            borderPos = Min(partitionPos, remaining.length(), -1);
            equalPos = remaining.find("="); // WORKAROUND: assume it exists
        }
        return form;
    }
};
