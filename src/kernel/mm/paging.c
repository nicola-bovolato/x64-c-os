#include "paging.h"
#include "../log.h"
#include "frame.h"
#include "multiboot2.h"

#define LOWER_HALF_TOP_ADDRESS     (0x0000800000000000 - 1)
#define HIGHER_HALF_BOTTOM_ADDRESS 0xffff800000000000

typedef void* (*allocate_frame_t)();
typedef void (*deallocate_frame_t)(void* frame);

// The last entry of the page map level4 table is mapped to itself in boot.asm
static page_table_t* table4_ptr = (page_table_t*)0xfffffffffffff000;

static inline size_t get_table4_index(page_t entry);
static inline size_t get_table3_index(page_t entry);
static inline size_t get_table2_index(page_t entry);
static inline size_t get_table1_index(page_t entry);

static inline void     zero_table_entries(page_table_t* table);
static inline void     flush_tlb_page(page_t page);
static inline void     flush_tlb();
static inline uint64_t read_cr3();

static inline page_table_t* next_table(page_table_t* table, size_t index);
static inline page_table_t*
get_or_create_next_table(page_table_t* table, size_t index, allocate_frame_t allocate_frame);

static inline void  init_temp_allocator();
static inline void* allocate_temp_frame();
static inline void  deallocate_temp_frame(void* frame);

void identity_map(
    page_table_t*    table4_ptr,
    uint16_t         page_flags,
    uint8_t*         frame_ptr,
    allocate_frame_t allocate_frame
);
void map_page_to_frame(
    page_table_t*    table4_ptr,
    page_t           page,
    uint16_t         page_flags,
    uint8_t*         frame_ptr,
    allocate_frame_t allocate_frame
);
void unmap_page(
    page_table_t* table4_ptr, page_t page, deallocate_frame_t deallocate_frame, bool panic_on_empty
);

void* get_physical_address(page_table_t* table4_ptr, void* virtual);

// temp allocator is used to map one page at a time
// since a page requires 4 tables and the root will be already mapped 4-1 = 3 frames
#define MAX_TEMP_ALLOCATOR_FRAMES 3
typedef struct {
    void* frames[MAX_TEMP_ALLOCATOR_FRAMES];
} temp_allocator_t;

temp_allocator_t temp_allocator;

void init_paging() {
    init_temp_allocator();

    page_t temp_page         = {.bits = 0};
    temp_page.fields.address = 0xdeadbeef;

    page_table_t* new_phys_table4_ptr = allocate_frame();
    DEBUG("Physical new table4 address: %p\n", new_phys_table4_ptr);

    // Map temporarily new table4 to be able to access it
    {
        map_page_to_frame(
            table4_ptr,
            temp_page,
            PAGE_FLAG_WRITABLE,
            (uint8_t*)new_phys_table4_ptr,
            allocate_temp_frame
        );
        DEBUG(
            "Temp page (%p) physical address: %p\n",
            (size_t)temp_page.fields.address * PAGE_SIZE,
            get_physical_address(table4_ptr, (void*)((size_t)temp_page.fields.address * PAGE_SIZE))
        );

        // Zero new table4 entries and recursively map it
        zero_table_entries(new_phys_table4_ptr);
        new_phys_table4_ptr->entries[PAGE_ENTRIES - 1].fields.address
            = (size_t)new_phys_table4_ptr / PAGE_SIZE;
        new_phys_table4_ptr->entries[PAGE_ENTRIES - 1].fields.present  = true;
        new_phys_table4_ptr->entries[PAGE_ENTRIES - 1].fields.writable = true;

        unmap_page(table4_ptr, temp_page, deallocate_temp_frame, true);
    }


    // To write to the new table4 we need to have a corresponding entry in the current table4
    {
        const page_table_t* phys_table4_ptr = (page_table_t*)read_cr3();
        DEBUG("Physical table4 address: %p\n", phys_table4_ptr);

        // Map temporarily table4's address
        {
            map_page_to_frame(
                table4_ptr,
                temp_page,
                PAGE_FLAG_WRITABLE,
                (uint8_t*)phys_table4_ptr,
                allocate_temp_frame
            );
            // temp_page now holds the table4 address, so we can use it as a table4
            page_table_t* temp_page_table_ptr
                = (page_table_t*)((size_t)temp_page.fields.address * PAGE_SIZE);
            DEBUG(
                "Temp page (%p) physical address: %p\n",
                (size_t)temp_page.fields.address * PAGE_SIZE,
                get_physical_address(
                    table4_ptr, (void*)((size_t)temp_page.fields.address * PAGE_SIZE)
                )
            );

            // Temporarily set the last table4 entry with the new table4's address
            {
                DEBUG(
                    "Last table4 entry contents (before swap): %p\n",
                    table4_ptr->entries[PAGE_ENTRIES - 1].bits
                );

                // Map new table4 to table4's 511th address
                table4_ptr->entries[PAGE_ENTRIES - 1].fields.address
                    = (size_t)new_phys_table4_ptr / PAGE_SIZE;
                table4_ptr->entries[PAGE_ENTRIES - 1].fields.present  = true;
                table4_ptr->entries[PAGE_ENTRIES - 1].fields.writable = true;
                flush_tlb();
                DEBUG(
                    "Last table4 entry contents (after swap, before remap): %p\n",
                    table4_ptr->entries[PAGE_ENTRIES - 1].bits
                );

                // TODO: remap kernel onto new table4
                {}

                // Restore the original recursive mapping
                temp_page_table_ptr->entries[PAGE_ENTRIES - 1].fields.address
                    = (size_t)phys_table4_ptr / PAGE_SIZE;
                temp_page_table_ptr->entries[PAGE_ENTRIES - 1].fields.present  = true;
                temp_page_table_ptr->entries[PAGE_ENTRIES - 1].fields.writable = true;
                flush_tlb();
                DEBUG(
                    "Last table4 entry contents (after swap, after remap): %p\n",
                    table4_ptr->entries[PAGE_ENTRIES - 1].bits
                );
            }
            DEBUG(
                "Temp page (%p) physical address: %p\n",
                (size_t)temp_page.fields.address * PAGE_SIZE,
                get_physical_address(
                    table4_ptr, (void*)((size_t)temp_page.fields.address * PAGE_SIZE)
                )
            );
            unmap_page(table4_ptr, temp_page, deallocate_temp_frame, true);
        }
    }


    // TODO:
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

    size_t offset = (size_t) virtual % PAGE_SIZE;

    page_t page         = {.bits = 0};
    page.fields.address = (size_t) virtual / PAGE_SIZE;

    page_table_t* table3_ptr = next_table(table4_ptr, get_table4_index(page));
    if (table3_ptr == (void*)-1) {
        DEBUG("(get_physical_address) Page table 3 is empty, (addr = %p)\n", page.bits);
        return (void*)-1;
    }

    page_table_t* table2_ptr = next_table(table3_ptr, get_table3_index(page));
    if (table2_ptr == (void*)-1) {
        DEBUG("(get_physical_address) Page table 2 is empty, (addr = %p)\n", page.bits);
        return (void*)-1;
    }

    page_table_t* table1_ptr = next_table(table2_ptr, get_table2_index(page));
    if (table1_ptr == (void*)-1) {
        DEBUG("(get_physical_address) Page table 1 is empty, (addr = %p)\n", page.bits);
        return (void*)-1;
    }

    page_t table1_entry = table1_ptr->entries[get_table1_index(page)];
    if (!table1_entry.fields.present) {
        DEBUG("(get_physical_address) Page table 1 entry is empty, (addr = %p)\n", page.bits);
        return (void*)-1;
    }

    return (void*)((size_t)table1_entry.fields.address * PAGE_SIZE + offset);
}

void identity_map(
    page_table_t*    table4_ptr,
    uint16_t         page_flags,
    uint8_t*         frame_ptr,
    allocate_frame_t allocate_frame
) {
    page_t page         = {.bits = 0};
    page.fields.address = (size_t)frame_ptr / PAGE_SIZE;
    map_page_to_frame(table4_ptr, page, page_flags, frame_ptr, allocate_frame);
}

void map_page_to_frame(
    page_table_t*    table4_ptr,
    page_t           page,
    uint16_t         page_flags,
    uint8_t*         frame_ptr,
    allocate_frame_t allocate_frame
) {

    page_table_t* table3_ptr
        = get_or_create_next_table(table4_ptr, get_table4_index(page), allocate_frame);
    page_table_t* table2_ptr
        = get_or_create_next_table(table3_ptr, get_table3_index(page), allocate_frame);
    page_table_t* table1_ptr
        = get_or_create_next_table(table2_ptr, get_table2_index(page), allocate_frame);

    page_t* table1_entry = &(table1_ptr->entries[get_table1_index(page)]);

    if (table1_entry->bits != 0)
        PANIC(
            "Page %p is already in use (points to %p)",
            page.bits,
            (size_t)table1_entry->fields.address * PAGE_SIZE
        );
    table1_entry->bits           |= (page_flags | PAGE_FLAG_PRESENT) & PAGE_FLAG_MASK;
    table1_entry->fields.address  = (size_t)frame_ptr / PAGE_SIZE;
}

void unmap_page(
    page_table_t* table4_ptr, page_t page, deallocate_frame_t deallocate_frame, bool panic_on_empty
) {
    page_table_t* table3_ptr = next_table(table4_ptr, get_table4_index(page));
    if (table3_ptr == (void*)-1) {
        DEBUG("(unmap_page) Page table 3 is empty, (page = %p)\n", page.bits);
        if (panic_on_empty) PANIC("(unmap_page) Page table 3 is empty, (page = %p)\n", page.bits);
        return;
    }

    page_table_t* table2_ptr = next_table(table3_ptr, get_table3_index(page));
    if (table2_ptr == (void*)-1) {
        DEBUG("(unmap_page) Page table 2 is empty, (page = %p)\n", page.bits);
        if (panic_on_empty) PANIC("(unmap_page) Page table 2 is empty, (page = %p)\n", page.bits);
        return;
    }

    page_table_t* table1_ptr = next_table(table2_ptr, get_table2_index(page));
    if (table1_ptr == (void*)-1) {
        DEBUG("(unmap_page) Page table 1 is empty, (page = %p)\n", page.bits);
        if (panic_on_empty) PANIC("(unmap_page) Page table 1 is empty, (page = %p)\n", page.bits);
        return;
    }

    page_t* table1_entry = &table1_ptr->entries[get_table1_index(page)];
    if (!table1_entry->fields.present) {
        DEBUG("(unmap_page) Page table 1 entry is empty, (page = %p)\n", page.bits);
        if (panic_on_empty)
            PANIC("(unmap_page) Page table 1 entry is empty, (page = %p)\n", page.bits);
        return;
    }

    void* frame_ptr = (void*)((size_t)table1_entry->fields.address * PAGE_SIZE);

    table1_entry->bits = 0;
    flush_tlb_page(page);

    deallocate_frame(frame_ptr);
}

static inline size_t get_table4_index(page_t entry) {
    return ((size_t)entry.fields.address >> 27) & 0777;
}

static inline size_t get_table3_index(page_t entry) {
    return ((size_t)entry.fields.address >> 18) & 0777;
}

static inline size_t get_table2_index(page_t entry) {
    return ((size_t)entry.fields.address >> 9) & 0777;
}

static inline size_t get_table1_index(page_t entry) {
    return ((size_t)entry.fields.address >> 0) & 0777;
}

static inline void zero_table_entries(page_table_t* table) {
    for (int i = 0; i < PAGE_ENTRIES; i++) table->entries[i].bits = 0;
}

static inline void flush_tlb_page(page_t page) {
    DEBUG("Flusing TLB for page %p\n", page.bits);
    __asm__ volatile("invlpg (%0)" ::"r"(page.bits) : "memory");
}

static inline uint64_t read_cr3() {
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

static inline void write_cr3(uint64_t cr3) {
    __asm__ volatile("mov %0, %%cr3" ::"r"(cr3) : "memory");
}

static inline void flush_tlb() {
    DEBUG("Flushing entire TLB\n");
    write_cr3(read_cr3());
}

static inline page_table_t* next_table(page_table_t* table, size_t index) {
    page_t* entry = &(table->entries[index]);
    if (!entry->fields.present) return (void*)-1;
    if (entry->fields.huge_page)
        PANIC("Huge pages not supported! (Table entry %d is a huge page)", entry->bits);

    return (page_table_t*)(((size_t)table << 9) | (index << 12));
}

static inline page_table_t*
get_or_create_next_table(page_table_t* table, size_t index, allocate_frame_t allocate_frame) {

    // if the next table does not exist, allocate a frame for a new one
    if (next_table(table, index) == (void*)-1) {

        DEBUG("(get_or_create_next_table) Creating next table\n");
        table->entries[index].fields.address  = (size_t)allocate_frame() / PAGE_SIZE;
        table->entries[index].fields.present  = true;
        table->entries[index].fields.writable = true;
        zero_table_entries(next_table(table, index));
    }

    return next_table(table, index);
}

static inline void init_temp_allocator() {
    for (int i = 0; i < MAX_TEMP_ALLOCATOR_FRAMES; i++) temp_allocator.frames[i] = allocate_frame();
}

static inline void* allocate_temp_frame() {
    for (int i = 0; i < MAX_TEMP_ALLOCATOR_FRAMES; i++) {
        if (temp_allocator.frames[i] != (void*)-1) {
            void* frame              = temp_allocator.frames[i];
            temp_allocator.frames[i] = (void*)-1;
            return frame;
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
