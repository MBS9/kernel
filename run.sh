#/bin/sh

qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom image.iso -net user -net nic,model=i82559er -enable-kvm -monitor stdio -s
