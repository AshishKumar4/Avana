CC:=i686-linux-gnu-gcc-11
CPP:=i686-linux-gnu-g++-11
LD:=i686-linux-gnu-ld
AS:=i686-linux-gnu-as
NASM:=nasm

SOURCE:=Source
ARCH_x86:=$(SOURCE)/Arch/x86
KLIBRARY:=$(SOURCE)/KLibrary
INCLUDED:=-I$(KLIBRARY) -I$(SOURCE)

KERNEL:=$(SOURCE)/Kernel.o

OBJ_DIR:=$(ARCH_x86)
OBJ_FILES := $(patsubst $(ARCH_x86)/%.asm,$(ARCH_x86)/%.o,$(wildcard $(ARCH_x86)/*.asm))
OBJ_FILES := $(OBJ_FILES) $(patsubst $(ARCH_x86)/%.S,$(ARCH_x86)/%.o,$(wildcard $(ARCH_x86)/*.S)) 

FLAGS:= -fno-pic -O2 -ffreestanding -fbuiltin -Wall -Wextra -nostdlib -lgcc -fno-builtin -fno-stack-protector -m32 $(INCLUDED)

QEMU_CMD:=qemu-system-x86_64 -kernel Avana.bin -m 4G -monitor stdio -drive file=hdd.img,format=raw,if=none,id=hdd0 -device ahci,id=ahci -device ide-hd,drive=hdd0,bus=ahci.0

all: clean build-asm build-kernel

clean:
	rm -f build-kernel *.o */*.o */*/*.o
	rm -rfv objs
	rm -rfv isodir

build-asm: $(OBJ_FILES)
build-kernel: $(KERNEL) linker.ld
	$(LD) -T linker.ld -o Avana.bin $(OBJ_FILES) $(KERNEL)
	make clean
	#echo $(SRC_FILES)
	#echo $(OBJ_FILES) 
	echo -e "\n\n\n\n\tKernel Built Successfully! Run it using "make qemu". \n\tBe sure to modify it accordingly!!!"

build-iso:
	sh grub_config.sh
	grub-mkrescue -o Avana.iso --modules="multiboot normal ext2 part_msdos" isodir

build-assembly:
	$(CC) -T linker.ld $(FLAGS) -S -m32 $(SOURCE)/kernel.c

build-objdump:
	objdump -D -M intel Avana.bin > Avana.txt



buildAndQemu: clean build-asm build-kernel qemu

buildAndQemu1: clean build-asm build-kernel qemu1

qemu_iso:
	qemu-system-x86_64 -smp 1 -m 4G -cdrom Avana.iso -monitor stdio

qemu_d:
	qemu-system-x86_64 -smp 1 -m 4G -kernel Avana.bin -monitor stdio -d cpu_reset

qemu_d2:
	qemu-system-x86_64 -smp 2 -m 4G -kernel Avana.bin -monitor stdio -d cpu_reset

qemu_d4:
	qemu-system-x86_64 -smp 4 -m 4G -kernel Avana.bin -monitor stdio -d cpu_reset

qemu8: 
	$(QEMU_CMD) -smp 8

qemu4: 
	$(QEMU_CMD) -smp 4

qemu1: 
	$(QEMU_CMD) -smp 1

qemu2: 
	$(QEMU_CMD) -smp 2

qemu: qemu4

%.o: %.c
	$(CC) -c $< -o $@ $(FLAGS)

%.o: %.cpp
	$(CPP) -c $< -o $@ $(FLAGS)

%.o: %.asm
	$(NASM)	-f elf $< -o $@

%.o: %.S
	$(AS) $< -o $@
