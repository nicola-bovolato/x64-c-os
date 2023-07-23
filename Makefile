ASM = nasm
LD = /usr/local/x86_64-elf-gcc/bin/x86_64-elf-ld
CC = /usr/local/x86_64-elf-gcc/bin/x86_64-elf-gcc
GDB = /usr/local/x86_64-elf-gcc/bin/x86_64-elf-gdb

ASMFLAGS = -f elf64 -I src/boot
LDFLAGS  = --nmagic # Disables automatic section alignment
CCFLAGS  = -std=c99 -ffreestanding -mno-red-zone -nostdlib -O0 -Wall -Wextra -fno-asynchronous-unwind-tables

ASM_SRC   := $(wildcard src/boot/*.asm)
ASM_OBJ   := $(patsubst src/%.asm, build/%.o, $(ASM_SRC))
C_HEADERS := $(wildcard src/kernel/*.h src/kernel/**/*.h)
C_SRC	  := $(wildcard src/kernel/*.c src/kernel/**/*.c)
C_OBJ     := $(patsubst src/%.c, build/%.o, $(C_SRC))
GRUB_CFG  := src/boot/grub.cfg
LDFILE    := src/boot/linker.ld

SYMBOL 	  := kernel.elf
KERNEL 	  := build/kernel.bin
ISO 	  := kernel.iso

.PHONY: iso run debug gdb clean

# Builds the kernel binary by linking required objects
$(KERNEL): $(LDFILE) $(ASM_OBJ) $(C_OBJ)
	mkdir -p $(@D)
	$(LD) $(LDFLAGS) -T $(LDFILE) -o $(KERNEL) $(ASM_OBJ) $(C_OBJ)

$(SYMBOL): $(LDFILE) $(ASM_OBJ) $(C_OBJ)
	mkdir -p $(@D)
	$(LD) $(LDFLAGS) -T $(LDFILE) -o $(SYMBOL) $(ASM_OBJ) $(C_OBJ)

# Builds a bootable image of the kernel using grub
$(ISO): $(KERNEL) $(GRUB_CFG)
	mkdir -p build/isofiles/boot/grub
	cp $(KERNEL) build/isofiles/boot/kernel.bin
	cp $(GRUB_CFG) build/isofiles/boot/grub
	grub-mkrescue -o $(ISO) build/isofiles 2> /dev/null

# Bootable image target
iso: $(ISO)

# Runs qemu
run: $(ISO)
	qemu-system-x86_64 -cdrom $<

# Runs qemu and enables debugging
debug: CCFLAGS += -g
debug: LDFILE := $(LDFILE).dbg
debug: $(ISO) $(SYMBOL)
	qemu-system-x86_64 -cdrom $< -s -S

# Launches gdb
gdb:
	$(GDB) $(SYMBOL) -ex "target remote localhost:1234" -ex "continue"

# Removes build files
clean:
	rm -rf build
	rm -f $(KERNEL)
	rm -f $(SYMBOL)
	rm -f $(ISO)

# Builds assembly object files
build/%.o: src/%.asm
	mkdir -p $(@D)
	$(ASM) $(ASMFLAGS) $< -o $@

# Builds c object files
build/%.o: src/%.c $(CHEADERS)
	mkdir -p $(@D)
	$(CC) $(CCFLAGS) -c $< -o $@
