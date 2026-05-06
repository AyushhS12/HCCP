#ifndef PARSER_H
#define PARSER_H

typedef struct Parser Parser;

Parser create_parser(char *input);

void parse_json(Parser *parser);

#endif