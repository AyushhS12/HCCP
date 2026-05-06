#include <http_.h>
#include <stdio.h>
#include <router.h>
#include <string.h>
#include <stdlib.h>

struct Route
{
    char *path;
    Handler handler;
};

struct Router
{
    Route *routes;
    int count;
    int capacity;
    int (*listen_and_serve)(Router *router);
};

Router *new_router()
{
    Router *router = malloc(sizeof(Router));
    if (!router)
        return NULL;

    router->capacity = 4;
    router->count = 0;
    router->routes = malloc(sizeof(Route) * router->capacity);
    router->listen_and_serve = listen_and_serve;
    return router;
}

int route(Router *router, char *path, Handler handler)
{
    if (!router || !path || !strchr(path, '/'))
    {
        return -1;
    }
    if (router->count == router->capacity)
    {
        int new_cap = router->capacity * 2;
        Route *new_routes = realloc(router->routes, sizeof(Route) * new_cap);
        if (!new_routes)
        {
            return -1;
        }
        router->routes = new_routes;
        router->capacity = new_cap;
    }
    if (router->count < router->capacity)
    {
        router->routes[router->count] = (Route){.path = strdup(path), .handler = handler};
        router->count++;
    }
}

int match_path(Router *router, Request *req, Response *res)
{
    printf("\nreq.path: %s\n",req->path);
    for (size_t i = 0; i <= router->count; i++)
    {
        if (strcmp(router->routes[i].path, req->path) == 0)
        {
            printf("\nRequest Arrived at %s\n",router->routes[i].path);
            router->routes[i].handler(req,res);
            return 1;
        }
    }
    return -1;
}

void print_path(Router *router){
    for (int i = 0; i < router->count; i++)
    {
        printf("\nRoute: %s\n",router->routes[i].path);
    }
}