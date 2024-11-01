#include <lang/cbreak.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <cpu/mem.h>
#include <vga.h>

static Lexer lexer;
static Token current_token;
static bool has_error = false;

static void advance();
static void advance() {
    lexer.pos++;
    if (lexer.pos < lexer.len) {
        lexer.current_char = lexer.source[lexer.pos];
    } else {
        lexer.current_char = '\0';
    }
}

void error_print(const char* message);
void error_print(const char* message) {
    printf("Error: %s\n", message);
    has_error = true;
}

void parse_function_body(Function* func) {
    int body_start = lexer.pos;
    int bracket_count = 1;
    
    // Пропускаем первую открывающую скобку
    advance();
    
    while (lexer.current_char != '\0') {
        if (lexer.current_char == '[') {
            bracket_count++;
        }
        if (lexer.current_char == ']') {
            bracket_count--;
            if (bracket_count == 0) {
                break;
            }
        }
        advance();
    }

    if (bracket_count != 0) {
        error_print("Unmatched brackets in function body\n");
        return;
    }

    int body_length = lexer.pos - body_start;
    func->body = malloc(body_length + 1);
    strncpy(func->body, lexer.source + body_start, body_length);
    func->body[body_length] = '\0';
}

bool find_start_function() {
    for (int i = 0; i < lexer.func_count; i++) {
        if (strcmp(lexer.functions[i].name, "start") == 0) {
            return true;
        }
    }
    return false;
}
static char* get_string();
static char* get_string() {
    char* str = malloc(256);  // Предполагаем, что строка не длиннее 255 символов
    int i = 0;
    
    advance();  // Пропускаем открывающую кавычку
    while (lexer.current_char != '"' && lexer.current_char != '\0') {
        str[i++] = lexer.current_char;
        advance();
    }
    str[i] = '\0';
    advance();  // Пропускаем закрывающую кавычку
    
    return str;
}
void free_compiler() {
    for (int i = 0; i < lexer.var_count; i++) {
        mfree(lexer.variables[i].name);
    }
    mfree(lexer.variables);

    for (int i = 0; i < lexer.func_count; i++) {
        mfree(lexer.functions[i].name);
        for (int j = 0; j < lexer.functions[i].param_count; j++) {
            mfree(lexer.functions[i].parameters[j]);
        }
        mfree(lexer.functions[i].parameters);
        mfree(lexer.functions[i].body);
    }
    mfree(lexer.functions);
}
void init_compiler(const char* source) {
    lexer.source = (char*)source;
    lexer.pos = 0;
    lexer.len = strlen(source);
    lexer.current_char = source[0];
    lexer.variables = malloc(sizeof(Variable) * 100);
    lexer.var_count = 0;
    lexer.functions = malloc(sizeof(Function) * 100);
    lexer.func_count = 0;
}

static void skip_whitespace() {
    while (lexer.current_char != '\0' && 
           (lexer.current_char == ' ' || 
            lexer.current_char == '\n' || 
            lexer.current_char == '\t')) {
        advance();
    }
}

static char* get_identifier() {
    char* identifier = malloc(100);
    int i = 0;
    
    while (isalpha(lexer.current_char) || lexer.current_char == '_') {
        identifier[i++] = lexer.current_char;
        advance();
    }
    identifier[i] = '\0';
    return identifier;
}

static int get_number() {
    char number[32] = {0};
    int i = 0;
    
    while (isdigit(lexer.current_char)) {
        number[i++] = lexer.current_char;
        advance();
    }
    number[i] = '\0';
    return atoi(number);
}

Token get_next_token() {
    Token token;
    token.value.string_value = NULL;
    
    skip_whitespace();

    if (lexer.current_char == '\0') {
        token.type = TOKEN_EOF;
        return token;
    }

    if (lexer.current_char == '#') {
        while (lexer.current_char != '\n' && lexer.current_char != '\0') {
            advance();
        }
        return get_next_token();
    }

    if (isalpha(lexer.current_char)) {
        char* identifier = get_identifier();
        
        if (strcmp(identifier, "int") == 0) {
            token.type = TOKEN_INT;
        } else if (strcmp(identifier, "func") == 0) {
            token.type = TOKEN_FUNC;
        } else if (strcmp(identifier, "return") == 0) {
            token.type = TOKEN_RETURN;
        } else if (strcmp(identifier, "print") == 0) {
            token.type = TOKEN_PRINT;
        } else if (strcmp(identifier, "print_int") == 0) {
            token.type = TOKEN_PRINT_INT;
        } else {
            token.type = TOKEN_IDENTIFIER;
            token.value.string_value = identifier;
        }
        return token;
    }

    if (lexer.current_char == '"') {
        token.type = TOKEN_STRING;
        token.value.string_value = get_string();
        return token;
    }

    if (isdigit(lexer.current_char)) {
        token.type = TOKEN_NUMBER;
        token.value.int_value = get_number();
        return token;
    }

    switch (lexer.current_char) {
        case '-':
            advance();
            if (lexer.current_char == '>') {
                advance();
                token.type = TOKEN_ARROW;
            } else {
                token.type = TOKEN_MINUS;
            }
            break;
        case '=':
            advance();
            token.type = TOKEN_EQUALS;
            break;
        case '(':
            advance();
            token.type = TOKEN_LPAREN;
            break;
        case ')':
            advance();
            token.type = TOKEN_RPAREN;
            break;
        case '[':
            advance();
            token.type = TOKEN_LBRACKET;
            break;
        case ']':
            advance();
            token.type = TOKEN_RBRACKET;
            break;
        case ',':
            advance();
            token.type = TOKEN_COMMA;
            break;
        case '+':
            advance();
            token.type = TOKEN_PLUS;
            break;
        case '*':
            advance();
            token.type = TOKEN_MULTIPLY;
            break;
        default:
            error_print("Unknown character");
            char c[2] = {lexer.current_char, '\0'};
            advance();
            return get_next_token();
    }

    return token;
}

void parse_variable_declaration() {
    // Expecting: int -> identifier = value
    current_token = get_next_token(); // Skip 'int'
    
    if (current_token.type != TOKEN_ARROW) {
        error_print("Expected '->' after 'int'");
        return;
    }
    
    current_token = get_next_token();
    if (current_token.type != TOKEN_IDENTIFIER) {
        error_print("Expected identifier after '->'");
        return;
    }
    
    char* var_name = current_token.value.string_value;
    
    current_token = get_next_token();
    if (current_token.type != TOKEN_EQUALS) {
        error_print("Expected '=' after identifier");
        return;
    }
    
    current_token = get_next_token();
    if (current_token.type != TOKEN_NUMBER) {
        error_print("Expected number after '='");
        return;
    }
    
    // Store variable
    lexer.variables[lexer.var_count].name = strdup(var_name);
    lexer.variables[lexer.var_count].value = current_token.value.int_value;
    lexer.var_count++;

    printf("Variable declared: %s = %d\n", var_name, current_token.value.int_value);
}


void compile() {
    current_token = get_next_token();

    while (current_token.type != TOKEN_EOF && !has_error) {
        if (current_token.type == TOKEN_INT) {
            Token next_token = get_next_token();
            if (next_token.type == TOKEN_ARROW) {
                Token peek_token = get_next_token();
                if (peek_token.type == TOKEN_IDENTIFIER) {
                    Token peek_next = get_next_token();
                    if (peek_next.type == TOKEN_LPAREN) {
                        parse_function_declaration();
                    } else {
                        current_token = next_token;
                        parse_variable_declaration();
                    }
                }
            } else {
                error_print("Expected '->' after 'int'");
            }
        } else if (current_token.type == TOKEN_FUNC) {
            parse_function_declaration();
        } else {
            // Игнорируем все остальные инструкции вне функций
            current_token = get_next_token();
        }
        
        if (has_error) break;
    }

    if (!has_error) {
        // Проверяем наличие функции start()
        if (!find_start_function()) {
            error_print("No entry point found! Function 'start()' is missing.");
        } else {
            execute_start_function();
        }
    }
}
void parse_print_statement() {
    TokenType print_type = current_token.type;
    current_token = get_next_token();
    
    if (current_token.type != TOKEN_LPAREN) {
        error_print("Excepted '(' after print function");
        return;
    }
    
    current_token = get_next_token();
    
    if (current_token.type == TOKEN_STRING) {
        printf(current_token.value.string_value);
        current_token = get_next_token();
    } else if (current_token.type == TOKEN_NUMBER || 
               current_token.type == TOKEN_IDENTIFIER) {
        int result = parse_expression();
        printf("%d", result);
        // Не нужно здесь получать следующий токен, 
        // так как parse_expression() уже это сделал
    } else {
        error_print("Expected string, number or identifier in print function");
        return;
    }
    
    if (current_token.type != TOKEN_RPAREN) {
        error_print("Expected ')' after print argument");
        return;
    }
    
    current_token = get_next_token();
    printf("\n");
}
void parse_function_declaration() {
    Function func;
    func.parameters = malloc(sizeof(char*) * 10);  // Максимум 10 параметров
    func.param_count = 0;

    current_token = get_next_token();  // Пропускаем 'int' или 'func'

    if (current_token.type != TOKEN_ARROW) {
        error_print("Expected '->' after type or 'func'");
        return;
    }

    current_token = get_next_token();
    if (current_token.type != TOKEN_IDENTIFIER) {
        error_print("Expected function name");
        return;
    }

    func.name = strdup(current_token.value.string_value);

    current_token = get_next_token();
    if (current_token.type != TOKEN_LPAREN) {
        error_print("Expected '(' after function declaration");
        return;
    }

    // Проверяем случай пустых параметров ()
    current_token = get_next_token();
    if (current_token.type == TOKEN_RPAREN) {
        // Функция без параметров
        func.param_count = 0;
    } else {
        // Функция с параметрами
        while (current_token.type != TOKEN_RPAREN) {
            if (current_token.type != TOKEN_IDENTIFIER) {
                printf("Error: Expected parameter name\n");
                return;
            }
            func.parameters[func.param_count++] = strdup(current_token.value.string_value);

            current_token = get_next_token();
            if (current_token.type == TOKEN_COMMA) {
                current_token = get_next_token();
            } else if (current_token.type != TOKEN_RPAREN) {
                kprint("Error: Expected ',' or ')' after parameter\n");
                return;
            }
        }
    }

    current_token = get_next_token();
    if (current_token.type != TOKEN_LBRACKET) {
        kprint("Error: Expected '[' to start function body\n");
        return;
    }

    // Сохраняем тело функции как строку
    int body_start = lexer.pos;
    int bracket_count = 1;
    while (bracket_count > 0 && lexer.current_char != '\0') {
        if (lexer.current_char == '[') bracket_count++;
        if (lexer.current_char == ']') bracket_count--;
        advance();
    }

    if (bracket_count != 0) {
        kprint("Error: Unmatched brackets in function body\n");
        return;
    }

    int body_length = lexer.pos - body_start - 1;  // -1 чтобы не включать закрывающую скобку
    func.body = malloc(body_length + 1);
    strncpy(func.body, lexer.source + body_start, body_length);
    func.body[body_length] = '\0';

    parse_function_body(&func);

    // Сохраняем функцию
    lexer.functions[lexer.func_count++] = func;
    printf("Function declared: %s (parameters: %d)\n", func.name, func.param_count);
}

void execute_start_function() {
    for (int i = 0; i < lexer.func_count && !has_error; i++) {
        if (strcmp(lexer.functions[i].name, "start") == 0) {
            char* body = lexer.functions[i].body;
            int old_pos = lexer.pos;
            char old_char = lexer.current_char;

            lexer.pos = 0;
            lexer.current_char = body[0];

            while (lexer.current_char != '\0' && !has_error) {
                current_token = get_next_token();
                if (current_token.type == TOKEN_PRINT || current_token.type == TOKEN_PRINT_INT) {
                    parse_print_statement();
                }
                // Здесь можно добавить обработку других инструкций
                
                if (has_error) break;
            }

            lexer.pos = old_pos;
            lexer.current_char = old_char;

            return;
        }
    }
    if (!has_error) {
        error_print("start() function not found!");
    }
}

int parse_expression() {
    int result = 0;
    
    if (current_token.type == TOKEN_NUMBER) {
        result = current_token.value.int_value;
        current_token = get_next_token();
    } else if (current_token.type == TOKEN_IDENTIFIER) {
        // Поиск переменной
        bool found = false;
        for (int i = 0; i < lexer.var_count; i++) {
            if (strcmp(lexer.variables[i].name, current_token.value.string_value) == 0) {
                result = lexer.variables[i].value;
                found = true;
                break;
            }
        }
        if (!found) {
            printf("Undefined variable '%s'\n", current_token.value.string_value);
            return 0;
        }
        current_token = get_next_token();
    } else {
        error_print("Expected number or variable in expression");
        return 0;
    }
    
    // Проверяем наличие оператора
    if (current_token.type == TOKEN_PLUS || 
        current_token.type == TOKEN_MINUS || 
        current_token.type == TOKEN_MULTIPLY) {
        TokenType operator = current_token.type;
        current_token = get_next_token();
        
        int right_operand = parse_expression(); // Рекурсивный вызов для правого операнда
        
        switch (operator) {
            case TOKEN_PLUS:
                result += right_operand;
                break;
            case TOKEN_MINUS:
                result -= right_operand;
                break;
            case TOKEN_MULTIPLY:
                result *= right_operand;
                break;
        }
    }
    
    return result;
}