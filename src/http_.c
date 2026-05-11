#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <request_handler.h>
#include <json.h>
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
Method method_parser(char *method)
{
    if (strcmp(method, "GET") == 0)
        return METHOD_GET;
    if (strcmp(method, "POST") == 0)
        return METHOD_POST;
    if (strcmp(method, "DELETE") == 0)
        return METHOD_DELETE;
    if (strcmp(method, "PUT") == 0)
        return METHOD_PUT;
    if (strcmp(method, "PATCH") == 0)
        return METHOD_PATCH;
    if (strcmp(method, "OPTIONS") == 0)
        return METHOD_OPTIONS;
    return METHOD_UNKNOWN;
}

/*
enum StatusCode
{
    // 2xx Success
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NO_CONTENT = 204,

    // 3xx Redirection
    MOVED_PERMANENTLY = 301,
    FOUND = 302,
    NOT_MODIFIED = 304,

    // 4xx Client Errors
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    CONFLICT = 409,
    UNSUPPORTED_MEDIA_TYPE = 415,
    TOO_MANY_REQUESTS = 429,

    // 5xx Server Errors
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    BAD_GATEWAY = 502,
    SERVICE_UNAVAILABLE = 503
};
*/

const char *get_status_text(StatusCode code)
{
    switch (code)
    {
    // 2xx Success Codes
    case OK:
        return "OK";
    case CREATED:
        return "Created";
    case ACCEPTED:
        return "Accepted";
    case NO_CONTENT:
        return "No Content";

    // 3xx Redirection Codes
    case MOVED_PERMANENTLY:
        return "Moved Permanently";
    case FOUND:
        return "Found";
    case NOT_MODIFIED:
        return "Not Modified";

    // 4xx Client Error Codes
    case BAD_REQUEST:
        return "Bad Request";
    case UNAUTHORIZED:
        return "Unauthorized";
    case FORBIDDEN:
        return "Forbidden";
    case NOT_FOUND:
        return "Not Found";
    case METHOD_NOT_ALLOWED:
        return "Method Not Allowed";
    case CONFLICT:
        return "Conflict";
    case UNSUPPORTED_MEDIA_TYPE:
        return "Unsupported Media Type";
    case TOO_MANY_REQUESTS:
        return "Too Many Requests";

    // 5xx Server Error Codes
    case INTERNAL_SERVER_ERROR:
        return "Internal Server Error";
    case NOT_IMPLEMENTED:
        return "Not Implemented";
    case BAD_GATEWAY:
        return "Bad Gateway";
    case SERVICE_UNAVAILABLE:
        return "Service Unavailable";
    default:
        return "Unknown Status";
    }
}

const Header *get_header_request(const Request *req, const char *key)
{
    for (int i = 0; i < req->header_count; i++)
    {
        if (strcmp(req->headers[i].key, key) == 0)
        {
            return &req->headers[i];
        }
    }
    return NULL;
}

Header *get_header_response(Response *res, const char *key)
{
    for (int i = 0; i < res->header_count; i++)
    {
        if (strcmp(res->headers[i].key, key) == 0)
        {
            return &res->headers[i];
        }
    }
    return NULL;
}

int set_header(Response *res, Header header)
{
    if (!res || !header.key || !header.value)
        return -1;

    // Check if header already exists
    for (int i = 0; i < res->header_count; i++)
    {
        if (strcmp(res->headers[i].key, header.key) == 0)
        {
            free(res->headers[i].value);

            res->headers[i].value = strdup(header.value);

            if (!res->headers[i].value)
                return -1;

            return 0;
        }
    }

    // Resize if needed
    if (res->header_count >= res->header_cap)
    {
        int new_cap = (res->header_cap == 0) ? 2 : res->header_cap * 2;

        Header *new_headers = realloc(
            res->headers,
            sizeof(Header) * new_cap);

        if (!new_headers)
            return -1;

        res->headers = new_headers;
        res->header_cap = new_cap;
    }

    // Add new header
    res->headers[res->header_count].key = strdup(header.key);
    res->headers[res->header_count].value = strdup(header.value);

    if (!res->headers[res->header_count].key ||
        !res->headers[res->header_count].value)
    {
        return -1;
    }

    res->header_count++;

    return 0;
}

const Request *parse_request(char *buffer)
{
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
        return NULL;
    }

    char *request_line = malloc(strlen(token) + 1);
    if (!request_line)
        return NULL;
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
    if (!headers)
        return NULL;
    char *content_type = NULL;
    token = strtok(NULL, "\r\n");
    while (token != NULL)
    {
        // printf("--%s--\n", token);
        if (count == capacity)
        {
            int new_cap = capacity * 2;
            Header *temp = realloc(headers, sizeof(Header) * new_cap);
            if (!temp)
            {
                free(headers);
                free(request_line);
                return NULL;
            }
            headers = temp;
            capacity = new_cap;
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
    if (content_type && strstr(content_type, "application/json"))
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

    Request *request = malloc(sizeof(Request));
    if (!request)
    {
        free(headers);
        free(request_line);
        return NULL;
    }
    if (body == NULL || body[0] == '\0')
    {
        request->body = NULL;
    }
    else
    {
        request->body = body;
    }
    request->headers = headers;
    request->header_count = count;
    request->method = method;
    request->path = strdup(path);
    if (!request->path)
    {
        free(headers);
        free(request);
        return NULL;
    }
    free(request_line);

    return request;
}

const char *serialize_response(Response *res)
{
    const char *status = get_status_text(res->status);
    int body_len = res->body ? strlen(res->body) : 0;
    int total = snprintf(NULL, 0, "HTTP/1.1 %d %s\r\n", res->status, status);

    for (int i = 0; i < res->header_count; i++)
    {
        total += strlen(res->headers[i].key);
        total += strlen(res->headers[i].value);
        total += 4; // For ": " and "\r\n" => total 4 extra characters
    }
    total += 2;        // for the new line "\r\n" before body
    total += body_len; // adding body len
    total += 1;        // For null terminator

    char *buffer = malloc(total);
    if (!buffer)
        return NULL;

    int written = sprintf(buffer, "HTTP/1.1 %d %s\r\n", res->status, status);

    for (int i = 0; i < res->header_count; i++)
    {
        written += sprintf(buffer + written, "%s: %s\r\n", res->headers[i].key, res->headers[i].value);
    }

    written += sprintf(buffer + written, "\r\n");

    if (res->body)
    {
        memcpy(buffer + written, res->body, body_len);
        written += body_len;
    }
    buffer[written] = '\0';
    return buffer;
}

void print_response(const Response *res)
{
    if (res == NULL)
    {
        printf("Response is NULL\n");
        return;
    }

    printf("-------- RESPONSE --------\n");

    printf("Path: %s\n",
           res->path ? res->path : "NULL");

    printf("Status: %d\n", res->status);

    printf("Body: %s\n",
           res->body ? res->body : "NULL");

    printf("Headers (%d):\n", res->header_count);

    for (int i = 0; i < res->header_count; i++)
    {
        printf("  %s: %s\n",
               res->headers[i].key ? res->headers[i].key : "NULL",
               res->headers[i].value ? res->headers[i].value : "NULL");
    }

    printf("--------------------------\n");
}

void free_request(const Request *req)
{
    if (!req)
        return;
    free(req->headers);
    free(req->path);
    free(req);
}

void free_response(Response *res)
{
    if (!res)
        return;

    // Free each header's key/value
    for (int i = 0; i < res->header_count; i++)
    {
        free(res->headers[i].key);
        free(res->headers[i].value);
    }

    // Free headers array
    free(res->headers);

    // Free body if dynamically allocated
    free(res->body);

    // Free path ONLY if response owns it
    free(res->path);

    free(res);
}

int send_text_response(Response *res, const char *data)
{
    if (!res || !data)
    {
        return -1;
    }

    res->body = strdup(data);
    if (!res->body)
        return -1;

    if (set_header(res, (Header){.key = "Content-Type", .value = "text/plain"}) == -1)
    {
        printf("\nSet Header failed, Content-type\n");
        return -1;
    };
    int len = strlen(res->body);
    char length[20];
    sprintf(length, "%d", len);
    if (set_header(res, (Header){.key = "Content-Length", .value = strdup(length)}) == -1)
    {
        printf("\nSet Header failed, Content-Length\n");
        return -1;
    };
};

int send_json_response(Response *res, Json *json){

}