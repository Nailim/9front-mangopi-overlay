# 9front-mangopi-overlay
An attempt to port 9front to RiscV [MangoPi](https://mangopi.org/mqpro) (Allwinner D1) SBC.

## about

About 0% done.

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
