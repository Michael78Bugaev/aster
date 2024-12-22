OUTPUT_FORMAT(elf32-i386)
ENTRY(start)

SECTIONS
{
  . = 1M;

  .text :
  {
  . = ALIGN(8);
  KEEP(*(.multiboot))
    *(.text*)
    *(.rodata)
  }
  .data  :
  {
    *(.data)
  }

  .bss  :
  {
    *(.bss)
  }
}