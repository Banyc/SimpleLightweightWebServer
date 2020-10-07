#pragma once

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include "HttpParser.h"
#include "HttpRequest.h"
#include "BufferMng.h"
// #include "MessagePrinter.h"
#include "SocketRole.h"

class SocketMng
{
    int _sockId;
    char _buffer[1024] = {0};
    bool _ShouldHalt = false;
    BufferMng *_bufferMng = new BufferMng();
    pthread_cond_t *_toRespondEvent;
    pthread_mutex_t *_toRespondEventMutex;
    SocketRole _role;
    HttpParser _parser;

public:
    // `responser` should be `NULL` if the socket is a client
    SocketMng(int sock, SocketRole role, pthread_cond_t *toRespondEvent, pthread_mutex_t *toRespondEventMutex)
    {
        this->_sockId = sock;
        this->_toRespondEvent = toRespondEvent;
        this->_toRespondEventMutex = toRespondEventMutex;
        this->_role = role;
    }

    // keep looping to receive incoming message
    static void ReceivingLoop(pthread_cond_t *toRespondEvent, pthread_mutex_t *toRespondEventMutex, bool *ShouldHalt, int *sockId, char *buffer, BufferMng *bufferMng, SocketRole role)
    {
        int len;
        char *message = NULL;
        while (!*ShouldHalt)
        {
            len = read(*sockId, buffer, 1024);
            if (len >= 0)
            {
                pthread_mutex_lock(toRespondEventMutex);  // make sure the "wait" has something waitting before "signal"
                
                if (len == 0)  // disconnection detected
                {
                    *ShouldHalt = true;  // responser will shut this down
                }
                else
                {
                    bufferMng->NewBuffer(buffer, len);
                }

                // raise event
                pthread_cond_signal(toRespondEvent);
                pthread_mutex_unlock(toRespondEventMutex);
            }
        }
    }

    // from buffer
    HttpRequest GetRequestStruct()
    {
        std::string message = _bufferMng->DequeueBuffer();

        HttpRequest msgStruct;
        msgStruct = _parser.Deserialize(message);
        return msgStruct;
    }

    // start receiving
    void StartReceivingLoop()
    {
        std::thread newThread(ReceivingLoop, _toRespondEvent, _toRespondEventMutex, &_ShouldHalt, &_sockId, _buffer, _bufferMng, _role);
        newThread.detach();
    }

    int Send(HttpResponse response)
    {
        std::string serialized = HttpParser::Serialize(response);
        return send(_sockId, serialized.c_str(), serialized.length(), 0);
    }

    int Shutdown()
    {
        _ShouldHalt = true;
        int state;
        if (state = shutdown(_sockId, SHUT_RDWR) < 0)
        {
            printf("Fail to shutdown socket with ID %d\n", _sockId);
        }
        else
        {
            printf("Socket with ID %d succeeded to shutdown\n", _sockId);
        }
        return state;
    }

    bool *GetShutdownBoolPointer()
    {
        return &_ShouldHalt;
    }

    int GetID()
    {
        return _sockId;
    }

    SocketRole GetRole()
    {
        return _role;
    }

    std::string GetPeerIpAddress()
    {
        socklen_t len;
        struct sockaddr_storage addr;
        char *ipstr = new char[INET6_ADDRSTRLEN];
        int port;

        len = sizeof addr;
        getpeername(_sockId, (struct sockaddr*)&addr, &len);

        // deal with both IPv4 and IPv6:
        if (addr.ss_family == AF_INET) {
            struct sockaddr_in *s = (struct sockaddr_in *)&addr;
            port = ntohs(s->sin_port);
            inet_ntop(AF_INET, &(s->sin_addr), ipstr, INET_ADDRSTRLEN);

        } else { // AF_INET6
            struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
            port = ntohs(s->sin6_port);
            inet_ntop(AF_INET6, &(s->sin6_addr), ipstr, INET6_ADDRSTRLEN);
        }

        std::string ipstrRealStr = std::string(ipstr);
        delete ipstr;
        // return ipstr;
        return ipstrRealStr;
    }

    int GetPeerPort()
    {
        socklen_t len;
        struct sockaddr_storage addr;
        char *ipstr = new char[INET6_ADDRSTRLEN];
        int port;

        len = sizeof addr;
        getpeername(_sockId, (struct sockaddr*)&addr, &len);

        // deal with both IPv4 and IPv6:
        if (addr.ss_family == AF_INET) {
            struct sockaddr_in *s = (struct sockaddr_in *)&addr;
            port = ntohs(s->sin_port);
            inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
        } else { // AF_INET6
            struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
            port = ntohs(s->sin6_port);
            inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
        }
        return port;
    }

    std::string GetPeerFullInfo()
    {
        char *info = new char[INET6_ADDRSTRLEN + 5 + 10];
        sprintf(info, "%d\t%s:%d", GetID(), GetPeerIpAddress().c_str(), GetPeerPort());
        std::string infoStr = std::string(info);
        delete info;
        return infoStr;
    }
};
