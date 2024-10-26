#include <sash.h>
#include <string.h>
#include <cpu/pit.h>
#include <fs/fat32.h>
#include <cpu/mem.h>
#include <drv/ata.h>
#include <sfat32.h>
#include <vga.h>
#include <sfat32.h>

void execute_sash(char *arg)
{
    f32 fat32;
    int count;
    char **args = splitString(arg, &count);
    if (count > 0)
    {
        if (strcmp(args[0], "help") == 0)
        {
            if (count > 1)
            {
                kprint("Usage: >help\n\n");
            }
            else
            {
                kprint("Aster Operating System Shell commands:\n");
                kprint("  >help: Displays this help message\n");
                kprint("  >clear: Clear the screen\n");
                kprint("  >tick: Get current tick count\n");

            }
        }
        else if (strcmp(args[0], "clear") == 0)
        {
            if (count > 1)
            {
                kprint("Usage: >clear\n\n");
            }
            else
            {
                clear_screen();
            }
        }
        else if (strcmp(args[0], "ls") == 0)
        {
            struct dir_s dir;
            fat_dir_open(&dir, get_current_directory(), 0);
            struct info_s* info = (struct info_s*)find_memblock(0x00000000, sizeof(struct info_s));
            fstatus status;
            status = fat_dir_read(&dir, info);
            if (status == FSTATUS_OK)
            {
                fat_kprint_info(info);
            }
        }
        else if (strcmp(args[0], "md") == 0)
        {
            if (count > 1)
            {
                if (startsWith("C:\\", args[1]) == 0)
                {
                    fat_dir_make(args[1]);
                }
                else
                {
                    char *pathdir = get_current_directory() + '/';
                    strcat(pathdir, args[1]);
                    fat_dir_make(pathdir);
                }
            }
            else
            {
                kprintc("Usage: C:\\>md <directory_name>\n", 0x0C);
            }
        }
        else if (strcmp(args[0], "verinfo") == 0)
        {
            kprint("\nAster 32-bit kernel 1.00 \n2024-2025 Created by Michael Bugaev\n");
        }
        else if (strcmp(args[0], "initfs") == 0)
        {
            kprint("Creating FAT32...\n");
            fat32_thread(0);
        }
        else if (strcmp(args[0], "shutdown") == 0)
        {
            port_word_out(0x604, 0x2000);
            port_word_out(0x4004, 0x3400);
            port_word_out(0xB004, 0x2000);
        }
        else if (strcmp(args[0], "rainbow") == 0)
        {
            clear_screen();
            for (int i = 0; i < 25; i++)
            {
                for (int i = 0; i < 16; i++)
                {
                    kprintc("#####", i);
                }
                kprint("\n");
            }
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
            kprintc("Unknown command \"", 0x0C);
            kprintc(args[0], 0x0C);
            kprintc("\"\n", 0x0C);
        }
    }
    else;
}