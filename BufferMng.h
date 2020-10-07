#pragma once

#include <stdio.h>
#include <pthread.h>
#include <string>

#define MAX_BUFFER_LEN (1024 * 4)

// Manager with Queue
class BufferMng
{
    char _buffer[MAX_BUFFER_LEN];  // queue
    pthread_mutex_t *_bufferLock = new pthread_mutex_t();
    int _front = 0;
    int _tail = (-1 + MAX_BUFFER_LEN) % MAX_BUFFER_LEN;

    int GetBufferLen()
    {
        int end = (_tail + 1) % MAX_BUFFER_LEN;
        if (end < _front)
        {
            end += MAX_BUFFER_LEN;
        }
        return end - _front;
    }

    bool IsCapable(int newItemLen)
    {
        int bufferLen = GetBufferLen();
        if (newItemLen < MAX_BUFFER_LEN - bufferLen)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

public:
    BufferMng()
    {
        pthread_mutex_init(_bufferLock, NULL);
    }

    // drop entire buffer if _buffer is full
    void NewBuffer(const char buffer[], int len)
    {
        pthread_mutex_lock(_bufferLock);
        if (IsCapable(len))  // including the new length prefix
        {
            int i;
            for (i = 0; i < len; i++)
            {
                _tail = (_tail + 1) % MAX_BUFFER_LEN;
                _buffer[_tail] = buffer[i];
            }
        }
        else
        {
            printf("Message dropped due to limited buffer size\n");
        }
        pthread_mutex_unlock(_bufferLock);
    }

    // pop all buffer out
    std::string DequeueBuffer()
    {
        pthread_mutex_lock(_bufferLock);
        std::string message;
        if (GetBufferLen())  // if there is message in buffer
        {
            int i;
            for (i = GetBufferLen(); i > 0; i--)
            {
                message += _buffer[_front];
                _front = (_front + 1) % MAX_BUFFER_LEN;
            }
        }
        pthread_mutex_unlock(_bufferLock);
        return message;
    }
};
