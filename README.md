# 9front-mangopi-overlay

An attempt to port 9front to RiscV [MangoPi](https://mangopi.org/mqpro) (Allwinner D1) SBC.

## about

About half of phase 4 of 9 done.

A practical learning project atempting to pring 9front to MangoPi SBC. This is standing on the complimentari project trying to port [riscv compiler to 9front](https://github.com/Nailim/9front-riscv-overlay).

Practicality is questionable, usefulness is peronal, but the reference is real.

To keep things managable a few phases are planned:

* phase 1: ~~kernel skeleton - boot, reach main() and print on something on uart~~
* phase 2: ~~traps - trigger/crash -> capture -> print~~
* phase 3: ~~timers and interrupts - print periodically, print to UART with interrupts~~
* phase 4: MMU - tables, virtual addresses, relocations, ...
* phase 5: scheduler - switch "processes" or print from different threads
* phase 6: console driver - issue commands to kernel
* phase 7: storage driver - read FS from SD card -> try to build and use some of the binaries
* phase 8: network driver - something generic LAN on USB (is this a real system now?)
* phase 9: implemen the rest of the "owl" -> some drivers for I2C would be nice, maybe a frame buffer

## requirements

A 9front installation. Development done on ["GEFS SERVICE PACK 1"](https://9front.org/releases/2026/01/24/0/) release.

A RiscV compiler for plan9. Development done with [9front-riscv-overlay](https://github.com/Nailim/9front-riscv-overlay)

Might work with something else, this is not trying something else.

## usage

Go to mangopi_overlay directory:

`cd mango_overlay`

Run the bind script to bind the overlay to file system:

`./bind.rc`

Then build the kernel:

`cd /sys/src/9/sun201`
`mk`

## notes

Development and testing done on MangoPi SBC.

Good luck with the rest.
