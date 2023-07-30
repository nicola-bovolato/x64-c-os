TARGET    ?=

ASM 	  := nasm
LD 		  := /usr/local/x86_64-elf-gcc/bin/x86_64-elf-ld
CC 		  := /usr/local/x86_64-elf-gcc/bin/x86_64-elf-gcc
GDB 	  := /usr/local/x86_64-elf-gcc/bin/x86_64-elf-gdb

ASMFLAGS  := -f elf64 -I src/boot
LDFLAGS   := --nmagic # Disables automatic section alignment
CCFLAGS   := -Wall -Wextra -std=c99 -pedantic -ffreestanding -nostdlib -mno-red-zone -fno-asynchronous-unwind-tables -O0

SRCDIR     = src
OUTDIR     = build/$(TARGET)

ASM_SRC    = $(shell find src/ -type f -name '*.asm')
ASM_OBJ    = $(patsubst $(SRCDIR)/%.asm, $(OUTDIR)/%.o, $(ASM_SRC))
C_HEADERS  = $(shell find src/ -type f -name '*.h')
C_SRC	   = $(shell find src/ -type f -name '*.c')
C_OBJ      = $(patsubst $(SRCDIR)/%.c, $(OUTDIR)/%.o, $(C_SRC))
GRUB_CFG   = src/boot/grub.cfg
LDFILE     = src/boot/linker.$(TARGET).ld

KERNEL 	   = $(OUTDIR)/kernel.elf
ISO 	   = $(OUTDIR)/kernel.iso

# This clunky, recursive thing is needed to expand $TARGET when used in wildcard targets
ifndef TARGET

.PHONY: all run debug gdb clean --iso --kernel

all: TARGET := release
all: --iso

# Runs qemu
run: TARGET := release
run: --iso
	qemu-system-x86_64 -cdrom $(ISO)

# Runs qemu and enables debugging
debug: TARGET := debug
debug: CCFLAGS += -g -DDEBUG
debug: --iso
	$(info $(ASM_OBJ))
	qemu-system-x86_64 -cdrom $(ISO) -s # --no-reboot -d int

# Attaches gdb to qemu
gdb: TARGET := debug
gdb: CCFLAGS += -g -DDEBUG
gdb: --kernel
	$(GDB) $(KERNEL) -ex "target remote localhost:1234"

# Removes build files
clean:
	rm -rf build

--iso:
	@$(MAKE) iso TARGET=$(TARGET) CCFLAGS="$(CCFLAGS)"

--kernel:
	@$(MAKE) kernel TARGET=$(TARGET) CCFLAGS="$(CCFLAGS)"

else

.PHONY: iso symbol

# Bootable image target
iso: $(ISO)

# Kernel binary
kernel: $(KERNEL)

# Builds assembly object files
$(OUTDIR)/%.o: $(SRCDIR)/%.asm
	mkdir -p $(@D)
	$(ASM) $(ASMFLAGS) $< -o $@

# Builds c object files
$(OUTDIR)/%.o: $(SRCDIR)/%.c $(C_HEADERS)
	mkdir -p $(@D)
	$(CC) $(CCFLAGS) -c $< -o $@

# Builds the kernel binary by linking required objects
$(KERNEL): $(LDFILE) $(ASM_OBJ) $(C_OBJ)
	mkdir -p $(@D)
	$(LD) $(LDFLAGS) -T $(LDFILE) -o $(KERNEL) $(ASM_OBJ) $(C_OBJ)

# Builds a bootable image of the kernel using grub
$(ISO): $(KERNEL) $(GRUB_CFG)
	mkdir -p $(OUTDIR)/isofiles/boot/grub
	cp $(KERNEL) $(OUTDIR)/isofiles/boot/kernel.elf
	cp $(GRUB_CFG) $(OUTDIR)/isofiles/boot/grub
	grub-mkrescue -o $(ISO) $(OUTDIR)/isofiles 2> /dev/null
	rm -rf $(OUTDIR)/isofiles/boot/grub

endif
