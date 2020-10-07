#pragma once

#include <string>
#include <arpa/inet.h> // for inet_pton
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "Secrets.h"

class SocketFactory
{
    std::string _ipAddr;
    int _port;

    struct sockaddr_in GetAddressForServer()
    {
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(_port);
        return address;
    }

public:
    void SetConfig(std::string ipAddr, int port = PORT)
    {
        _ipAddr = ipAddr;
        _port = port;
    }

    // return socket id
    int BuildClientSocket()
    {
        int sockId;

        // create a sock
        sockId = socket(AF_INET, SOCK_STREAM, 0);
        if (sockId < 0)
        {
            printf("\n Socket failed \n");
            return -1;
        }

        // config
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(this->_port);

        // Convert IPv4 and IPv6 addresses from text to binary form
        if (inet_pton(AF_INET, this->_ipAddr.c_str(), &serv_addr.sin_addr) <= 0)
        {
            printf("\nInvalid address\n");
            return -1;
        }

        if (connect(sockId, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\nConnection Failed\n");
            return -1;
        }

        return sockId;
    }

    int BuildListener()
    {
        int listenerId;
        struct sockaddr_in address;
        int opt = 1;

        // Create socket file descriptor
        listenerId = socket(AF_INET, SOCK_STREAM, 0);
        if (listenerId == 0)
        {
            printf("socket() failed\n");
            return -1;
        }

        // set config
        if (setsockopt(listenerId, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        {
            printf("setsockopt() failed\n");
            return -1;
        }

        // config
        address = GetAddressForServer();

        // attach socket to the port
        if (bind(listenerId, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            printf("bind() failed on port %d\n", _port);
            return -1;
        }

        // build listening queue
        if (listen(listenerId, 3) < 0)
        {
            printf("listen() failed\n");
            return -1;
        }

        return listenerId;
    }

    int BuildServerSocket(int listenerId)
    {
        int socketId;
        struct sockaddr_in address = GetAddressForServer();
        socklen_t addrLen = sizeof(address);

        // accept one request from the listening queue
        socketId = accept(listenerId, (struct sockaddr *)&address, &addrLen);
        if (socketId < 0)
        {
            printf("accept() failed\n");
            return -1;
        }

        return socketId;
    }
};
