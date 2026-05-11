#include <stdio.h>
#include <stdlib.h>
#include <server.h>
#include <router.h>
#include <json.h>

void handle_app(const Request *req, Response *res);
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

void handle_app(const Request *req, Response *res)
{
    printf("\nHandler Called from path : %s\n",req->path);
    res->status = OK;
    // Parser parser = create_parser(req->body);
    Parser parser = create_parser(req->body);
    printf("\n-----------Parser Created------------\n");
    Object *obj = parse_json_for_real(&parser);
    print_object(obj,1);
    printf("\n--------------------Json Parsed--------------------\n");
    send_text_response(res, "{\"message\":\"Hello World\"}");
    // free(parser.input);
    return;
}