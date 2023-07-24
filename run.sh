#/bin/sh

E1000_DEBUG=tx,txerr,unknown,eeprom,rx,rxerr /home/me/qemu-mit/bin/qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -cdrom image.iso -net nic,model=e1000 -net user -monitor stdio -s -net dump,file=qemu.pcap