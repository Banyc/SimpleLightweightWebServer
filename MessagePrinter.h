#pragma once

// log printer

#include <stdio.h>
#include <iostream>
#include "HttpParser.h"
#include "SocketRole.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include <time.h>
#include <string.h>

class MessagePrinter
{
    int messageCnt = 0;
public:
    // void Print(SocketRole role, int sourceIdForMe, HttpResponse response)
    // {

    // }
    // sourceIdForMe: the socket id in my aspect
    void Print(SocketRole role, int sourceIdForMe, HttpRequest request)
    {
        time_t now;
        char *time_str;
        int len;

        int srcSockId;
        int desSockId;

        now = time(0);  // seconds from 1970.1.1
        time_str = ctime(&now);  // seconds to human-readable
        len = strlen(time_str);
        time_str[len - 1] = '\0';  // replace '\n' to '\0'
        // new line
        printf("\n");
        // print msg count
        printf("[%3d] ", messageCnt);
        messageCnt++;
        // print current time
        printf("[%s] ", time_str);
        // print source id if the role is Server
        if (role == Server)
            printf("Source ID %d, ", sourceIdForMe);
        
        // print content
        std::cout << request.Method << " " << request.Path << "\n";
        std::cout << request.Content << "\n";
    }
};
