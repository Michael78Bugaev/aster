#include <sash.h>
#include <string.h>
#include <cpu/pit.h>
#include <sedit.h>
#include <drv/ata.h>
#include <fs/ext2.h>
#include <drv/vbe.h>
#include <config.h>
#include <multiboot.h>
#include <stdio.h>
#include <cpu/mem.h>
#include <drv/pci.h>
#include <chset/chipset.h>
#include <config.h>
#include <vga.h>

bool init = false;

void execute_ls(char *path);
void execute_mkdir(char *name);
void execute_touch(char *name);
void execute_cd(char *path);
void delete_directory_recursively();
void execute_rm(char *name, int recursive, int force);
void execute_cat(char *name);
void execute_edit(char *filename);

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
                printf("Usage: &help\n\n");
            }
            else
            {
                printf("Aster Operating System Shell commands:\n");
                printf("  &help:              Displays this help message\n");
                printf("  &clear:             Clear the screen\n");
                printf("  &tick:              Get current tick count\n");
                printf("  &ls <dir>:          Get all entries in the current directory or the specified\n                      directory\n");
                printf("  &mkdir <dir>:       Create a new directory\n");
                printf("  &touch <file>:      Create a new empty file\n");
                printf("  &cd <dir>:          Change the current directory\n");
                printf("  &rm [-r -f] <file>: Delete a file or directory\n");
                printf("  &cat <file>:        Display the contents of a file\n");
                printf("  &edit <file>:       Edit a file\n");
                printf("  &reboot:            Reboot the system now (for QEMU)\n");
                printf("  &shutdown:          Shutdown the system now (for QEMU)\n");
                printf("  &rainbow:           Show all 16 colors (for screen testing)\n");
                printf("  &debug:             Run debug (MAY CRASH SYSTEM)\n");
                printf("  &debug_end:         End debug mode\n");

            }
            return;
        }
        else if (strcmp(args[0], "lspci") == 0)
        {
            //get_pci_list();
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
            return;
        }
        else if (strcmp(args[0], "pwd") == 0)
        {
            
            return;
        }
        else if (strcmp(args[0], "mkfat32") == 0)
        {
            
        }
        else if (strcmp(args[0], "fat_dir") == 0)
        {
            
        }
        else if (strcmp(args[0], "mkext2") == 0)
        {
            
        }
        else if (strcmp(args[0], "format") == 0)
        {
            
        }
        else if (strcmp(args[0], "in") == 0)
        {
            //ata_pio_write48(0, 0, args[1]);
        }
        else if (strcmp(args[0], "out") == 0)
        {
            // uint8_t *data;
            // ata_pio_read28(0, 1, data);

            // for (int i = 0; i < 512; i++)
            // {
            //     if (i % 64 == 0)
            //     {
            //         printf("\n");
            //     }
            //     printf("%c", data[i]);
            // }
        }
        else if (strcmp(args[0], "cpu") == 0)
        {
            printf(CPUNAME);
            printf("\n");
            return;
        }
        else if (strcmp(args[0], "disk") == 0)
        {
            if (count == 2)
            {
                uint8_t dsk = atoi(args[1]);
                disk = dsk;
            }
            else
            {
                printf("disk = %d\n", disk);
            }
            return;
        }
        else if (strcmp(args[0], "ls") == 0)
        {
            // if (count == 1)
            // {
            //     list_directory(current_directory);
            // }
            // else
            // {
            //     if (startsWith(args[1], ".") == 0)
            //     {
            //         list_directory(current_directory);
            //     }
            //     else if (startsWith(args[1], "..") == 0)
            //     {
            //         if (strcmp(current_directory->name, "/") == 0);
            //         else
            //         {
            //             list_directory(current_directory->parent);
            //         }
            //     }
            //     else
            //     {

            //     }
            // }
            return;
        }
        else if (strcmp(args[0], "sedit") == 0) {
            
            return;
        }
        else if (strcmp(args[0], "mkdir") == 0) {
            
            return;
        } else if (strcmp(args[0], "touch") == 0) {
            
            return;
        } else if (strcmp(args[0], "rm") == 0) {
            // int recursive = 0;
            // int force = 0;
            // char *target = NULL;

            // for (int i = 1; i < count; i++) {
            //     if (strcmp(args[i], "-r") == 0) {
            //         recursive = 1;
            //     } else if (strcmp(args[i], "-f") == 0) {
            //         force = 1;
            //     } else {
            //         target = args[i];
            //     }
            // }

            // if (target) {
            //     execute_rm(target, recursive, force);
            // } else {
            //     printf("Usage: rm [-r] [-f] <file/directory>\n");
            // }
            return;
        } else if (strcmp(args[0], "cat") == 0) {
            // if (count > 1) {
            //     execute_cat(args[1]);
            // } else {
            //     printf("Usage: cat <file>\n");
            // }
            return;
        }
        else if (strcmp(args[0], "reboot") == 0)
        {
            reboot_system();
        }
        else if (strcmp(args[0], "shutdown") == 0)
        {
            shutdown_system();
        }
        else if (strcmp(args[0], "cd") == 0) {
            // if (count == 1) {
            //     printf("Usage: &cd <directory>\n");
            // } else {
            //     // Обрабатываем аргумент для cd
            //     change_directory(args[1]);
            // }
            return;
        }
        else if (strcmp(args[0], "debug") == 0)
        {
            run_debug();
        }
        else if (strcmp(args[0], "debug_end") == 0)
        {
            irq_install_handler(0, &pit_handler);
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
            return;
        }
        else if (strcmp(args[0], "echo") == 0)
        {
            if (count > 1)
            {
                for (int i = 1; i < count; i++)
                {
                    struct global_variable* var_found = find_variable(args[i]);
                    if (var_found == NULL) {
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
            return;
        }
        else if (strcmp(args[0], "int") == 0)
        {
            if (count == 3) {
                init_variable(args[1], args[2], TYPE_INT);
                kprint("\n");
            } else {
                kprint("Usage: >int <var_name> <value>\n");
            }
            return;
        }
        else if (strcmp(args[0], "str") == 0)
        {
            if (count == 3) {
                init_variable(args[1], args[2], TYPE_STR);
                kprint("\n");
            } else {
                kprint("Usage: >str <var_name> <value>\n");
            }
            return;
        }
        else if (strcmp(args[0], "diag_chipset") == 0)
        {
            printf("diagchset v1.0\nError: .\n");
            return;
        }
        else if (strcmp(args[0], "verinfo") == 0)
        {
            printf("\n%s \n2024-2025 Created by Michael Bugaev\n", OS_VERSION);
            return;
        }
        else if (strcmp(args[0], "rainbow") == 0)
        {
            for (int i = 0; i < 16; i++)
            {
                kprintc("####", i);
            }
            printf("\n");
            return;
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
            return;
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
                printf("sash: %s: incorrect command\n", args[0]);
                strnone(arg);
            }
        }
    }
    else strnone(arg);
}

void execute_ls(char *path) {
    
}

void execute_mkdir(char *name) {
    
}

void execute_touch(char *path) {
    
}

void execute_cd(char *path) {
    
}

void delete_directory_recursively() {
    
}

void execute_rm(char *name, int recursive, int force) {
    
}  

void execute_cat(char *name) {
    
}

void execute_edit(char *filename) {
    EditorBuffer editor_buffer;
    init_editor(&editor_buffer); // Инициализация редактора

    if (filename) {
        load_file(&editor_buffer, filename); // Загружаем файл, если указан
    }

    // Запускаем основной цикл редактора
    while (1) {
        display_editor(&editor_buffer); // Отображаем редактор
        // Здесь может быть логика для обработки ввода и выхода из редактора
        // Например, можно использовать keyboard_irq_handler как обработчик ввода
    }

    free_editor_resources(&editor_buffer); // Освобождаем ресурсы перед выходом
}

void run_debug()
{
    irq_install_handler(0, &debug_handler);
}

void sash_shell()
{
    char *command;
    while (1)
    {
        printf(" &");
        command = scanf();
        execute_sash(command);
    }
}