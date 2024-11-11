#include <sash.h>
#include <string.h>
#include <cpu/pit.h>
#include <stdio.h>
#include <cpu/mem.h>
#include <chset/chipset.h>
#include <config.h>
#include <vga.h>

bool init = false;

void execute_sash(char *arg)
{
    int count;
    char **args = splitString(arg, &count);
    if (count > 0)
    {
        if (strcmp(args[0], "help") == 0)
        {
            if (count > 1)
            {
                kprint("Usage: &help\n\n");
            }
            else
            {
                printf("Aster Operating System Shell commands:\n");
                printf("  &help:     Displays this help message\n");
                printf("  &clear:    Clear the screen\n");
                printf("  &tick:     Get current tick count\n");
                printf("  &identify: Identify disks (searching for one disk)\n            to initiliaze FAT32 on them\n");
                printf("  &initfs:   Initiliaze FAT32 on the first disk\n");
                printf("  &rainbow:  Show all 16 colors (for screen testing)\n");
                printf("  &mem:      Display memory map\n");

            }
        }
        else if (strcmp(args[0], "clear") == 0)
        {
            if (count > 1)
            {
                printf("Usage: &clear\n\n");
            }
            else
            {
                clear_screen();
            }
        }
        else if (strcmp(args[0], "ls") == 0)
        {
            printf("LS command\n");
        }
        else if (strcmp(args[0], "reboot") == 0)
        {
            reboot_system();
        }
        else if (strcmp(args[0], "shutdown") == 0)
        {
            shutdown_system();
        }
        else if (strcmp(args[0], "mkhello") == 0)
        {
            printf("make 'hello.txt' file!\n");
        }
        else if (strcmp(args[0], "sector") == 0)
        {
            printf("don't touch me, i'm reading sector, which is not implemented yet\n");
        }
        else if (strcmp(args[0], "export") == 0)
        {
            for (int i = 0; i < get_var_count(); i++)
            {
                char *name = var->name[i];
                if (var->type == TYPE_INT)
                {
                    kprint("int ");
                    kprint(tostr(name));
                    kprint(" = ");
                    kprinti(var->data.int_value);
                    kprint("\n");
                }
                else
                {
                    kprint("str ");
                    kprint(tostr(name));
                    kprint(" = ");
                    kprint(var->data.str_value);
                    kprint("\n");
                }
            }
        }
        else if (strcmp(args[0], "echo") == 0)
        {
            if (count > 1)
            {
                for (int i = 1; i < count; i++)
                {
                    struct global_variable* var_found = find_variable(args[i]);
                    if (var_found != NULL) {
                        // Если переменная найдена, выводим её значение
                        if (var_found->type == TYPE_INT) {
                            kprinti(var_found->data.int_value);
                        } else if (var_found->type == TYPE_STR) {
                            kprint(var_found->data.str_value);
                        }
                    } else {
                        if (args[i][0] == '"' && args[i][strlen(args[i]) - 1] == '"') {
                            // Удаляем кавычки и выводим строку
                            args[i][strlen(args[i]) - 1] = '\0'; // Убираем завершающую кавычку
                            kprint(&args[i][1]); // Выводим строку без начальной кавычки
                        } else {
                            // Если не строка, выводим имя
                            kprint(args[i]);
                        }
                    }

                    if (i < count - 1)
                    {
                        kprint(" ");
                    }
                }
                kprint("\n");
            }
            else
            {
                kprint("Usage: echo <variable_name> | \"hello world\" | 12345\n");
            }
        }
        else if (strcmp(args[0], "int") == 0)
        {
            if (count == 3) {
                init_variable(args[1], args[2], TYPE_INT);
                kprint("\n");
            } else {
                kprint("Usage: >int <var_name> <value>\n");
            }
        }
        else if (strcmp(args[0], "str") == 0)
        {
            if (count == 3) {
                init_variable(args[1], args[2], TYPE_STR);
                kprint("\n");
            } else {
                kprint("Usage: >str <var_name> <value>\n");
            }
        }
        else if (strcmp(args[0], "diag_chipset") == 0)
        {
            printf("diagchset v1.0\nError: not implemented yet\n");
        }
        else if (strcmp(args[0], "verinfo") == 0)
        {
            kprint("\nAster 32-bit kernel 1.00 \n2024-2025 Created by Michael Bugaev\n");
        }
        else if (strcmp(args[0], "rainbow") == 0)
        {
            for (int i = 0; i < 16; i++)
            {
                kprintc("#####", i);
            }
            printf("\n");
        }
        else if (strcmp(args[0], "tick") == 0)
        {
            if (count > 1)
            {
                kprint("Usage: >tick\n\n");
            }
            else
            {
                kprint("Current tick: ");
                kprinti(get_ticks());
                kprint("ms\n");
            }
        }
        else
        {
            if (strcmp(args[1], "=") == 0)
            {
                if (count == 3) {
                    assign_variable(args[0], args[2]); // Присваиваем значение
                    kprint("\n");
                } else {
                    kprint("Usage: <var_name> = <value>\n");
                }
            }
            else
            {
                kprintc("Unknown command", 0x0C);
            }
        }
    }
    else;
}