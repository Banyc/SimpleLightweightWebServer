// linked-list

#pragma once

#include <stdio.h>
#include <pthread.h>
#include "SocketMng.h"

template <typename T>
class LinkedListNode
{
public:
    T *Item;
    LinkedListNode *Next;
};

template <typename T>
class LinkedList
{
    LinkedListNode<T> *_head = NULL;
    LinkedListNode<T> *_tail = NULL;
    int _count = 0;
    pthread_mutex_t *_lock = new pthread_mutex_t();

public:
    LinkedList()
    {
        pthread_mutex_init(_lock, NULL);
    }

    void Append(T *elm)
    {
        pthread_mutex_lock(_lock);
        LinkedListNode<T> *newNode = new LinkedListNode<T>();
        newNode->Item = elm;
        newNode->Next = NULL;

        if (_head)
        {
            _tail->Next = newNode;
        }
        else
        {
            _head = newNode;
        }
        _tail = newNode;

        _count++;
        pthread_mutex_unlock(_lock);
    }

    void Delete(T *elm)
    {
        pthread_mutex_lock(_lock);
        LinkedListNode<T> *node = _head;
        LinkedListNode<T> *preNode = NULL;

        while (node)
        {
            if (node->Item == elm)
            {
                LinkedListNode<T> *tmpNode;
                // bridge up the neighbours
                if (preNode)
                {
                    preNode->Next = node->Next;
                }
                else
                {
                    _head = node->Next;
                }
                // update `_tail`
                if (node == _tail)
                {
                    _tail = preNode;
                }

                delete node;

                _count--;

                break;
            }

            preNode = node;
            node = node->Next;
        }
        pthread_mutex_unlock(_lock);
    }

    LinkedListNode<T> *GetHeadItem()
    {
        return _head;
    }

    int Count()
    {
        return _count;
    }

    // WORKAROUND CAUTION: don't casually run this method unless `T` is `SocketMng`
    SocketMng *FindSocketMng(int sockId)
    {
        pthread_mutex_lock(_lock);
        LinkedListNode<T> *node = _head;
        SocketMng *result = NULL;
        while (node)
        {
            if (((SocketMng *)node->Item)->GetID() == sockId)
            {
                result = (SocketMng *)node->Item;
                break;
            }
            node = node->Next;
        }
        pthread_mutex_unlock(_lock);
        return result;
    }

    // Print all socket id
    // WORKAROUND CAUTION: don't casually run this method unless `T` is `SocketMng`
    void PrintAsSocketMng()
    {
        pthread_mutex_lock(_lock);
        LinkedListNode<T> *node = _head;
        while (node)
        {
            printf("\n");
            printf("%d\n", ((SocketMng *) node->Item)->GetID());
            printf("\n");
            node = node->Next;
        }
        pthread_mutex_unlock(_lock);
    }
};
