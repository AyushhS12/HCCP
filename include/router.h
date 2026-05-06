#ifndef ROUTER_H
#define ROUTER_H
#include <http_.h>

typedef struct Router Router;
typedef struct Route Route;

typedef void (*Handler)(Request *req, Response *res);

Router *new_router();

int route(Router *router, char *path, Handler handler);

int match_path(Router *router,Request *req,Response *res);
void print_path(Router *router);

int listen_and_serve(Router *router);

#endif