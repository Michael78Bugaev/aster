// kernel.c
#include <vga.h>
#include <cpu/gdt.h>
#include <io/idt.h>
#include <cpu/mem.h>
#include <cpu/pit.h>
#include <multiboot.h>
#include <fs/initrd.h>
#include <drv/ata.h>
#include <stdio.h>
#include <drv/pci.h>
#include <config.h>
#include <chset/chipset.h>
#include <io/kb.h>
#include <sash.h>
#include <progress.h>

void kentr(uint32_t magic, struct multiboot_info* boot_info) {

    start_global_config();
    // Инициализация GDT
    init_gdt();
    // Инициализация IDT
    init_idt();
    // Инициализация PIT
    init_pit();
    // Инициализация памяти
    init_dmem();

    init_chipset();
    pci_init();
    // sata_init();
    // sata_init_all_disks();
    ata_init();
    //fat_init();

    //uint8_t buffer[512];
    // sata_read_sector(0, buffer);
    //ataWrite(PRIMARY_SLAVE, 1, 1, "H e l l o ,   a s t e r ! ");
    // ide_write_sectors(0, 1, 1, 0xDEAD);
    // ide_read_sectors(0, 1, 1, buffer);
    // // Печатаем текстовое представление
    // printf(" ");
    // for (int i = 0; i < 256; i++) {
    //     if (i % 16 == 0)
    //     {
    //         printf("\n");
    //     }
    //     // Если байт является печатным символом, выводим его, иначе выводим точку
    //     printf("%2x ", buffer[i]);
    // }
    // printf("\n"); 

    init_vfs();
    printf("\n");
    uint8_t init[] = "clear\nverinfo\n";
    File *init_src = new_file("/init", init, sizeof(init));

    execute_init("/init");

    init_multiboot(boot_info);

    printf("\n%s &", current_directory->name);
    irq_install_handler(1, &sash);
}