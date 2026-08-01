# U-Boot binary

Here is provided binary of u-boot capable of running on [MangoPi](https://mangopi.org/mqpro) SBC.

# Installation

Install on SD card ...

    export card=/dev/sdb
    export p=""
    dd if=/dev/zero of=${card} bs=1k count=1023 seek=1
    sudo dd if=/dev/zero of=${card} bs=1k count=1023 seek=1
    sudo dd if=u-boot-sunxi-with-spl.bin of=${card} bs=1024 seek=8

### note

If build from source, make sure to use the correct path to u-boot-sunxi-with-spl.bin.

# Building from source

MangoPi requres a special branch of OpenSBI and U-Boot to enable support for AllWinner D1 SOC.

## OpenSBI

Required: riscv64-linux-gnu-gcc 

Building ...

    git clone https://github.com/riscv-software-src/opensbi
    pushd opensbi
    make CROSS_COMPILE=riscv64-linux-gnu- PLATFORM=generic FW_PIC=y
    popd

## U-Boot (smaeul fork)

Required: opensbi (from previous step),swig and openssl-devel-engine

Building ...
    
    git clone https://github.com/smaeul/u-boot.git smaeul-u-boot
    pushd smaeul-u-boot
    git checkout d1-2022-10-31riscv64-linux-gnu-gcc
    make CROSS_COMPILE=riscv64-linux-gnu- nezha_defconfig
    make CROSS_COMPILE=riscv64-linux-gnu- OPENSBI=../opensbi/build/platform/generic/firmware/fw_dynamic.bin
    popd

### note

U-Boot build also provides mkimage which is used to generate u-boot boot script.
