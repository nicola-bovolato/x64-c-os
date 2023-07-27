/*
** More information on the multiboot2 specification here:
** https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html
*/

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include "memlayout.h"
#include <stddef.h>
#include <stdint.h>

void init_multiboot_info(uint32_t* address);

size_t get_used_mem_regions(mem_region_t* regions);

mem_region_t get_system_mem_region();
mem_region_t get_kernel_mem_region();
mem_region_t get_multiboot_mem_region();

// Available multiboot info tags
#define MULTIBOOT_TAG_TYPE_END              0
#define MULTIBOOT_TAG_TYPE_CMDLINE          1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME 2
#define MULTIBOOT_TAG_TYPE_MODULE           3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO    4
#define MULTIBOOT_TAG_TYPE_BOOTDEV          5
#define MULTIBOOT_TAG_TYPE_MMAP             6
#define MULTIBOOT_TAG_TYPE_VBE              7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER      8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS     9
#define MULTIBOOT_TAG_TYPE_APM              10
#define MULTIBOOT_TAG_TYPE_EFI32            11
#define MULTIBOOT_TAG_TYPE_EFI64            12
#define MULTIBOOT_TAG_TYPE_SMBIOS           13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD         14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW         15
#define MULTIBOOT_TAG_TYPE_NETWORK          16
#define MULTIBOOT_TAG_TYPE_EFI_MMAP         17
#define MULTIBOOT_TAG_TYPE_EFI_BS           18
#define MULTIBOOT_TAG_TYPE_EFI32_IH         19
#define MULTIBOOT_TAG_TYPE_EFI64_IH         20
#define MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR   21

#define MULTIBOOT_HEADER_TAG_END 0

// Multiboot Structures

typedef struct {
    uint64_t base_addr;
    uint64_t length;
#define MULTIBOOT_MEMORY_AVAILABLE        1
#define MULTIBOOT_MEMORY_RESERVED         2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS              4
#define MULTIBOOT_MEMORY_BADRAM           5
    uint32_t type;
    uint32_t _; // reserved
} multiboot_mmap_entry_t;

typedef struct {
    uint32_t name;
#define MULTIBOOT_ELF_SECTION_UNUSED                      0
#define MULTIBOOT_ELF_SECTION_PROGRAM_SECTION             1
#define MULTIBOOT_ELF_SECTION_LINKER_SYMBOL_TABLE         2
#define MULTIBOOT_ELF_SECTION_STRING_TABLE                3
#define MULTIBOOT_ELF_SECTION_RELA_RELOCATION             4
#define MULTIBOOT_ELF_SECTION_SYMBOL_HASH_TABLE           5
#define MULTIBOOT_ELF_SECTION_DYNAMIC_LINKING_TABLE       6
#define MULTIBOOT_ELF_SECTION_NOTE                        7
#define MULTIBOOT_ELF_SECTION_UNINITIALIZED               8
#define MULTIBOOT_ELF_SECTION_REL_RELOCATION              9
#define MULTIBOOT_ELF_SECTION_RESERVED                    10
#define MULTIBOOT_ELF_SECTION_DYNAMIC_LOADER_SYMBOL_TABLE 11
    uint32_t type;
    uint64_t flags;
    uint64_t address;
    uint64_t file_offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t address_align;
    uint64_t entry_size;
} multiboot_elf_section_t;

typedef struct {
    uint32_t type;
    uint32_t size;
} multiboot_tag_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
} multiboot_tag_basic_meminfo_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t biosdev;
    uint32_t slice;
    uint32_t part;
} multiboot_tag_bootdev_t;

typedef struct {
    uint32_t               type;
    uint32_t               size;
    uint32_t               entry_size;
    uint32_t               entry_version;
    multiboot_mmap_entry_t entries[];
} multiboot_tag_mmap_t;

typedef struct {
    uint32_t                type;
    uint32_t                size;
    uint32_t                num;
    uint32_t                entsize;
    uint32_t                shndx;
    multiboot_elf_section_t sections[];
} multiboot_tag_elf_sections_t;

#endif
