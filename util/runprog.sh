#!/bin/sh -e
# Helper script to test an EFI executable in a VM

CODE=/usr/share/OVMF/OVMF_CODE_4M.fd
VARS=/usr/share/OVMF/OVMF_VARS_4M.fd

VMDIR=$(dirname $(dirname $0))/vm

if [ ! -d $VMDIR ]; then
	mkdir $VMDIR
fi

if [ ! -e $VMDIR/vars.fd ]; then
	cp $VARS $VMDIR/vars.fd
fi

if [ ! -e $VMDIR/disk.img ]; then
	qemu-img create -f raw $VMDIR/disk.img 200M
	mformat -F -i $VMDIR/disk.img
fi

mmd -D skip -i $VMDIR/disk.img /EFI || true
mmd -D skip -i $VMDIR/disk.img /EFI/BOOT || true
mcopy -D overwrite -i $VMDIR/disk.img $1 ::/EFI/BOOT/BOOTX64.EFI

qemu-system-x86_64 \
	-M q35 \
	-cpu qemu64 \
	-m 128M \
	-drive if=pflash,unit=0,format=raw,file=$CODE,readonly=on \
	-drive if=pflash,unit=1,format=raw,file=$VMDIR/vars.fd \
	-drive if=virtio,format=raw,file=$VMDIR/disk.img
