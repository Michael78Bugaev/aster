#include <sash.h>
#include <string.h>
#include <fs/fat16.h>
#include <fs/file.h>
#include <vga.h>

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
                kprint("Usage: >help\n\n");
            }
            else
            {
                kprint("Aster Operating System Shell commands:\n");
                kprint("  >help: Displays this help message\n");
                kprint("  >clear: Clear the screen\n");

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
        else if (strcmp(args[0], "testing_fs") == 0)
        {
            fopen("0:\\hello.txt", 'r');
        }
        else
        {
            kprintc("Unknown command: ", 0x0C);
            kprintc(args[0], 0x0C);
            kprintc("\n", 0x0C);
        }
    }
    else;

    kprint(">");
}