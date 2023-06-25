#/bin/sh

qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom image.iso -net nic,model=e1000 -net user -monitor stdio -s