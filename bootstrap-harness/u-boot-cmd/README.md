# U-Boot boot script

U-Boot requires a boot script on the first partition with instructions what/where to load and execution.

Script is written in text, e.g. _boot.cmd_, but needs to be in u-boot script format, e.g. _boot.scr_.

Script is generated with mkimage tool provided by U-Boot. Check README.md in u-boot-bin for more infor on building that.

# Instalation

Copy boot folder containig boot.scr to the root of the boot partition on the SD card.

# Boot script generation

Again, have correct mkimage installed or in path or provide path to!

Generate boot script from boot commands:

    mkimage -C none -A riscv -T script -d boot.cmd boot.scr

