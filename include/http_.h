#ifndef HTTP_TYPES_H
#define HTTP_TYPES_H

typedef enum Method Method;
typedef enum StatusCode StatusCode;
typedef struct Header Header;
typedef struct Request Request;
typedef struct Response Response;

enum Method
{
    METHOD_GET,
    METHOD_POST,
    METHOD_OPTIONS,
    METHOD_DELETE,
    METHOD_PUT,
    METHOD_PATCH,
    METHOD_UNKNOWN
};

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

struct Request
{
    char *path;
    int header_count;
    Method method;
    char *body;
    Header *headers;
    Header (*get_header)(Request *req, char *key);
};

struct Response
{
    char *path;
    char *body;
    int header_count;
    int header_cap;
    int code;
    StatusCode status;
    Header *headers;
    Header (*get_header)(Request *req, char *key);
};

struct Header
{
    char *key;
    char *value;
};

Method method_parser(char *method);
const char *get_status_text(StatusCode code);

#endif