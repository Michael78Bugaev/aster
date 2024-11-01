#ifndef CBREAK_H
#define CBREAK_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TOKEN_INT,
    TOKEN_ARROW,
    TOKEN_IDENTIFIER,
    TOKEN_EQUALS,
    TOKEN_NUMBER,
    TOKEN_FUNC,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_PLUS,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_RETURN,
    TOKEN_EOF,
    TOKEN_PRINT,
    TOKEN_PRINT_INT,
    TOKEN_STRING,
    TOKEN_COMMENT
} TokenType;

typedef struct {
    TokenType type;
    union {
        char* string_value;
        int int_value;
    } value;
} Token;

typedef struct {
    char* name;
    int value;
} Variable;

typedef struct {
    char* name;
    char** parameters;
    int param_count;
    char* body;
} Function;

typedef struct {
    char* source;
    int pos;
    int len;
    char current_char;
    Variable* variables;
    int var_count;
    Function* functions;
    int func_count;
} Lexer;

void parse_function_body(Function* func);
bool find_start_function();
void free_compiler();
void init_compiler(const char* source);
void compile();
Token get_next_token();
void parse_statement();
void parse_variable_declaration();
void parse_function_declaration();
void parse_print_statement();
void execute_start_function();
int parse_expression();

#endif