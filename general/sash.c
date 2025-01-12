#include <sash.h>
#include <string.h>
#include <cpu/pit.h>
#include <fs/file.h>
#include <fs/initrd.h>
#include <sedit.h>
#include <drv/ata.h>
#include <fs/ext2.h>
#include <fs/fat32.h>
#include <fs/dir.h>
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
void delete_directory_recursively(Directory *dir);
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
            get_pci_list();
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
            printf(current_directory->full_name);
            printf("\n");
            return;
        }
        else if (strcmp(args[0], "mkfat32") == 0)
        {
            IDE_DEVICE* device = get_device();
            if (!device) { printf("Something went wrong...\n"); return; }
            if (fat32_format(disk, 1, 128, device->size) != FAT32_SUCCESS)
            {
                printf("Error initiliazing FAT32 on drive %d\n", disk);
                return;
            }
            printf("FAT32 initialized on drive %d\n", disk);
        }
        else if (strcmp(args[0], "fat_dir") == 0)
        {
            DIR_ENTRY_FAT32 *dir;
            int num_entries = fat32_list_dir(disk, "0:/", dir, 10);
            if (num_entries > 0) {
                printf("List of directory:\n");
                for (int i = 0; i < num_entries; i++) {
                    char name[12];
                    memcpy(name, dir[i].DIR_Name, 11);
                    name[11] = '\0';
                    printf("%s\n", name);
                }
            } else {
                printf("Ошибка обхода директории\n");
            }
        }
        else if (strcmp(args[0], "mkext2") == 0)
        {
            ext2_format(disk, 1024, 128);
        }
        else if (strcmp(args[0], "format") == 0)
        {
            // Заполнение всего диска нулями
            uint8_t buffer[512]; // Буфер для записи
            memset(buffer, 0, sizeof(buffer)); // Заполняем буфер нуля
            // Предполагаем, что диск имеет 1024 сектора (это нужно изменить в зависимости от реального размера диска)
            for (uint32_t lba = 0; lba < 1024; lba++) {
                ide_write_sectors(disk, 1 + lba, lba, buffer); // Записываем один сектор
            }
            printf("format: disk %d formatted successfully.\n", disk);
        }
        else if (strcmp(args[0], "in") == 0)
        {
            char buffer[512];
            strcpy(buffer, args[3]);
            ide_write_sectors(disk, atoi(args[1]), atoi(args[2]), buffer);
            return;
        }
        else if (strcmp(args[0], "out") == 0)
        {
            uint8_t buffer[512];
            int num_sectors_to_read = atoi(args[1]);
            ide_read_sectors(disk, 1, 0, buffer);
            // Печатаем текстовое представление
            printf(" ");
            for (int i = 0; i < num_sectors_to_read; i++) {
                if (i % 64 == 0)
                {
                    printf("\n");
                }
                // Если байт является печатным символом, выводим его, иначе выводим точку
                if (buffer[i] >= 32 && buffer[i] <= 126) {
                    printf("%c", buffer[i]);
                } else {
                    printf(".");
                }
            }
            printf("\n");                     /**/
            return;
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
            if (count == 1)
            {
                list_directory(current_directory);
            }
            else
            {
                if (startsWith(args[1], ".") == 0)
                {
                    list_directory(current_directory);
                }
                else if (startsWith(args[1], "..") == 0)
                {
                    if (strcmp(current_directory->name, "/") == 0);
                    else
                    {
                        list_directory(current_directory->parent);
                    }
                }
                else
                {

                }
            }
            return;
        }
        else if (strcmp(args[0], "sedit") == 0) {
            if (count > 1) {
                execute_edit(args[1]); // Передаем имя файла в редактор
            } else {
                execute_edit(NULL); // Запускаем редактор без загрузки файла
            }
            return;
        }
        else if (strcmp(args[0], "mkdir") == 0) {
            if (count > 1) {
                execute_mkdir(args[1]);
            } else {
                printf("Usage: mkdir <directory>\n");
            }
            return;
        } else if (strcmp(args[0], "touch") == 0) {
            if (count > 1) {
                execute_touch(args[1]);
            } else {
                printf("Usage: touch <file>\n");
            }
            return;
        } else if (strcmp(args[0], "rm") == 0) {
            int recursive = 0;
            int force = 0;
            char *target = NULL;

            for (int i = 1; i < count; i++) {
                if (strcmp(args[i], "-r") == 0) {
                    recursive = 1;
                } else if (strcmp(args[i], "-f") == 0) {
                    force = 1;
                } else {
                    target = args[i];
                }
            }

            if (target) {
                execute_rm(target, recursive, force);
            } else {
                printf("Usage: rm [-r] [-f] <file/directory>\n");
            }
            return;
        } else if (strcmp(args[0], "cat") == 0) {
            if (count > 1) {
                execute_cat(args[1]);
            } else {
                printf("Usage: cat <file>\n");
            }
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
            if (count == 1) {
                printf("Usage: &cd <directory>\n");
            } else {
                // Обрабатываем аргумент для cd
                change_directory(args[1]);
            }
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
                printf("sash: incorrect command\n");
                strnone(arg);
            }
        }
    }
    else strnone(arg);
}

void execute_ls(char *path) {
    Directory *dir = (path && strcmp(path, ".") != 0) ? find_directory(path, current_directory) : current_directory;
    if (dir) {
        list_directory(dir);
    } else {
        printf("ls: %s: No such directory\n", path);
    }
}

void execute_mkdir(char *name) {
    if (create_directory(name)) {
    } else {
        printf("mkdir: error: %s: Directory already exists or failed to create\n", name);
    }
}

void execute_touch(char *path) {
    // Проверяем, что путь не пустой
    if (path == NULL || strlen(path) == 0) {
        printf("Usage: touch <file>\n");
        return;
    }

    // Находим последнюю '/' в пути
    char *last_slash = strrchr(path, '/');
    if (last_slash != NULL) {
        // Разделяем путь на директорию и имя файла
        *last_slash = '\0'; // Завершаем строку на месте последнего '/'
        char *filename = last_slash + 1; // Имя файла

        // Находим или создаем директорию
        Directory *target_dir = find_or_create_directory(path);
        if (target_dir == NULL) {
            printf("Error: Directory not found for path: %s\n", path);
            return;
        }

        // Создаем пустой файл в найденной директории
        File *file = create_file(filename, NULL, 0, target_dir);
        if (file) {
            printf("File created: %s/%s\n", path, filename);
        } else {
            printf("Error: Failed to create file %s\n", filename);
        }
    } else {
        // Если '/' не найден, создаем файл в текущей директории
        File *file = create_file(path, NULL, 0, current_directory);
        if (file) {
            printf("File created: %s\n", path);
        } else {
            printf("Error: Failed to create file %s\n", path);
        }
    }
}

void execute_cd(char *path) {
    
}

void delete_directory_recursively(Directory *dir) {
    for (uint32_t i = 0; i < dir->file_count; i++) {
        mfree(dir->files[i]->data);
        mfree(dir->files[i]);
    }
    for (uint32_t i = 0; i < dir->dir_count; i++) {
        delete_directory_recursively(dir->subdirs[i]);
        mfree(dir->subdirs[i]);
    }
}

void execute_rm(char *name, int recursive, int force) {
    Directory *target_dir = find_directory(name, current_directory);
    if (target_dir) {
        if (recursive) {
            delete_directory_recursively(target_dir);
        } else {
            printf("rm: %s: directory is not empty, use -r to remove\n", name);
        }
    } else {
        File *file = find_file(name, current_directory);
        if (file) {
            delete_file(name, current_directory);
            printf("File removed: %s\n", name);
        } else {
            printf("rm: %s: no such file or directory\n", name);
        }
    }
}  

void execute_cat(char *name) {
    // Копируем путь для обработки
    char *path_copy = strdup(name);
    char *token = strtok(path_copy, "/");
    Directory *current_dir = root; // Начинаем с корневой директории

    // Обработка полного пути
    while (token != NULL) {
        // Проверяем, является ли это последним токеном
        if (strtok(NULL, "/") == NULL) {
            // Мы находимся на последнем элементе, который должен быть файлом
            File *file = find_file(token, current_dir);
            if (file) {
                // Проверяем, что это действительно файл
                if (file->data != NULL) {
                    for (uint32_t i = 0; i < file->size; i++) {
                        putchar(file->data[i], 0x07); // Выводим каждый байт
                    }
                    putchar('\n', 0x07); // Переход на новую строку после вывода
                } else {
                    printf("cat: %s: File is empty\n", name);
                }
                mfree(path_copy);
                return;
            } else {
                printf("cat: %s: No such file\n", name);
                mfree(path_copy);
                return;
            }
        } else {
            // Если это не последний токен, ищем директорию
            Directory *next_dir = find_directory(token, current_dir);
            if (next_dir == NULL) {
                printf("cat: %s: No such file\n", name);
                mfree(path_copy);
                return;
            }
            current_dir = next_dir; // Переходим в подкаталог
        }
        token = strtok(NULL, "/");
    }

    mfree(path_copy);
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
        printf("<(0a)>%s@%s<(0f)>:<(09)>/%s<(07)> &", current_username, COMPUTER_NAME, current_directory->full_name);
        command = scanf();
        execute_sash(command);
    }
}