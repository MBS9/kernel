#/bin/sh

qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom image.iso -net nic -net user -monitor stdio -s
