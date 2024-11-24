#include <io/kb.h>
#include <io/iotools.h>
#include <vga.h>
#include <stdio.h>
#include <fs/initrd.h>
#include <fs/dir.h>
#include <sash.h>
#include <string.h>
#include <stdint.h>

#define MAX_HISTORY 1024
#define MAX_COMMAND_LENGTH 1024


uint8_t *input = "";
char agent_input[512];
bool capsOn = false;
bool capsLock = false;
bool enter = false;
int barrier = 2;
uint8_t shell_history[MAX_COMMAND_LENGTH][MAX_HISTORY];
int history_index = 0;
int cursor_index = 0;

int backspace_func(char buffer[]);
char get_acsii_low(char code);
char get_acsii_high(char code);

int backspace_func(char buffer[])
{
    int len = strlen(buffer);
    if (len > 0)
    {
        buffer[len - 1] = '\0';
        return 1;
    }
    else
    {
        return 0;
    }
}

void set_barrier(int n)
{
  barrier = n;
}

void handler(struct InterruptRegisters *regs)
{
      char scanCode = port_byte_in(0x60) & 0x7F;
	  char press = port_byte_in(0x60) & 0x80;
    //kprint(lowercase[0x22]);

    switch (scanCode)
    {
        case 1:
        case 29:
        case 56:
        case 59:
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
        case 87:
        case 88:
            break;
        case 42:
            if (press == 0) {
                capsOn = true;
                int old = get_cursor();
                set_cursor(0);
                kprintc(" SHIFT", 0x70);
                set_cursor(old);
            }else{
                capsOn = false;
                int old = get_cursor();
                set_cursor(0);
                kprintc("      ", 0x70);
                set_cursor(old);
            }
            break;
        case 58:
            if (!capsLock && press == 0)
            {
                capsLock = true;
                int old = get_cursor();
                set_cursor(0);
                kprintc(" CAPS LOCK", 0x70);
                set_cursor(old);
            }
            else if (capsLock && press == 0)
            {
                capsLock = false;
                int old = get_cursor();
                set_cursor(0);
                kprintc("          ", 0x70);
                set_cursor(old);
            }
            break;
        case 40:
            if (press)
                putchar('\'', 0x07);
                join(input, '\'');
            break;
        case 0x29:
            if (press)
                putchar('~', 0x07);
                join(input, '~');
            break;
        case 0x0E:
            if (press == 0)
                if (get_cursor_x() > barrier)
                {
                  kprint("\b");
                  backspace_func(input);
                }
            break;
        case 0x1C:
            if (press == 0)
            {
                kprint("\n");
                //input[0] = '\0';
                enter = true;
                irq_uninstall_handler(1);
            }
            else;    
                
            break;
        case 0x0F:
            if (press);
            break;
        default:
            if (press == 0)
            {
                if (capsOn || capsLock)
                {
                    kprintci_vidmem(scanCode, 0x70, 150);
                    putchar(get_acsii_high(scanCode), 0x07);
                    join(input, get_acsii_high(scanCode));
                }
                else
                {
                    kprintci_vidmem(scanCode, 0x70, 150);
                    putchar(get_acsii_low(scanCode), 0x07);
                    join(input, get_acsii_low(scanCode));
                }
            }
            else{
              int old = get_cursor();
              set_cursor(150);
              kprintc("     ", 0x70);
              set_cursor(old);
            }
            break;
    }
}
void agent_handler(struct InterruptRegisters *regs)
{
    while (1)
    {
      char scanCode = port_byte_in(0x60) & 0x7F;
	  char press = port_byte_in(0x60) & 0x80;
    //kprint(lowercase[0x22]);

    switch (scanCode)
    {
        case 1:
        case 29:
        case 56:
        case 59:
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
        case 87:
        case 88:
            break;
        case 42:
            if (press == 0) {
                capsOn = true;
                int old = get_cursor();
                set_cursor(0);
                kprintc(" SHIFT", 0x70);
                set_cursor(old);
            }else{
                capsOn = false;
                int old = get_cursor();
                set_cursor(0);
                kprintc("      ", 0x70);
                set_cursor(old);
            }
            break;
        case 58:
            if (!capsLock && press == 0)
            {
                capsLock = true;
                int old = get_cursor();
                set_cursor(0);
                kprintc(" CAPS LOCK", 0x70);
                set_cursor(old);
            }
            else if (capsLock && press == 0)
            {
                capsLock = false;
                int old = get_cursor();
                set_cursor(0);
                kprintc("          ", 0x70);
                set_cursor(old);
            }
            break;
        case 40:
            if (press)
                putchar('*', 0x07);
                join(input, '\'');
            break;
        case 0x29:
            if (press)
                putchar('*', 0x07);
                join(input, '~');
            break;
        case 0x0E:
            if (press == 0)
                putchar('*', 0x07);
                backspace_func(input);
            break;
        case 0x1C:
            if (press == 0)
            {
                kprint("\n");
                //input[0] = '\0';
                enter = true;
                irq_uninstall_handler(1);
                break;
            }
            else;    
                
            break;
        case 0x0F:
            if (press);
            break;
        default:
            if (press == 0)
            {
                if (capsOn || capsLock)
                {
                    putchar('*', 0x07);
                    join(input, get_acsii_high(scanCode));
                }
                else
                {
                    putchar('*', 0x07);
                    join(input, get_acsii_low(scanCode));
                }
            }
            break;
    }
    }
}

void add_to_history(const char *command);

void sash(struct InterruptRegisters *regs)
{
    char scanCode = port_byte_in(0x60) & 0x7F;
	  char press = port_byte_in(0x60) & 0x80;
    int length = strlen(input);
    //kprint(lowercase[0x22]);

    switch (scanCode)
    {
        case 1:
        case 29:
        case 56:
        case 59:
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
        case 87:
        case 80:
        case 72:
        case 88:
            break;
        case 0x4B: // Стрелка влево
            if (press == 0 && cursor_index > 0) {
                cursor_index--;
                // Перемещение курсора влево на экране
                set_cursor(get_cursor() - 1); // Предполагается, что kprint поддерживает этот код
            }
            break;
        case 0x4D: // Стрелка вправо
            if (press == 0 && cursor_index < length) {
                cursor_index++;
                set_cursor(get_cursor() + 1);
            }
            break;
        case 42:
            if (press == 0) {
                capsOn = true;
            }else{
                capsOn = false;
            }
            break;
        case 58:
            if (!capsLock && press == 0)
            {
                capsLock = true;
            }
            else if (capsLock && press == 0)
            {
                capsLock = false;
            }
            break;
        case 40:
            if (press) {
                if (capsOn || capsLock)
                {
                  putchar('\"', 0x07);
                  join(input, '\"'); 
                }
                else
                {
                  putchar('\'', 0x07);
                  join(input, '\''); 
                }
            }
            break;
        case 0x29:
            if (press)
                putchar('~', 0x07);
                join(input, '~');
            break;
        case 0x0E:
            if (press == 0)
                if (get_cursor_x() > barrier)
                {
                  kprint("\b");
                  backspace_func(input);
                }
            break;
        case 0x1C:
            if (press == 0)
            {
                kprint("\n");
                //input[0] = '\0';
                enter = true;
                add_to_history(input);
                execute_sash(input);
                
                if (strcmp(current_directory->name, "/") == 0)
                {
                  printf("\nmasteruser: / &");
                  strnone(input);
                  return;
                }

                if (startsWith(current_directory->name, "/") == 0)
                {
                  printf("\nmasteruser: /%s &", current_directory->name);
                }
                else
                {
                  printf("\nmasteruser: %s &", current_directory->name);
                }
                
                strnone(input);
            }
            else;    
                
            break;
        case 0x0F:
            if (press);
            break;
        default:
            if (press == 0)
            {
                if (capsOn || capsLock)
                {
                    putchar(get_acsii_high(scanCode), 0x07);
                    join(input, get_acsii_high(scanCode));
                }
                else
                {
                    putchar(get_acsii_low(scanCode), 0x07);
                    join(input, get_acsii_low(scanCode));
                }
            }
            break;
    }
}

void add_to_history(const char *command) {
    static int history_count = 0;
    static int current_index = 0;

    // Проверяем, не пустая ли команда
    if (command == NULL || command[0] == '\0') {
        return;
    }

    // Копируем команду в history
    strncpy(shell_history[current_index], command, MAX_COMMAND_LENGTH - 1);
    shell_history[current_index][MAX_COMMAND_LENGTH - 1] = '\0';  // Обеспечиваем завершающий нуль

    // Увеличиваем индекс и счетчик
    current_index = (current_index + 1) % MAX_HISTORY;
    if (history_count < MAX_HISTORY) {
        history_count++;
    }
}

void get_string(uint8_t *buffer)
{
    barrier = get_cursor_x();
    while (enter != true)
    {
      irq_install_handler(1, &handler);
    }
    enter = false;
    buffer = input;
    
}
char *agent_get_string()
{
    irq_install_handler(1, &agent_handler);
    return input;
}

char get_acsii_low(char code)
{
     switch (code)
  {
  case KEY_A:
    return 'a';
  case KEY_B:
    return 'b';
  case KEY_C:
    return 'c';
  case KEY_D:
    return 'd';
  case KEY_E:
    return 'e';
  case KEY_F:
    return 'f';
  case KEY_G:
    return 'g';
  case KEY_H:
    return 'h';
  case KEY_I:
    return 'i';
  case KEY_J:
    return 'j';
  case KEY_K:
    return 'k';
  case KEY_L:
    return 'l';
  case KEY_M:
    return 'm';
  case KEY_N:
    return 'n';
  case KEY_O:
    return 'o';
  case KEY_P:
    return 'p';
  case KEY_Q:
    return 'q';
  case KEY_R:
    return 'r';
  case KEY_S:
    return 's';
  case KEY_T:
    return 't';
  case KEY_U:
    return 'u';
  case KEY_V:
    return 'v';
  case KEY_W:
    return 'w';
  case KEY_X:
    return 'x';
  case KEY_Y:
    return 'y';
  case KEY_Z:
    return 'z';
  case KEY_1:
    return '1';
  case KEY_2:
    return '2';
  case KEY_3:
    return '3';
  case KEY_4:
    return '4';
  case KEY_5:
    return '5';
  case KEY_6:
    return '6';
  case KEY_7:
    return '7';
  case KEY_8:
    return '8';
  case KEY_9:
    return '9';
  case KEY_0:
    return '0';
  case KEY_MINUS:
    return '-';
  case KEY_EQUAL:
    return '=';
  case KEY_SQUARE_OPEN_BRACKET:
    return '[';
  case KEY_SQUARE_CLOSE_BRACKET:
    return ']';
  case KEY_SEMICOLON:
    return ';';
  case KEY_BACKSLASH:
    return '\\';
  case KEY_COMMA:
    return ',';
  case KEY_DOT:
    return '.';
  case KEY_FORESLHASH:
    return '/';
  case KEY_SPACE:
    return ' ';
  default:
    return 0;
  }
}
char get_acsii_high(char code)
{
     switch (code)
  {
  case KEY_A:
    return 'A';
  case KEY_B:
    return 'B';
  case KEY_C:
    return 'C';
  case KEY_D:
    return 'D';
  case KEY_E:
    return 'E';
  case KEY_F:
    return 'F';
  case KEY_G:
    return 'G';
  case KEY_H:
    return 'H';
  case KEY_I:
    return 'I';
  case KEY_J:
    return 'J';
  case KEY_K:
    return 'K';
  case KEY_L:
    return 'L';
  case KEY_M:
    return 'M';
  case KEY_N:
    return 'N';
  case KEY_O:
    return 'O';
  case KEY_P:
    return 'P';
  case KEY_Q:
    return 'Q';
  case KEY_R:
    return 'R';
  case KEY_S:
    return 'S';
  case KEY_T:
    return 'T';
  case KEY_U:
    return 'U';
  case KEY_V:
    return 'V';
  case KEY_W:
    return 'W';
  case KEY_X:
    return 'X';
  case KEY_Y:
    return 'Y';
  case KEY_Z:
    return 'Z';
  case KEY_1:
    return '!';
  case KEY_2:
    return '@';
  case KEY_3:
    return '#';
  case KEY_4:
    return '$';
  case KEY_5:
    return '%';
  case KEY_6:
    return '^';
  case KEY_7:
    return '&';
  case KEY_8:
    return '*';
  case KEY_9:
    return '(';
  case KEY_0:
    return ')';
  case KEY_MINUS:
    return '_';
  case KEY_EQUAL:
    return '+';
  case KEY_SQUARE_OPEN_BRACKET:
    return '{';
  case KEY_SQUARE_CLOSE_BRACKET:
    return '}';
  case KEY_SEMICOLON:
    return ':';
  case KEY_BACKSLASH:
    return '|';
  case KEY_COMMA:
    return '<';
  case KEY_DOT:
    return '>';
  case KEY_FORESLHASH:
    return '?';
  case KEY_SPACE:
    return ' ';
  default:
    return 0;
  }
}