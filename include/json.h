#ifndef JSON_H
#define JSON_H

typedef struct Parser
{
    char *input;
    int pos;
} Parser;
typedef enum Type Type;
typedef union Data Data;
typedef struct Value Value;
typedef struct Object Object;
typedef struct Array Array;
typedef struct Entry Entry;

typedef struct Json Json;

Parser create_parser(char *input);

Object *parse_json(Parser *parser);
Object *parse_json_for_real(Parser *parser);

// Create a Json Array
Array *create_array();
// Push into a Json Array
int array_push(Array *arr, Data data, Type type);

// Create a Json object
Object *create_object();
// Insert into a Json Object
int object_insert(Object *obj, Entry entry);

void print_object(Object *obj, int indent);
// Create a new key value pair Entry
Entry create_entry(char *key, Data data, Type type);

#endif