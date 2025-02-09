#ifndef IOTOOLS_H
#define IOTOOLS_H

#include <stdint.h>

typedef struct {
    uint16_t ax;
    uint16_t bx;
    uint16_t cx;
    uint16_t dx;
    uint16_t si;
    uint16_t di;
    uint16_t bp;
    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
} regs_t;


unsigned char   port_byte_in(unsigned short port);
void    port_byte_out(unsigned short port, unsigned char data);
unsigned char   port_word_in(unsigned short port);
void port_word_out(unsigned short port, unsigned short data);
void	memcpy(int *src, int *dest, int bytes);
void memset(void *dest, char val, uint32_t count);
void insw(uint16_t port, void *addr, unsigned long count);
void outsw(uint16_t port, const void *addr, unsigned long count);
void insb(uint16_t port, void *addr, unsigned long count);
int k_toupper(int c);
uint32_t port_dword_in(unsigned short port);
void port_dword_out(unsigned short port, uint32_t data);
void reboot_system();
void shutdown_system();
extern unsigned char _insb(unsigned short port);
extern unsigned short _insw(unsigned short port);
uint32_t inportl(uint16_t _port);
void outportl(uint16_t _port, uint32_t _data);
uint16_t inports(uint16_t _port);
void outports(uint16_t _port, uint16_t _data);
void int86(uint8_t int_num, regs_t *regs);
void insl (uint32_t addr, uint32_t buffer, uint32_t count);

#endif