# Aster

![](https://github.com/Michael78Bugaev/aster/blob/main/aster)

The new 32-bit kernel using **GRUB**.

At the latest moment this kernel has:
1. VGA text driver
2. PIT timer
3. SATA disk support (up to 2GB)
4. FAT32 support (in progress)
5. Standard chipset driver
6. PCI support
### How to compile?
To compile, run `$ make`
Warning: the Aster kernel will run on QEMU, you need to
add SATA disk to work with filesystem.
To clear temp files, run `$ make clean`