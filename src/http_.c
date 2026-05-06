#include <http_.h>

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

const char *get_status_text(StatusCode code){
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