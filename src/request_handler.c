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

        char *body = strstr(buffer, "\r\n\r\n");
        if (body != NULL)
        {
            *body = '\0';
            body += 4;
            printf("Body: %s\n", body);
        }
        char *token = strtok(buffer, "\r\n");
        if (token == NULL)
        {
            closesocket(client_fd);
            continue;
        }
        char *request_line = malloc(strlen(token) + 1);
        strcpy(request_line, token);

        char *m = request_line;
        char *path = NULL;
        char *version = NULL;

        for (int i = 0; request_line[i] != '\0'; i++)
        {
            if (request_line[i] == ' ')
            {
                request_line[i] = '\0';

                if (path == NULL)
                {
                    path = &request_line[i + 1];
                }
                else if (version == NULL)
                {
                    version = &request_line[i + 1];
                }
            }
        }
        Method method = method_parser(m);
        printf("%s: %d\n", m, method);
        printf("Request Method: %s\n", m);
        printf("Request Path: %s\n", path);
        printf("Request Version: %s\n", version);
        int count = 0;
        int capacity = 8;
        Header *headers = malloc(sizeof(Header) * capacity);
        char *content_type = NULL;
        token = strtok(NULL, "\r\n");
        while (token != NULL)
        {
            // printf("--%s--\n", token);
            if (count == capacity)
            {
                capacity *= 2;
                headers = realloc(headers, sizeof(Header) * capacity);
            }
            char *colon = strchr(token, ':');
            if (colon)
            {
                *colon = '\0';
                char *key = token;
                char *value = colon + 1;
                if (strcmp(key, "Content-Type") == 0)
                {
                    content_type = value;
                }
                while (*value == ' ' || *value == '\t')
                    value++;
                Header header = {.key = key, .value = value};
                headers[count] = header;
                count++;
            }
            token = strtok(NULL, "\r\n");
        }
        if (strstr(content_type, "application/json"))
        {
            printf("\n");
            for (int i = 0; i < strlen(body); i++)
            {
                if (body[i] == '\r')
                {
                    printf("\\r");
                }
                else if (body[i] == '\n')
                {
                    printf("\\n");
                }
                else if (body[i] == '\0')
                {
                    printf("\\0");
                }
                else if (body[i] == ' ')
                {
                    printf("_");
                }
                else
                {
                    printf("%c", body[i]);
                }
            }
            printf("\n");
        }
        else
        {
            printf("Content-Type is not Json");
        }
        Request request = {
          .headers = headers,
          .method = method,
          .path = path,
          .header_count = count,
        };
        if(body[0] == '\0'){
            request.body = NULL;
        }
        request.body = body;
        Response response = {
            .path = path,
        };
        if(!match_path(queue->router,&request,&response)){
            exit(-1);
        }
        char *res =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 13\r\n"
            "Connection: close\r\n"
            "\r\n"
            "Hello, world!";
        send(client_fd, res, strlen(res), 0);
        free(request_line);
        free(headers);
        closesocket(client_fd);
    }
}
