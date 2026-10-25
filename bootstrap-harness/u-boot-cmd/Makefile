# This requires mkimage from U-Boot's tools. Either put it in PATH, or set it:
#   make MKIMAGE=../smaeul-u-boot/tools/mkimage
#   export MKIMAGE=... ; make
MKIMAGE ?= mkimage

all: boot/boot.scr

# Check if mkimage is accessible
check-mkimage:
	@command -v "$(MKIMAGE)" >/dev/null 2>&1 || { \
		echo "error: mkimage not found: MKIMAGE='$(MKIMAGE)'"; \
		echo "       set MKIMAGE=/path/to/mkimage or add it to PATH"; \
		exit 1; }

boot/boot.scr: boot.cmd check-mkimage
	@mkdir -p boot
	$(MKIMAGE) -C none -A riscv -T script -d boot.cmd $@

clean:
	rm -f boot/boot.scr

.PHONY: all clean check-mkimage
