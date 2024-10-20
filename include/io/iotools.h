#ifndef IOTOOLS_H
#define IOTOOLS_H

#include <stdint.h>

unsigned char   port_byte_in(unsigned short port);
void    port_byte_out(unsigned short port, unsigned char data);
unsigned char   port_word_in(unsigned short port);
void port_word_out(unsigned short port, unsigned short data);
void	memcpy(int *src, int *dest, int bytes);
void memset(void *dest, char val, uint32_t count);
void insw(uint16_t port, void *addr, unsigned long count);
void outsw(uint16_t port, const void *addr, unsigned long count);
int k_toupper(int c);

#endif