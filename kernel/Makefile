all : hank_os.bin

hank_os.bin : bootstrap.bin kernel.bin
	cat ./obj_bin/bootstrap.bin ./obj_bin/kernel.bin ./obj_bin/extra_padding_debug.bin > ./binary/hank_os.bin

bootstrap.bin : ./boot/bootstrap.asm
	nasm -f bin -o ./obj_bin/bootstrap.bin ./boot/bootstrap.asm

kernel.bin : kernel_entry.o kernel.o
	ld -o ./obj_bin/kernel.bin -Ttext 0x1000 ./obj_bin/kernel_entry.o ./obj_bin/kernel.o --oformat binary

kernel.o : ./kernel/kernel.c
	gcc -ffreestanding -c ./kernel/kernel.c -o ./obj_bin/kernel.o

kernel_entry.o : ./kernel/kernel_entry.asm
	nasm -f elf64 -o ./obj_bin/kernel_entry.o ./kernel/kernel_entry.asm

clean :
	rm  *.bin *.o
