#!/bin/sh
# make-card.sh DEV   -  DESTROYS everything on DEV.
set -eu
dev=${1:?usage: make-card.sh /dev/sdX}
here=$(cd "$(dirname "$0")" && pwd)
uboot=$here/../../u-boot-bin/u-boot-sunxi-with-spl.bin
name=$(basename "$dev")
[ -b "$dev" ] || { echo "$dev is not a block device" >&2; exit 1; }
[ "$(cat /sys/block/$name/removable 2>/dev/null)" = 1 ] || [ "${FORCE:-}" = 1 ] ||
	{ echo "$dev is not removable (FORCE=1 to override)" >&2; exit 1; }
! grep -q "^$dev" /proc/mounts || { echo "$dev is mounted" >&2; exit 1; }
[ -f "$uboot" ] || { echo "missing $uboot" >&2; exit 1; }
[ "$(sudo blockdev --getsz "$dev")" -ge 7340032 ] || { echo "card smaller than 3.5 GiB" >&2; exit 1; }
lsblk -o NAME,SIZE,MODEL,TRAN "$dev"
printf 'Type %s again to ERASE it: ' "$dev"; read -r again
[ "$again" = "$dev" ] || { echo aborted >&2; exit 1; }
sudo dd if=/dev/zero of="$dev" bs=1M count=16 conv=fsync
cat "$uboot" | sudo dd of="$dev" bs=1024 seek=8 iflag=fullblock conv=fsync
sudo sfdisk "$dev" <<EOF
label: dos
start=16384, size=262144, type=c
start=278528, size=7061504, type=39
EOF
case $dev in *[0-9]) p=p ;; *) p= ;; esac
sudo mkfs.vfat -F 32 -n 9FAT "${dev}${p}1"
sudo sfdisk -d "$dev"
