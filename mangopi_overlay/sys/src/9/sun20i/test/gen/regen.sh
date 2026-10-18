#!/bin/sh
# Regenerate the generated tests (host side; the VM has no python).
# Run after editing convgen.py / fpgen.py, then `mk refs` on the VM
# and review the ref diff.
cd `dirname $0`
python3 convgen.py > ../cc/int.c
python3 fpgen.py > ../cc/fp.c
