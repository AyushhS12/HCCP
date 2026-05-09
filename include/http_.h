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
    Header *headers;
    int header_count;
    Method method;
    char *body;
};

struct Response
{
    char *path;
    char *body;
    int header_count;
    int header_cap;
    Header *headers;
    StatusCode status;
};

struct Header
{
    char *key;
    char *value;
};

// Utility Method for Convertion between string mehtod and Method enum
Method method_parser(char *method);
const char *get_status_text(StatusCode code);

// Setter for Response Headers
int set_header(Response *res, Header header);
// Getter for Request Headers
const Header *get_header_request(const Request *req, const char *key);
// Getter for Response Headers
Header *get_header_response(Response *res, const char *key);

// Parsing Request
const Request *parse_request(char *buffer);
// Serializing Response into string
const char *serialize_response(Response *res);
// Print Response struct
void print_response(const Response *res);

// Freeing Response
void free_response(Response *res);

// Freeing Request
void free_request(const Request *req);

// Sending Response
int send_text_response(Response *res, const char *data);

#endif