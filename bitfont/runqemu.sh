#!/bin/sh

# -vga [std|cirrus|vmware|qxl|xenfb|tcx|cg3|virtio|none]

qemu-system-x86_64 \
	-machine q35,smm=on,accel=kvm -m 1G -vga qxl \
	-bios /usr/share/ovmf/OVMF.fd \
	-drive format=raw,file=fat:rw:hdd
