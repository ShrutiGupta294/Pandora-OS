CC:=i686-elf-gcc
CPP:=i686-elf-g++
LD:=i686-elf-ld
AS:=i686-elf-as
NASM:=nasm


SOURCE:=Source
ASSEMBLY:=$(SOURCE)/ASM
KERNEL:=$(SOURCE)/kernel.o
INCLUDED:=-IKlib -ISource
OBJ:=objs
FLAGS:= -O2 -ffreestanding -fbuiltin -Wall -Wextra -nostdlib -lgcc -fno-builtin -fno-stack-protector $(INCLUDED)
OBJS:=$(OBJ)/*.o

all: clean build-asm build-kernel

clean:
	rm -f build-kernel *.o */*.o */*/*.o
	rm -rfv objs
	rm -rfv isodir

build-asm:
	mkdir -p objs
#	$(NASM) -f elf $(ARCH)/v86.asm -o $(OBJ)/v86.o
	$(NASM) -f elf $(ASSEMBLY)/descriptors.asm -o $(OBJ)/descriptors.o
	$(NASM) -f elf $(ASSEMBLY)/switcher.asm -o $(OBJ)/tasking.o
	$(NASM) -f elf $(ASSEMBLY)/interrupts.asm -o $(OBJ)/interrupts.o
	$(NASM) -f elf $(ASSEMBLY)/FaultHandlers.asm -o $(OBJ)/FaultHandlers.o
#	$(NASM) -f elf $(ARCH)/asmlibrary.asm -o $(OBJ)/asmlibrary.o
#	$(NASM) -f elf $(ARCH)/a.asm -o $(OBJ)/a.o
#	$(NASM) -f elf $(ARCH)/scheduler.asm -o $(OBJ)/scheduler.o
#	$(NASM) -f elf $(ARCH)/AP_Bootup.asm -o $(OBJ)/AP_Bootup.o
#	$(AS) $(SOURCE)/asm/boot.S -o $(OBJ)/boot.o
	$(NASM) -f elf $(ASSEMBLY)/init.asm -o $(OBJ)/init.o
	$(NASM) -f elf $(ASSEMBLY)/AP_Bootup.asm -o $(OBJ)/AP_Bootup.o

build-kernel: $(KERNEL) linker.ld
	$(LD) -T linker.ld -o $(OBJ)/ShrutiOS $(OBJS) $(KERNEL)
	cp $(OBJ)/ShrutiOS ShrutiOS.bin
	make clean
	echo -e "\n\n\n\n\tKernel Built Successfully! Run it using Qemu_ShrutiOS.bat. \n\tBe sure to modify it accordingly!!!"

build-iso:
	sh grub_config.sh
	grub-mkrescue -o ShrutiOS.iso --modules="multiboot normal ext2 part_msdos" isodir

build-installer:
	cd installer
	make
	cd ..

build-assembly:
	$(CC) -T linker.ld $(FLAGS) -S -m32 $(SOURCE)/kernel.c

build-objdump:
	objdump -D -M intel ShrutiOS.bin > ShrutiOS.txt

qemu:
	qemu-system-i386 -smp 4 -m 4G -kernel ShrutiOS.bin -monitor stdio

%.o: %.c
	$(CC) -c $< -o $@ $(FLAGS)

%.o: %.cpp
	$(CPP) -c $< -o $@ $(FLAGS)

%.o: %.asm
	$(NASM)	-f elf $c -o $@

%.o: %.S
	$(AS) $< -o $@
