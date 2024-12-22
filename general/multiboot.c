#include <stdint.h>
#include <string.h>
#include <multiboot.h>
#include <stdio.h>

void init_multiboot(struct multiboot_info* mboot_info)
{
    for (int i = 0; i < mboot_info->mmap_length; i += sizeof(struct multiboot_mmap_entry))
    {
        struct multiboot_mmap_entry *mmmt = (struct multiboot_mmap_entry*)(mboot_info->mmap_addr);

        printf("Low addr: 0x%x High addr: 0x%x Low length: 0x%x High length: 0x%x Size: %d Type: %d\n", 
                mmmt->addr_low, mmmt->addr_high, mmmt->len_low, mmmt->len_high, mmmt->size, mmmt->type);
    }
}