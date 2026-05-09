#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <json.h>

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
                // print_object(inner, 1);
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

Object *parser_json_for_real(Parser *parser)
{
    Object *obj = create_object();
    int level = 0;
    while (parser->input[parser->pos] != '\0')
    {
        if (parser->input[parser->pos] == '{')
        {
            level++;
            parser->pos++;
        }
        while (parser->input[parser->pos] == '\n' || parser->input[parser->pos] == '\r' || parser->input[parser->pos] == '\t' || parser->input[parser->pos] == ' ')
        {
            parser->pos++;
        }
        if (parser->input[parser->pos] == '"')
        {
            parser->pos++;
            char *key = &parser->input[parser->pos];
            while (parser->input[parser->pos] != '"')
            {
                parser->pos++;
            }
            if (parser->input[parser->pos] == '"')
            {
                parser->input[parser->pos] = '\0';
                parser->pos++;
            } else {
                free_object(obj);
                return NULL;
            }
            // TODO: create entry and insert the key;
        }
        if(parser->input[parser->pos] == ':'){
            
        }
    }
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

Entry *create_entry_from_value(const char *key, const char *value)
{
    if (!key || !value)
        return NULL;

    Entry *entry = malloc(sizeof(Entry));
    if (!entry)
        return NULL;

    entry->key = strdup(key);
    if (!entry->key)
    {
        free(entry);
        return NULL;
    }

    // -------- Type Detection --------
    Type type = STRING;

    if (strcmp(value, "true") == 0 || strcmp(value, "false") == 0)
    {
        type = BOOLEAN;
    }
    else if (strcmp(value, "null") == 0)
    {
        type = TYPE_NULL;
    }
    else
    {
        int has_dot = 0;
        int start = 0;
        int is_number = 1;

        if (value[0] == '-')
        {
            start = 1;
        }

        for (int i = start; value[i] != '\0'; i++)
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
            type = has_dot ? FLOAT : INTEGER;
        }
    }

    entry->type = type;

    // -------- Data Conversion --------
    switch (type)
    {
    case INTEGER:
        entry->data.in = strtoll(value, NULL, 10);
        break;

    case FLOAT:
        entry->data.fl = strtod(value, NULL);
        break;

    case BOOLEAN:
        entry->data.bool = (strcmp(value, "true") == 0);
        break;

    case TYPE_NULL:
        break;

    case STRING:
    {
        entry->data.string = strdup(value);
        if (!entry->data.string)
        {
            free(entry->key);
            free(entry);
            return NULL;
        }
        break;
    }

    default:
        free(entry->key);
        free(entry);
        return NULL;
    }

    return entry;
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
            printf("%f", entry->data.fl);
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
            printf("%f", value->item.fl);
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