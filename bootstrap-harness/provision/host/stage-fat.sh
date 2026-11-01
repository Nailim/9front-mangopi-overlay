#!/bin/sh
# stage-fat.sh [MOUNTPOINT]
set -eu
here=$(cd "$(dirname "$0")" && pwd)
root=$here/../..
f=$here/../build/fat
rm -rf "$f"; mkdir -p "$f/boot"
k=$here/../build/kernel/9sun20i
[ -f "$k" ] || { echo "missing $k: mount the overlay from the VM and run vm/build-kernel.rc first" >&2; exit 1; }
cp "$k" "$f/9sun20i"
cp "$root/u-boot-cmd/boot/boot.scr" "$f/boot/boot.scr"
cp "$root/plan9-config/plan9.ini" "$f/plan9.ini"
ls -lR "$f"
if [ $# -ge 1 ]; then cp -r "$f"/. "$1"/ && sync; fi
