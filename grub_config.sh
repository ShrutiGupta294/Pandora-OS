#!/bin/bash

mkdir -p isodir

mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp ShrutiOS.bin isodir/boot/ShrutiOS.bin
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "ShrutiOS" {
	multiboot /boot/ShrutiOS.bin
	boot
}
EOF
