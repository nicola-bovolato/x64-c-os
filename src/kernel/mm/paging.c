#include "paging.h"
#include "../log.h"
#include "frame.h"
#include "multiboot2.h"

#define LOWER_HALF_TOP_ADDRESS     (0x0000800000000000 - 1)
#define HIGHER_HALF_BOTTOM_ADDRESS 0xffff800000000000

typedef void* (*allocate_frame_t)();
typedef void (*deallocate_frame_t)(void* frame);

// The last entry of the page map level4 table is mapped to itself in boot.asm
static const page_table_t* table4_ptr = (page_table_t*)0xfffffffffffff000;

static inline size_t get_table4_index(page_entry_t entry);
static inline size_t get_table3_index(page_entry_t entry);
static inline size_t get_table2_index(page_entry_t entry);
static inline size_t get_table1_index(page_entry_t entry);

static inline void zero_table_entries(page_table_t* table);
static inline void flush_tlb_page(page_entry_t page);
static inline void flush_tlb();

static inline page_table_t* next_table(page_table_t* table, size_t index);
static inline page_table_t*
get_or_create_next_table(page_table_t* table, size_t index, allocate_frame_t allocate_frame);

#define MAX_TEMP_ALLOCATOR_FRAMES 3
typedef struct {
    void* frames[MAX_TEMP_ALLOCATOR_FRAMES];
} temp_allocator_t;

temp_allocator_t temp_allocator;

static inline void init_temp_allocator() {
    for (int i = 0; i < MAX_TEMP_ALLOCATOR_FRAMES; i++) temp_allocator.frames[i] = allocate_frame();
}

static inline void* allocate_temp_frame() {
    for (int i = 0; i < MAX_TEMP_ALLOCATOR_FRAMES; i++) {
        if (temp_allocator.frames[i] != (void*)-1) {
            temp_allocator.frames[i] = (void*)-1;
            return temp_allocator.frames[i];
        }
    }
    PANIC("No available frames on temp allocator");
    return (void*)-1;
}

static inline void deallocate_temp_frame(void* frame) {
    for (int i = 0; i < MAX_TEMP_ALLOCATOR_FRAMES; i++) {
        if (temp_allocator.frames[i] == (void*)-1) {
            temp_allocator.frames[i] = frame;
            return;
        }
    }
    PANIC("No allocated frames on temp allocator");
}

void init_paging(uint32_t* multiboot_header) {
    init_multiboot_info(multiboot_header);
    init_frame_allocator();
    init_temp_allocator();

    page_entry_t temp_page   = {.bits = 0};
    temp_page.fields.address = 0xdeadbeef;

    // page_table_t* new_table4_ptr = allocate_frame

    // TODO:
    // - Create a new temporary pml4 table
    // - Identity map the kernel (using the correct page flags for each elf section)
    // - Add a guard page below the kernel stack to prevent stack overflows
    // - Move the temporary table to the new address
    // - Use the new table as the main one
}

void* get_physical_address(page_table_t* table4_ptr, void* virtual) {

    if ((size_t) virtual > LOWER_HALF_TOP_ADDRESS
        && (size_t) virtual < HIGHER_HALF_BOTTOM_ADDRESS) {
        // Invalid address
        PANIC("Virtual address not in allowed memory");
    }

    size_t offset = (size_t) virtual % FRAME_SIZE;

    page_entry_t page   = {.bits = 0};
    page.fields.address = (size_t) virtual / FRAME_SIZE;

    page_table_t* table3_ptr = next_table(table4_ptr, get_table4_index(page));
    if (table3_ptr == (void*)-1) {
        DEBUG("(get_physical_address) Page table 3 is empty\n");
        return (void*)-1;
    }

    page_table_t* table2_ptr = next_table(table3_ptr, get_table3_index(page));
    if (table2_ptr == (void*)-1) {
        DEBUG("(get_physical_address) Page table 2 is empty\n");
        return (void*)-1;
    }

    page_table_t* table1_ptr = next_table(table2_ptr, get_table2_index(page));
    if (table1_ptr == (void*)-1) {
        DEBUG("(get_physical_address) Page table 1 is empty\n");
        return (void*)-1;
    }

    page_entry_t table1_entry = table1_ptr->entries[get_table1_index(page)];
    if (!table1_entry.fields.present) {
        DEBUG("(get_physical_address) Page table 1 entry is empty\n");
        return (void*)-1;
    }

    return (void*)(table1_entry.fields.address * FRAME_SIZE + offset);
}

void map_page_to_frame(
    page_table_t* table4_ptr, page_entry_t page, uint8_t* frame_ptr, allocate_frame_t allocate_frame
) {

    page_table_t* table3_ptr
        = get_or_create_next_table(table4_ptr, get_table4_index(page), allocate_frame);
    page_table_t* table2_ptr
        = get_or_create_next_table(table3_ptr, get_table3_index(page), allocate_frame);
    page_table_t* table1_ptr
        = get_or_create_next_table(table2_ptr, get_table2_index(page), allocate_frame);

    page_entry_t* table1_entry = &(table1_ptr->entries[get_table1_index(page)]);

    if (table1_entry->bits != 0)
        PANIC("Page %x is already in use (points to %x)", page.bits, table1_entry->bits);
    table1_entry->fields.present = true;
    table1_entry->fields.address = (size_t)frame_ptr / FRAME_SIZE;
}

void unmap_page(page_entry_t page, page_table_t* table4_ptr, deallocate_frame_t deallocate_frame) {
    page_table_t* table3_ptr = next_table(table4_ptr, get_table4_index(page));
    if (table3_ptr == (void*)-1) {
        DEBUG("(unmap_page) Page table 3 is empty\n");
        return;
    }

    page_table_t* table2_ptr = next_table(table3_ptr, get_table3_index(page));
    if (table2_ptr == (void*)-1) {
        DEBUG("(unmap_page) Page table 2 is empty\n");
        return;
    }

    page_table_t* table1_ptr = next_table(table2_ptr, get_table2_index(page));
    if (table1_ptr == (void*)-1) {
        DEBUG("(unmap_page) Page table 1 is empty\n");
        return;
    }

    int table1_index = get_table1_index(page);

    void* frame_ptr
        = (void*)((size_t)table1_ptr->entries[table1_index].fields.address * FRAME_SIZE);

    table1_ptr->entries[table1_index].bits = 0;
    flush_tlb_page(page);

    deallocate_frame(frame_ptr);
}

static inline size_t get_table4_index(page_entry_t entry) {
    return (entry.fields.address >> 27) & 0777;
}

static inline size_t get_table3_index(page_entry_t entry) {
    return (entry.fields.address >> 18) & 0777;
}

static inline size_t get_table2_index(page_entry_t entry) {
    return (entry.fields.address >> 9) & 0777;
}

static inline size_t get_table1_index(page_entry_t entry) {
    return (entry.fields.address >> 0) & 0777;
}

static inline void zero_table_entries(page_table_t* table) {
    for (int i = 0; i < PAGE_ENTRIES; i++) table->entries[i].bits = 0;
}

static inline void flush_tlb_page(page_entry_t page) {
    DEBUG("Flusing TLB for page %x\n", page.bits);
    __asm__ volatile("invlpg (%0)" ::"r"(page.bits) : "memory");
}

static inline void flush_tlb() {
    uint64_t cr3;
    DEBUG("Flusing whole TLB (cr3 register)\n");
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    DEBUG("Cr3 contains: %p\n", cr3);
    __asm__ volatile("mov %0, %%cr3" ::"r"(cr3) : "memory");
}

static inline page_table_t* next_table(page_table_t* table, size_t index) {
    page_entry_t* entry = &(table->entries[index]);
    if (!entry->fields.present) {
        return (void*)-1;
    }
    if (entry->fields.huge_page)
        PANIC("Huge pages not supported! (Table entry %d is a huge page)", entry->bits);
    return (page_table_t*)(((size_t)table << 9) | (index << 12));
}

static inline page_table_t*
get_or_create_next_table(page_table_t* table, size_t index, allocate_frame_t allocate_frame) {

    // if the next table does not exist, allocate a frame for a new one
    if (next_table(table, index) == (void*)-1) {

        table->entries[index].fields.address  = (size_t)allocate_frame() / FRAME_SIZE;
        table->entries[index].fields.present  = true;
        table->entries[index].fields.writable = true;
        zero_table_entries(next_table(table, index));
    }

    return next_table(table, index);
}
