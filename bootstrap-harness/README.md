# How to boot a compiled 9sun20i kernel

A SD card with installed (correct) u-boot and a small FAT partition is required.

Once installed, copy the 9sun20i binary to the root of the FAT partition, power on Mango Pi board with the SD card inserted.

For debugging purpuses it helpts if a UART is connected to the board.


## Installed prepared u-boot binary and script

### 1: install u-boot binary

Prepared u-boot binary is provided in u-boot-bin directory. Reference README.md inside.

### 2: create a FAT partition

Reference [sunxi wiki](https://linux-sunxi.org/Bootable_SD_card#Bootloader) for boot partition instructions.

TLDR: create a 16MB FAT partition 1MB from the beginning of the SD card.

### 3: install u-boot boot script

Prepared u-boot script to load and execute 9sun20i kernel is provided in u-boot-cmd. Reference README.md inside.

