#include <stdint.h>
#include <string.h>
#include <multiboot.h>
#include <stdio.h>

void init_multiboot(struct multiboot_info* mboot_info)
{
    for (int i = 0; i < mboot_info->mmap_length; i += sizeof(struct multiboot_mmap_entry))
    {
        struct multiboot_mmap_entry *mmmt = (struct multiboot_mmap_entry*)(mboot_info->mmap_addr + 1);

        printf("Low addr: 0x%x\nHigh addr: 0x%x\nLow length: 0x%x\nHigh length: 0x%x\nSize: %d\nType: %d\n\n", 
                mmmt->addr_low, mmmt->addr_high, mmmt->len_low, mmmt->len_high, mmmt->size, mmmt->type);
    }
}