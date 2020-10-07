#pragma once

// accept received message and take response

#include <iostream>
#include <signal.h>
#include <thread>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "SocketMng.h"
#include "LinkedList.h"
#include "HttpParser.h"
#include "MessagePrinter.h"
#include "SocketRole.h"
#include <fstream>
#include <streambuf>
#include "PathRouter.h"
#include "Secrets.h"

#include <unistd.h>
#include <limits.h>

class Responser
{
    LinkedList<SocketMng> *_connections;
    pthread_cond_t *_toRespondEvent;
    SocketMng *_source;
    MessagePrinter *_printer;

public:
    Responser(SocketMng *source, LinkedList<SocketMng> *connections, pthread_cond_t *toRespondEvent, pthread_mutex_t *toRespondEventMutex, bool *ShouldHalt)
    {
        _source = source;
        _connections = connections;
        _toRespondEvent = toRespondEvent;
        _printer = new MessagePrinter();

        std::thread newThread(RespondThread, _toRespondEvent, toRespondEventMutex, _source, _connections, _printer, ShouldHalt);
        newThread.detach();
    }
    
private:
    static void RespondThread(pthread_cond_t *toRespondEvent, pthread_mutex_t *toRespondEventMutex, SocketMng *source, LinkedList<SocketMng> *connections, MessagePrinter *printer, bool *ShouldHalt)
    {
        pthread_mutex_lock(toRespondEventMutex);
        while (!*ShouldHalt)
        {
            pthread_cond_wait(toRespondEvent, toRespondEventMutex);

            HttpRequest request = source->GetRequestStruct();

            while (!request.IsEmpty)
            {
                printer->Print(source->GetRole(), source->GetID(), request);  // print the request

                if (source->GetRole() == Server)  // respond to the client and print the response
                {
                    time_t now;
                    char *time_str;
                    int peerId;
                    SocketMng *peer;
                    LinkedListNode<SocketMng> *connection;  // list of connected peers
                    std::string idInfo;

                    // map original path to real path
                    std::string realPath = PathRouter::Route(request.Path);

                    HttpResponse response;

                    if (request.Method == GET)
                    {
                        // read
                        std::ifstream fStream(std::string(".") + realPath);

                        // state
                        if (fStream.bad() 
                            || realPath.rfind('/') == realPath.length() - 1)
                        {
                            response.StateCode = "404 Not Found";
                        }
                        else
                        {
                            response.StateCode = "200 OK";

                            // content type
                            int dotPos = realPath.rfind('.');
                            std::string postfix;
                            if (dotPos >= 0 && dotPos + 1 < realPath.length() && realPath.rfind('/') < dotPos)
                            {
                                postfix = realPath.substr(dotPos + 1);
                                if (postfix.compare("html") == 0)
                                    response.CnttType = text_html;
                                else if (postfix.compare("txt") == 0)
                                    response.CnttType = text_plain;
                                else if (postfix.compare("jpg") == 0)
                                    response.CnttType = image_jpeg;
                                else
                                    response.CnttType = application_octet_stream;
                            }
                            else
                            {
                                response.CnttType = application_octet_stream;
                            }

                            // content
                            std::string fullContent((std::istreambuf_iterator<char>(fStream)),
                                                std::istreambuf_iterator<char>());
                            response.Content = fullContent;
                            response.ContentLength = response.Content.length();
                        }
                    }
                    else if (request.Method == POST)
                    {
                        if (realPath == "/html/dopost")
                        {
                            LoginForm form = HttpParser::DeserializeLoginForm(request.Content);
                            std::ifstream fStream;

                            if (form.Login == LOGIN_USERNAME && form.Pass == LOGIN_PASSWORD)
                            {
                                fStream.open("./html/succeed.html");
                            }
                            else
                            {
                                fStream.open("./html/fail.html");
                            }
                            response.StateCode = "200 OK";
                            response.CnttType = text_html;
                            // content
                            std::string fullContent((std::istreambuf_iterator<char>(fStream)),
                                                std::istreambuf_iterator<char>());
                            response.Content = fullContent;
                            response.ContentLength = response.Content.length();
                        }
                        else
                        {
                            response.StateCode = "404 Not Found";
                        }
                    }
                    else
                    {
                        response.StateCode = "404 Not Found";
                    }
                    if (response.StateCode == "404 Not Found")
                    {
                        response.CnttType = text_plain;
                        response.Content = "404 not found";
                        response.ContentLength = response.Content.length();
                    }

                    response.IsEmpty = false;
                    source->Send(response);

                    if (response.StateCode == "404 Not Found")
                    {
                        source->Shutdown();
                    }
                }

                request = source->GetRequestStruct();
            }
        }
        connections->Delete(source);
        source->Shutdown();
        delete source;

        pthread_mutex_unlock(toRespondEventMutex);
    }
};
