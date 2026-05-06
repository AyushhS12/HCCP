#include <string.h>
#include <stdint.h>
#include <ctype.h>

typedef struct Parser Parser;
typedef enum Type Type;
typedef union Data Data;
typedef struct Object Object;
typedef struct Array Array;
typedef struct Entry Entry;

struct Parser
{
    char *input;
    int pos;
    int end;
};

enum Type
{
    INTEGER,
    FLOAT,
    STRING,
    CHAR,
    BOOLEAN,
    TYPE_NULL,
    OBJECT,
    ARRAY,
};

union Data
{
    int64_t in;
    double fl;
    char ch;
    char *string;
    int bo;
    Array *array;
    Object *obj;
};

struct Entry
{
    char *key;
    Type type;
    Data data;
};

struct Object
{
    Entry *entries;
    int count;
    int capacity;
};

struct Array
{
    Data *items;
    Type *types;
    int count;
    int capacity;
};

Parser create_parser(char *input)
{
    return (Parser){
        .input = strdup(input),
        .pos = 0,
        .end = strlen(input),
    };
}

void parse_json(Parser *parser){
    while(parser->input[parser->pos] != '\0'){
        char current = parser->input[parser->pos];
        if(current == '\r' || current == '\n' || current == '{' || current == ',' || current == '"') 
        {
            parser->pos++;
            continue;
        }
        char *key = &parser->input[parser->pos];
        
    }
}