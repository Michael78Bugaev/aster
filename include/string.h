#pragma once

typedef unsigned long        size_t;

int strlen(char s[]);
char *strncpy(char *dest, const char *src, size_t n);
void strncat(char *s, char c);
int strcmp(char s1[], char s2[]);
char *mem_set(char *dest, int val);
void join(char s[], char n);
void memcp(char *source, char *dest, int nbytes);
void clearString(char *string);
void reverse(char s[]);
void intToString(int n, char str[]);
char *strcat(char* s, char* append);
int startsWith(char s1[], char s2[]);
void strnone(char *str);
char **splitString(const char *str, int *count);
size_t strnlen(const char *s, size_t maxlen);
int isdigit(int c);
int memcmp(const void *s1, const void *s2, size_t n);
int to_integer(const char* str);