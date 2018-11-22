CC:=gcc
LD:=i686-elf-ld
AS:=i686-elf-as
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

qemu_iso:
	qemu-system-x86_64 -smp 1 -m 4G -cdrom Avana.iso -monitor stdio

qemu_d:
	qemu-system-x86_64 -smp 1 -m 4G -kernel Avana.bin -monitor stdio -d cpu_reset

qemu_d2:
	qemu-system-x86_64 -smp 2 -m 4G -kernel Avana.bin -monitor stdio -d cpu_reset

qemu_d4:
	qemu-system-x86_64 -smp 4 -m 4G -kernel Avana.bin -monitor stdio -d cpu_reset

qemu4: 
	qemu-system-x86_64 -kernel Avana.bin -smp 4 -m 4G -monitor stdio -drive format=raw,file=hdd.img,if=none,id=hdd0 -device ahci,id=ahci -device ide-drive,drive=hdd0,bus=ahci.1

qemu8: 
	qemu-system-x86_64 -kernel Avana.bin -smp 8 -m 4G -monitor stdio -drive format=raw,file=hdd.img,if=none,id=hdd0 -device ahci,id=ahci -device ide-drive,drive=hdd0,bus=ahci.1

qemu1: 
	qemu-system-x86_64 -kernel Avana.bin -smp 1 -m 4G -monitor stdio -drive format=raw,file=hdd.img,if=none,id=hdd0 -device ahci,id=ahci -device ide-drive,drive=hdd0,bus=ahci.1

qemu2: 
	qemu-system-x86_64 -kernel Avana.bin -smp 2 -m 4G -monitor stdio -drive format=raw,file=hdd.img,if=none,id=hdd0 -device ahci,id=ahci -device ide-drive,drive=hdd0,bus=ahci.1

qemu: 
	make qemu4

%.o: %.c
	$(CC) -c $< -o $@ $(FLAGS)

%.o: %.cpp
	$(CPP) -c $< -o $@ $(FLAGS)

%.o: %.asm
	$(NASM)	-f elf $< -o $@

%.o: %.S
	$(AS) $< -o $@
