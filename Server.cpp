#include <iostream>
#include <string>
#include <stdio.h>
#include <pthread.h>
#include "SocketFactory.h"
#include "LinkedList.h"
#include "SocketMng.h"
#include "SocketRole.h"
#include "Responser.h"
#include <string.h>
#include "Secrets.h"

#define SHOULD_ACCEPT 1

LinkedList<SocketMng> *_sockList = new LinkedList<SocketMng>();


void PrintSocketList()
{
    LinkedListNode<SocketMng> *connection;
    connection = _sockList->GetHeadItem();
    while (connection)
    {
        std::cout << connection->Item->GetPeerFullInfo() << '\n';
        connection = connection->Next;
    }
}

int main()
{
    int port = PORT;
    SocketFactory *factory = new SocketFactory();
    int listenerId;
    int sockId;

    factory->SetConfig(std::string("0.0.0.0"), port);
    listenerId = factory->BuildListener();
    while (true)
    {
        if (listenerId >= 0)
        {
            printf("Avaliable on port %d\n", port);
            while (!SHOULD_ACCEPT)
                ;
            sockId = factory->BuildServerSocket(listenerId);
            if (sockId >= 0)
            {
                pthread_cond_t *toRespondEvent = new pthread_cond_t();
                pthread_mutex_t *toRespondEventMutex = new pthread_mutex_t();
                pthread_cond_init(toRespondEvent, NULL);
                pthread_mutex_init(toRespondEventMutex, NULL);
                SocketMng *sock = new SocketMng(sockId, Server, toRespondEvent, toRespondEventMutex);
                bool *ShouldHalt = sock->GetShutdownBoolPointer();  // let responser knows if the socket is shut
                _sockList->Append(sock);
                Responser *responser = new Responser(sock, _sockList, toRespondEvent, toRespondEventMutex, ShouldHalt);
                // print log
                time_t now;
                char *time_str;
                now = time(0);  // seconds from 1970.1.1
                time_str = std::ctime(&now);  // seconds to human-readable
                int len = strlen(time_str);
                time_str[len - 1] = '\0';  // replace '\n' to '\0'
                printf("[%s] new connection from %s:%d\n", time_str, sock->GetPeerIpAddress().c_str(), sock->GetPeerPort());
                // print all connections
                PrintSocketList();
                sock->StartReceivingLoop();
            }
        }
        else  // this port seems to have been occupied
        {
            port++;
            if (port >= 65535)
            {
                port = 1024;
            }
            factory->SetConfig(std::string("0.0.0.0"), port);
            listenerId = factory->BuildListener();
        }
    }
}
