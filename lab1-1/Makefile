play:
	gcc -c -m32 mbr.s -o mbr.o
	ld -m elf_i386 -e start -Ttext 0x7c00 mbr.o -o mbr.elf
	objcopy -S -j .text -O binary mbr.elf mbr.bin
	./genboot.pl mbr.bin
	qemu-system-i386 mbr.bin

clean:
	rm mbr.o
	rm mbr.bin 
	rm mbr.elf

