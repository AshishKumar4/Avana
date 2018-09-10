#!/bin/bash

mkdir -p isodir

mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp Avana.bin isodir/boot/Avana.bin
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "Avana" {
	multiboot /boot/Avana.bin
	boot
}
EOF
