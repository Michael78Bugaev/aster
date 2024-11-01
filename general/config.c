#include <config.h>
#include <stdint.h>
#include <string.h>
#include <vga.h>
#include <cpu/mem.h>

uint64_t var_count = 0;

struct global_variable* find_variable(const char *name) {
    for (int i = 0; i < VAR_MAXCOUNT; i++) {
        if (strcmp(var[i].name, name) == 0) {
            return &var[i];
        }
    }
    return NULL; // Если переменная не найдена
}

void init_variable(const char *name, const char *value, int type) {
    struct global_variable new_var;
    new_var.name = strdup(name); // Копируем имя переменной
    new_var.type = type;

    if (isdigit(name[0])) {
        kprint("error: variable name cannot start with a digit\n");
        return;
    }

    if (type == TYPE_INT) {
        // Проверка на наличие букв в строке
        for (const char *p = value; *p; p++) {
            if (isalpha(*p)) {
                kprint("error: variable contains letters\n");
                return; // Выход из функции, если ошибка
            }
        }
        new_var.data.int_value = atoi(value); // Преобразуем строку в int
    } else if (type == TYPE_STR) {
        new_var.data.str_value = strdup(value); // Копируем строку
    }
    var[var_count++] = new_var; // Сохраняем переменную
}

void assign_variable(const char *var_name, const char *value) {
    struct global_variable *var_to_assign = find_variable(var_name);
    if (var_to_assign != NULL) {
        if (var_to_assign->type == TYPE_INT) {
            var_to_assign->data.int_value = atoi(value);
        } else if (var_to_assign->type == TYPE_STR) {
            mfree(var_to_assign->data.str_value); // Освобождаем старую строку
            var_to_assign->data.str_value = strdup(value); // Копируем новое значение
        }
    } else {
        kprint("syntax error: variable ");
        kprint(var_name);
        kprint(" not found\n");
    }
}

void free_variables() {
    for (int i = 0; i < VAR_MAXCOUNT; i++) {
        mfree(var[i].name);
        if (var[i].type == TYPE_STR) {
            mfree(var[i].data.str_value);
        }
    }
}

int get_var_count()
{
    return var_count;
}