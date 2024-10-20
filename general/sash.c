#include <sash.h>
#include <string.h>
#include <cpu/pit.h>
#include <fs/fat32.h>
#include <drv/ata.h>
#include <vga.h>

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
            struct directory dir;
            
        }
        else if (strcmp(args[0], "initfs") == 0)
        {
            kprint("Creating FAT32...");
            makeFilesystem(master_fs);
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

    kprint(">");
}