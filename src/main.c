#include <stdio.h>
#include <server.h>
#include <router.h>

void handle_app(Request *req, Response *res);
int main()
{
    Router *router = new_router();
    route(router, "/app", handle_app);
    route(router, "/app1", handle_app);
    route(router, "/app2", handle_app);
    int res = listen_and_serve(router);
    printf("\nServer Stopped\n");
    return 0;
}

void handle_app(Request *req, Response *res)
{
    char *path = req->path;
    printf("\nHandler Called from path : %s\n",path);
    return;
}