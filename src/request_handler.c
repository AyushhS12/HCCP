#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <request_handler.h>
#include <http_.h>
#ifdef _WIN32
#define WINVER 0x0600
#define _WIN32_WINNT 0x0600

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

// Response *handle_request(Router *router, Request *req);

int is_empty(SocketQueue *queue)
{
    if (queue->rear == -1)
        return 0;
    return -1;
}

void enqueue(SocketQueue *q, SOCKET s)
{
    EnterCriticalSection(&q->lock);

    q->rear++;
    q->sockets[q->rear] = s;

    WakeConditionVariable(&q->cond);

    LeaveCriticalSection(&q->lock);
}

SOCKET dequeue(SocketQueue *q)
{
    EnterCriticalSection(&q->lock);

    while (q->front > q->rear)
    {
        SleepConditionVariableCS(
            &q->cond,
            &q->lock,
            INFINITE);
    }

    SOCKET client = q->sockets[q->front];
    q->front++;

    LeaveCriticalSection(&q->lock);

    return client;
}

void init_queue(SocketQueue *queue)
{
    queue->front = 0;
    queue->rear = -1;

    queue->enqueue = enqueue;
    queue->dequeue = dequeue;
    queue->is_empty = is_empty;

    InitializeCriticalSection(&queue->lock);
    InitializeConditionVariable(&queue->cond);
}

DWORD WINAPI worker(void *args)
{
    SocketQueue *queue = (SocketQueue *)args;
    char buffer[1024];
    for (;;)
    {
        SOCKET client_fd = queue->dequeue(queue);
        int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0)
        {
            closesocket(client_fd);
            continue;
        }
        buffer[bytes] = '\0';

        const Request *request = parse_request(buffer);
        printf("\nFinished parsing req\n");
        Response response = {
            .header_cap = 4,
            .path = request->path,
            .headers = malloc(sizeof(Header) * 4),
        };
        set_header(&response, (Header){.key = "Connection", .value = "close"});
        if (!request)
        {
            closesocket(client_fd);
            continue;
        }
        if (match_path(queue->router, request, &response) == -1)
        {
            printf("\nNo path matched\n");
            response.status = NOT_FOUND;
            const char *res = serialize_response(&response);
            printf("\nNot Found request sent %s\n", res);
            // printf("\nNot Found request sent %s\n",res);
            send(client_fd, res, strlen(res), 0);
            closesocket(client_fd);
            continue;
        };
        if (response.body == NULL || response.body[0] == '\0')
        {
            printf("\nEmpty Response Body\n");
        }
        const char *res = serialize_response(&response);
        print_response(&response);
        printf("\nFinished serializing response\n");
        send(client_fd, res, strlen(res), 0);
        free_request(request);
        free_response(&response);
        closesocket(client_fd);
    }
}
