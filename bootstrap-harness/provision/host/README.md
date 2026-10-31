# Host (Linux) side provisioning scripts

Execute on linux.

## Provided scripts

### make-card.sh

    make-card.sh DEV

Sets up u-boot + partition table + FAT32 (3.5 GiB layout). Refuses non-removable devices unless FORCE=1; you retype DEV.

## stage-fat.sh

    stage-fat.sh [DIR]

Checks build/kernel against its SHA1SUM, builds boot.scr, gathers kernel + boot.scr + plan9.ini in build/fat, copies to DIR.


## Notes

Layout: p1 FAT32 sectors 16384+262144 (128 MiB), p2 type 0x39 278528+7061504.
