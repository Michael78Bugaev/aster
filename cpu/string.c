#include <string.h>
#include <vga.h>
#include <stdint.h>
#include <cpu/mem.h>

typedef unsigned long        size_t;

int strlen(char s[]) {
    int i = 0;
    while (s[i] != '\0') {
        ++i;
    }
    return i;
}
char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;

    // Copy up to n characters from src to dest
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }

    // If we copied fewer than n characters, pad the rest of dest with null bytes
    for (; i < n; i++) {
        dest[i] = '\0';
    }

    return dest;
}
void strncat(char *s, char c)
{
  int len = strlen(s);
  s[len] = c;
  s[len + 1] = '\0';
}

char *strcat(char *s, char *append)
{
	char *save = s;

	for (; *s; ++s);
	while (*s++ = *append++);
	return(save);
}

// compare string
int strcmp(char s1[], char s2[]) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') return 0;
    }
    return s1[i] - s2[i];
}

char *mem_set(char *dest, int val)
{
  unsigned char *ptr = dest;

  size_t len = strlen(dest);

  while (len-- > 0)
    *ptr++ = val;
  return dest;
}
void join(char s[], char n) {
    int len = strlen(s);
    s[len] = n;
    s[len + 1] = '\0';
}
void memcp(char *source, char *dest, int nbytes) {
    int i;
    for (i = 0; i < nbytes; i++) {
        *(dest + i) = *(source + i);
    }
}

void clearString(char *string)
{

  for (int i = strlen(string); i = 0; i--)
  {

    string[i] = 0x00;
  }
}

void reverse(char s[]) {
    int c, i, j;
    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void intToString(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) str[i++] = '-';
    str[i] = '\0';

    reverse(str);
}
int startsWith(char s1[], char s2[]) {
    int i;
    for (i = 0; s2[i] != '\0'; i++) {
        if (s1[i] != s2[i]) return 0;
    }
    return 1;
}

void strnone(char *str)
{
    for (int i = 0; i < strlen(str); i++)
    {
        str[i] = 0;
    }
    
}

char **splitString(const char *str, int *count) {
    // Сначала подсчитаем количество слов
    int n = 0;
    const char *ptr = str;
    while (*ptr) {
        // Пропускаем пробелы
        while (*ptr == ' ') {
            ptr++;
        }
        if (*ptr) {
            n++; // Нашли слово
            // Пропускаем само слово
            while (*ptr && *ptr != ' ') {
                ptr++;
            }
        }
    }

    // Выделяем память для массива строк
    char **result = malloc(n * sizeof(char *));
    if (!result) {
        kprint("Error memory allocation!\n");
        return NULL; // Ошибка выделения памяти
    }

    // Заполняем массив словами
    int index = 0;
    ptr = str;
    while (*ptr) {
        // Пропускаем пробелы
        while (*ptr == ' ') {
            ptr++;
        }
        if (*ptr) {
            const char *start = ptr;
            // Находим конец слова
            while (*ptr && *ptr != ' ') {
                ptr++;
            }
            // Выделяем память для слова и копируем его
            int length = ptr - start;
            result[index] = malloc((length + 1) * sizeof(char));
            if (!result[index]) {
                // Освобождаем ранее выделенную память в случае ошибки
                for (int j = 0; j < index; j++) {
                    mfree(result[j]);
                }
                mfree(result);
                return NULL; // Ошибка выделения памяти
            }
            strncpy(result[index], start, length);
            result[index][length] = '\0'; // Завершаем строку нулевым символом
            index++;
        }
    }

    *count = n; // Возвращаем количество найденных слов
    return result;
}

size_t strnlen(const char *s, size_t maxlen) {
    size_t len;
    for (len = 0; len < maxlen; len++) {
        if (s[len] == '\0') {
            break;
        }
    }
    return len;
}

int isdigit(int c) {
    return (c >= '0' && c <= '9');
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = s1, *p2 = s2;
    
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    
    return 0;
}

int atoi(const char *str);
int atoi(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    // Handle whitespace
    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') {
        i++;
    }

    // Handle sign
    if (str[i] == '-' || str[i] == '+') {
        sign = (str[i] == '-') ? -1 : 1;
        i++;
    }

    // Process digits
    while (str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + (str[i] - '0');
        i++;
    }

    return sign * result;
}

int to_integer(const char* str) {
    return atoi(str);
}

#define MAX_HISTORY 1024
#define MAX_COMMAND_LENGTH 1024