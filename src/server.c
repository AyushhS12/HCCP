#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <request_handler.h>
#include <stdlib.h>
#include <http_.h>
#include <router.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

static char *remove_quotes(char *str)
{
    if (str == NULL)
        return NULL;

    int len = strlen(str);

    if (len >= 2 && str[0] == '"' && str[len - 1] == '"')
    {
        str[len - 1] = '\0';
        return str + 1;
    }

    return str;
}

int listen_and_serve(Router *router)
{
    // Parser parser = create_parser("Hello there my friend");
    // print_parser(&parser);
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    int res = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res < 0)
    {
        return 1;
    }
    listen(server_fd, 6);
    SocketQueue queue;
    queue.router = router;
    init_queue(&queue);
    printf("Server Started on port 8080\n");
    for (int i = 0; i < 4; i++)
    {
        CreateThread(NULL, 0, worker, &queue, 0, NULL);
    }
    for (;;)
    {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == INVALID_SOCKET)
        {
            printf("\nInternal Error, Couldn't Receive Data from Client\n");
            continue;
        }
        queue.enqueue(&queue, client_fd);
    }
    closesocket(server_fd);
    WSACleanup();
    return 0;
}
