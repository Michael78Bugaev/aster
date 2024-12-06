# Aster

![](https://github.com/Michael78Bugaev/aster/blob/main/aster.png)

The new 32-bit kernel using **GRUB**.

At the latest moment this kernel has:
1. VGA text driver
2. PIT timer
3. ATA disk support (in progress)
4. FAT32 support (in progress)
5. Standard chipset driver
6. PCI support
# Compiling
First of all you need to clone the repository:
```bash
git clone https://github.com/Michael78Bugaev/aster.git
```
After that, go to the Aster folder and run
```bash
make
```
To clear all temp *.o and *.asmo files, run
```bash
make clean
```

**Warning**: the Aster kernel will run on QEMU, you need to
add ATA disk to work with him.