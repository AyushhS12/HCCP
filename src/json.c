#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <json.h>

#define PARSE(x) parse_json_for_real(x)

char *parse_string_value(Parser *parser);
Array *parse_array_value(Parser *parser);

void check_type_and_assign_value(Array *arr, Parser *parser);
void check_type_and_assign_value_for_object(Object *obj, char *key, Parser *parser);

void free_object(Object *obj);
void free_array(Array *arr);

void print_array(Array *arr, int indent);
void print_object(Object *obj, int indent);

enum Type
{
    INTEGER,
    FLOAT,
    STRING,
    BOOLEAN,
    TYPE_NULL,
    OBJECT,
    ARRAY,
};

union Data
{
    int64_t in;
    double fl;
    char *string;
    int bool;
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

struct Value
{
    Type type;
    Data item;
};

struct Array
{
    Value *items;
    int count;
    int capacity;
};

Parser create_parser(char *input)
{
    return (Parser){
        .input = strdup(input),
        .pos = 0,
    };
}

Array *create_array()
{
    Array *arr = malloc(sizeof(Array));
    if (!arr)
        return NULL;

    arr->capacity = 4;
    arr->count = 0;

    arr->items = malloc(sizeof(Value) * arr->capacity);
    if (!arr->items)
    {
        free(arr);
        return NULL;
    }

    return arr;
}

Object *create_object()
{
    Object *obj = malloc(sizeof(Object));
    if (!obj)
        return NULL;
    obj->entries = malloc(sizeof(Entry) * 4);
    if (!obj->entries)
    {
        free(obj);
        return NULL;
    }
    obj->capacity = 4;
    obj->count = 0;
    return obj;
}
// OLD API
Object *parse_json(Parser *parser)
{
    Object *obj = create_object();
    printf("\nParser Position: %d\n", parser->pos);
    if (parser->input[parser->pos] == '{')
    {
        parser->pos++;
    }
    while (parser->input[parser->pos] != '\0')
    {
        if (parser->input[parser->pos] == '}')
        {
            parser->pos++;
            return obj;
        }
        char current = parser->input[parser->pos];
        while (current == '\t' || current == ',' || current == '"' || current == ' ' || current == '\r' || current == '\n')
        {
            parser->pos++;
            current = parser->input[parser->pos];
            continue;
        }
        char *key = &parser->input[parser->pos];
        while (parser->input[parser->pos] != '"')
        {
            parser->pos++;
        }
        parser->input[parser->pos] = '\0';
        // printf("\nKey : %s\n", key);
        parser->pos++;
        char *value;
        if (parser->input[parser->pos] == ':')
        {
            parser->pos++;
            while (parser->input[parser->pos] == '\n' || parser->input[parser->pos] == '\r' || parser->input[parser->pos] == '\t' || parser->input[parser->pos] == ' ')
            {
                parser->pos++;
                continue;
            }

            if (parser->input[parser->pos] == '"')
            {
                parser->pos++;
                value = &parser->input[parser->pos];
                while (parser->input[parser->pos] != '"')
                {
                    parser->pos++;
                }
                parser->input[parser->pos] = '\0';
                parser->pos++;
                // printf("\nValue : %s\n", value);
            }
            else if (parser->input[parser->pos] == '{')
            {
                value = &parser->input[parser->pos];
                Object *inner = parse_json(parser);
                printf("\nAnother object Parsed\n");
            }
        }

        if (parser->input[parser->pos] == '}')
        {
            parser->input[parser->pos] = '\0';
            parser->pos++;
            return obj;
        }
        parser->input[parser->pos] = '\0';
    }
    return obj;
}

void skip_whitespace(Parser *parser)
{
    while (isspace(parser->input[parser->pos]))
    {
        parser->pos++;
    }
}

// NEW PARSING LOGIC
Object *parse_json_for_real(Parser *parser)
{
    if (parser->input[parser->pos] == '{')
    {
        parser->pos++;
    }
    Object *obj = create_object();
    while (parser->input[parser->pos] != '}' && parser->input[parser->pos] != '\0')
    {
        skip_whitespace(parser);
        if (parser->input[parser->pos] == '}')
        {
            parser->pos++;
            break;
        }
        if (parser->input[parser->pos] == ',')
        {
            parser->pos++;
            while (isspace(parser->input[parser->pos]))
            {
                parser->pos++;
            }
        }
        char *key;
        if (parser->input[parser->pos] == '"')
        {
            key = parse_string_value(parser);
            if (!key)
            {
                free_object(obj);
                return NULL;
            }
        }
        if (parser->input[parser->pos] == ':')
        {
            // Parsing value
            if (parser->input[parser->pos] == ',')
            {
                printf("\n------------------Invalid json data------------------\n");
                free_object(obj);
                free(key);
                return NULL;
            }
            parser->pos++;
            skip_whitespace(parser);
            if (parser->input[parser->pos] == '"')
            {
                char *value = parse_string_value(parser);
                if (!value)
                {
                    printf("\nInvalid Value");
                    free(key);
                    free(value);
                    free_object(obj);
                    return NULL;
                }
                Entry entry = create_entry(key, (Data){.string = value}, STRING);
                if (object_insert(obj, entry) < 0)
                {
                    free(entry.key);
                    free(value);
                    free_object(obj);
                    return NULL;
                };
            }
            else if (parser->input[parser->pos] == '{')
            {
                Object *inner = parse_json_for_real(parser);
                if (!inner)
                {
                    free(key);
                    free_object(obj);
                    return NULL;
                }
                Entry entry = create_entry(key, (Data){.obj = inner}, OBJECT);
                if (object_insert(obj, entry) < 0)
                {
                    free(entry.key);
                    free_object(inner);
                    free_object(obj);
                    return NULL;
                };
            }
            else if (parser->input[parser->pos] == '[')
            {
                Array *arr = parse_array_value(parser);
                if (!arr)
                {
                    printf("\nreached Here 1\n");
                    return NULL;
                }
                Entry entry = create_entry(key, (Data){.array = arr}, ARRAY);
                if (object_insert(obj, entry) < 0)
                {
                    free(entry.key);
                    free_array(arr);
                    free_object(obj);
                    return NULL;
                };
            }
            else
            {
                check_type_and_assign_value_for_object(obj, key, parser);
            }
        }
        else
        {
            printf("\n corrupt json \n");
            free_object(obj);
            return NULL;
        }
    }
    if (parser->input[parser->pos] == '}')
    {
        parser->pos++;
    }
    return obj;
}

char *parse_string_value(Parser *parser)
{
    parser->pos++;
    char *start = &parser->input[parser->pos];
    int len = 0;
    while (start[len] != '"' && start[len] != '\0')
    {
        len++;
    }
    if (start[len] == '\0')
        return NULL;
    char *out = malloc(len + 1);
    memcpy(out, start, len);
    out[len] = '\0';
    parser->pos += len + 1;
    return out;
}

Array *parse_array_value(Parser *parser)
{
    Array *arr = create_array();
    parser->pos++;
    while (parser->input[parser->pos] != ']')
    {
        if (parser->input[parser->pos] == '\0')
        {
            free_array(arr);
            printf("\n%s\n", &parser->input[parser->pos]);
            printf("\ninvalid string\n");
            return NULL;
        }
        switch (parser->input[parser->pos])
        {
        case '"':
        {
            char *value = parse_string_value(parser);
            if (!value)
            {
                printf("\ninvalid string value\n");
                free_array(arr);
                return NULL;
            }
            if (array_push(arr, (Data){.string = value}, STRING) < 0)
            {
                printf("\ncannot push string value\n");
                free_array(arr);
                return NULL;
            };
            break;
        }
        case '[':
        {

            Array *new = parse_array_value(parser);
            if (!new)
            {
                printf("\ncannot create new array value\n");
                free_array(arr);
                return NULL;
            }
            if (array_push(arr, (Data){.array = new}, ARRAY) < 0)
            {
                printf("\ncannot push new array value\n");
                free_array(new);
                free_array(arr);
                return NULL;
            };
            break;
        }
        case '{':
        {

            Object *obj = PARSE(parser);
            if (!obj)
            {
                printf("\ncannot create new object value\n");
                free_array(arr);
                return NULL;
            }
            if (array_push(arr, (Data){.obj = obj}, OBJECT) < 0)
            {
                printf("\ncannot push new object value\n");
                free_object(obj);
                free_array(arr);
                return NULL;
            };
            break;
        }
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            while (isspace(parser->input[parser->pos]))
                parser->pos++;
            break;
        case ',':
            parser->pos++;
            while (isspace(parser->input[parser->pos]))
                parser->pos++;
            if (parser->input[parser->pos] == ']')
            {
                printf("\ninvalid array value\n");
                free_array(arr);
                return NULL;
            }
            break;
        default:
            check_type_and_assign_value(arr, parser);
            break;
        }
    }
    if (parser->input[parser->pos] == ']')
    {
        parser->pos++;
        return arr;
    }
    return NULL;
}

void check_type_and_assign_value_for_object(Object *obj, char *key, Parser *parser)
{
    if (!obj || !parser || !key)
        return;

    char *start = &parser->input[parser->pos];

    // move until primitive ends
    while (
        parser->input[parser->pos] != ',' &&
        parser->input[parser->pos] != '}' &&
        parser->input[parser->pos] != '\0' &&
        !isspace(parser->input[parser->pos]))
    {
        parser->pos++;
    }

    char current = parser->input[parser->pos];
    parser->input[parser->pos] = '\0';

    char *value = start;

    // boolean
    if (strcmp(value, "true") == 0)
    {
        Entry entry = create_entry(key, (Data){.bool = 1}, BOOLEAN);
        object_insert(obj, entry);
    }
    else if (strcmp(value, "false") == 0)
    {
        Entry entry = create_entry(key, (Data){.bool = 0}, BOOLEAN);
        object_insert(obj, entry);
    }
    // null
    else if (strcmp(value, "null") == 0)
    {
        Entry entry = create_entry(key, (Data){0}, TYPE_NULL);
        object_insert(obj, entry);
    }
    else
    {
        int has_dot = 0;
        int start_idx = 0;
        int is_number = 1;

        if (value[0] == '-')
        {
            start_idx = 1;
        }

        for (int i = start_idx; value[i] != '\0'; i++)
        {
            if (value[i] == '.')
            {
                if (has_dot)
                {
                    is_number = 0;
                    break;
                }
                has_dot = 1;
            }
            else if (!isdigit((unsigned char)value[i]))
            {
                is_number = 0;
                break;
            }
        }

        if (is_number)
        {
            if (has_dot)
            {
                Entry entry = create_entry(
                    key,
                    (Data){.fl = strtod(value, NULL)},
                    FLOAT);
                object_insert(obj, entry);
            }
            else
            {
                Entry entry = create_entry(
                    key,
                    (Data){.in = strtoll(value, NULL, 10)},
                    INTEGER);
                object_insert(obj, entry);
            }
        }
        else
        {
            printf("Invalid primitive value: %s\n", value);
        }
    }
    // restore delimiter
    parser->input[parser->pos] = current;
}

void check_type_and_assign_value(Array *arr, Parser *parser)
{
    if (!arr || !parser)
        return;

    char *start = &parser->input[parser->pos];

    // move until primitive ends
    while (
        parser->input[parser->pos] != ',' &&
        parser->input[parser->pos] != ']' &&
        parser->input[parser->pos] != '\0' &&
        !isspace(parser->input[parser->pos]))
    {
        parser->pos++;
    }

    char current = parser->input[parser->pos];
    parser->input[parser->pos] = '\0';

    char *value = start;

    // boolean
    if (strcmp(value, "true") == 0)
    {
        array_push(arr, (Data){.bool = 1}, BOOLEAN);
    }
    else if (strcmp(value, "false") == 0)
    {
        array_push(arr, (Data){.bool = 0}, BOOLEAN);
    }

    // null
    else if (strcmp(value, "null") == 0)
    {
        array_push(arr, (Data){0}, TYPE_NULL);
    }

    else
    {
        int has_dot = 0;
        int start_idx = 0;
        int is_number = 1;

        if (value[0] == '-')
        {
            start_idx = 1;
        }

        for (int i = start_idx; value[i] != '\0'; i++)
        {
            if (value[i] == '.')
            {
                if (has_dot)
                {
                    is_number = 0;
                    break;
                }
                has_dot = 1;
            }
            else if (!isdigit((unsigned char)value[i]))
            {
                is_number = 0;
                break;
            }
        }

        if (is_number)
        {
            if (has_dot)
            {
                array_push(
                    arr,
                    (Data){.fl = strtod(value, NULL)},
                    FLOAT);
            }
            else
            {
                array_push(
                    arr,
                    (Data){.in = strtoll(value, NULL, 10)},
                    INTEGER);
            }
        }
        else
        {
            printf("Invalid primitive value: %s\n", value);
        }
    }

    // restore delimiter
    parser->input[parser->pos] = current;

    // if current char is ] after this , let the outer or parent consume it
}

Entry create_entry(char *key, Data data, Type type)
{
    Entry entry;
    entry.data = data;
    entry.type = type;
    entry.key = key;
    return entry;
}

int object_insert(Object *obj, Entry entry)
{
    if (obj->count == obj->capacity)
    {
        int new_cap = obj->capacity * 2;
        Entry *new_entries = realloc(obj->entries, sizeof(Entry) * new_cap);
        if (!new_entries)
        {
            return -1;
        }
        obj->capacity = new_cap;
        obj->entries = new_entries;
    }
    obj->entries[obj->count] = entry;
    obj->count++;
    return 0;
}

int array_push(Array *arr, Data data, Type type)
{
    if (arr->count == arr->capacity)
    {
        int new_cap = arr->capacity * 2;
        Value *new_data = realloc(arr->items, sizeof(Value) * new_cap);
        if (!new_data)
        {
            return -1;
        }
        arr->capacity = new_cap;
        arr->items = new_data;
    }

    if (type == STRING)
    {
        char *copy = strdup(data.string);
        if (!copy)
            return -1;
        arr->items[arr->count] = (Value){
            .item.string = copy,
            .type = type};
        arr->count++;
        return 0;
    }

    arr->items[arr->count] = (Value){
        .item = data,
        .type = type};
    arr->count++;
    return 0;
}

void free_object(Object *obj)
{
    if (!obj)
        return;

    for (int i = 0; i < obj->count; i++)
    {
        Entry *entry = &obj->entries[i];
        switch (entry->type)
        {
        case STRING:
            free(entry->data.string);
            break;
        case OBJECT:
            free_object(entry->data.obj);
            break;
        case ARRAY:
            free_array(entry->data.array);
            break;
        default:
            break;
        }
        free(entry->key);
    }
    free(obj->entries);
    free(obj);
}

void free_array(Array *arr)
{
    if (!arr)
        return;

    for (int i = 0; i < arr->count; i++)
    {
        Value *current = &arr->items[i];

        switch (current->type)
        {
        case STRING:
            free(current->item.string);
            break;
        case OBJECT:
            free_object(current->item.obj);
            break;
        case ARRAY:
            free_array(current->item.array);
            break;
        default:
            break;
        }
    }

    free(arr->items);
    free(arr);
}

void print_indent(int indent)
{
    for (int i = 0; i < indent; i++)
    {
        printf("    ");
    }
}

void print_array(Array *arr, int indent);

void print_object(Object *obj, int indent)
{
    if (!obj)
    {
        printf("null");
        return;
    }

    printf("{\n");

    for (int i = 0; i < obj->count; i++)
    {
        Entry *entry = &obj->entries[i];

        print_indent(indent + 1);
        printf("\"%s\": ", entry->key);

        switch (entry->type)
        {
        case INTEGER:
            printf("%lld", entry->data.in);
            break;

        case FLOAT:
            printf("%g", entry->data.fl);
            break;

        case STRING:
            printf("\"%s\"", entry->data.string);
            break;

        case BOOLEAN:
            printf("%s", entry->data.bool ? "true" : "false");
            break;

        case TYPE_NULL:
            printf("null");
            break;

        case OBJECT:
            print_object(entry->data.obj, indent + 1);
            break;

        case ARRAY:
            print_array(entry->data.array, indent + 1);
            break;

        default:
            printf("unknown");
            break;
        }

        if (i < obj->count - 1)
        {
            printf(",");
        }

        printf("\n");
    }

    print_indent(indent);
    printf("}");
}

void print_array(Array *arr, int indent)
{
    if (!arr)
    {
        printf("null");
        return;
    }

    printf("[\n");

    for (int i = 0; i < arr->count; i++)
    {
        Value *value = &arr->items[i];

        print_indent(indent + 1);

        switch (value->type)
        {
        case INTEGER:
            printf("%lld", value->item.in);
            break;

        case FLOAT:
            printf("%g", value->item.fl);
            break;

        case STRING:
            printf("\"%s\"", value->item.string);
            break;

        case BOOLEAN:
            printf("%s", value->item.bool ? "true" : "false");
            break;

        case TYPE_NULL:
            printf("null");
            break;

        case OBJECT:
            print_object(value->item.obj, indent + 1);
            break;

        case ARRAY:
            print_array(value->item.array, indent + 1);
            break;

        default:
            printf("unknown");
            break;
        }

        if (i < arr->count - 1)
        {
            printf(",");
        }

        printf("\n");
    }

    print_indent(indent);
    printf("]");
}