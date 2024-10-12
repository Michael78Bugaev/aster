#pragma once

typedef unsigned long        size_t;

int strlen(char s[]);
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