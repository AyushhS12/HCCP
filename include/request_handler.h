#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <router.h>

#define WINVER 0x0600
#define _WIN32_WINNT 0x0600

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

typedef struct SocketQueue SocketQueue;
struct SocketQueue
{
    SOCKET sockets[20];
    int front;
    int rear;
    Router *router;
    CRITICAL_SECTION lock;
    CONDITION_VARIABLE cond;

    void (*enqueue)(SocketQueue *queue, SOCKET s);
    SOCKET (*dequeue)(SocketQueue *queue);
    int (*is_empty)(SocketQueue *queue);
};

void init_queue(SocketQueue *queue);

int is_empty(SocketQueue *queue);

void enqueue(SocketQueue *q, SOCKET s);

SOCKET dequeue(SocketQueue *q);

// Worker Functions
DWORD WINAPI worker(void *args);

#endif