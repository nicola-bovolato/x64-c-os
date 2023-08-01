#include "remap.h"
#include "../../cpu/cpu.h"
#include "../../log.h"
#include "../frameallocator/basic.h"
#include "../frameallocator/temp.h"
#include "../multiboot2.h"
#include "helpers.h"
#include "paging.h"


static inline void initialize_new_table4(page_table_t* new_table4_addr, const page_t temp_page);
static inline void remap_to_new_table4(
    const page_table_t* old_table4_addr,
    page_table_t*       new_table4_addr,
    const page_t        temp_page,
    const mem_region_t  to_map[],
    const size_t        to_map_size
);

// Remaps the kernel and other important memory areas onto a the new table4
// The pages will now have the correct flags
// Returns the new table4 physical address
const page_table_t* remap_kernel(const mem_region_t to_map[], const size_t to_map_size) {

    page_t              temp_page           = {.fields.address = 0xdeadbeef};
    const page_table_t* phys_table4_ptr     = (page_table_t*)read_cr3();
    page_table_t*       new_phys_table4_ptr = (page_table_t*)allocate_frame();
    DEBUG("Physical table4 address: %p\n", phys_table4_ptr);
    DEBUG("Physical new table4 address: %p\n", new_phys_table4_ptr);

    // Map temporarily new table4 to be able to access it
    initialize_new_table4(new_phys_table4_ptr, temp_page);

    // Avoid triggering cpu exceptions when marking a page as not executable
    enable_nxe_bit();

    // Map the mem regions onto the new table4
    remap_to_new_table4(phys_table4_ptr, new_phys_table4_ptr, temp_page, to_map, to_map_size);

    // Remove write access to not writable pages
    enable_write_protect_bit();

    // Switch to the new table4
    write_cr3((uint64_t)new_phys_table4_ptr);

    // Transform the old p4 table in a guard page (by unmapping it)
    // The kernel stack is right under the old page tables, so:
    // - a stack overflow triggers a page fault
    // - we increased the kernel stack size by:
    //    - 1   table3 (4KiB)
    //    - 1   table2 (4KiB)
    //    - 512 table1 (2MiB)
    unmap_page(
        (page_t){.fields.address = (size_t)phys_table4_ptr / PAGE_SIZE}, deallocate_frame, true
    );

    return new_phys_table4_ptr;
}

// Zeroes table entries and sets up recursive mapping
static inline void initialize_new_table4(page_table_t* new_table4_addr, const page_t temp_page) {

    // Map temporarily new table4 to be able to access it
    map_page_to_frame(
        temp_page, PAGE_FLAG_WRITABLE, (uint8_t*)new_table4_addr, allocate_temp_frame
    );
    DEBUG(
        "Temp page (%p) physical address: %p\n",
        (size_t)temp_page.fields.address * PAGE_SIZE,
        get_physical_address((void*)((size_t)temp_page.fields.address * PAGE_SIZE))
    );

    // Zero new table4 entries and recursively map it
    zero_table_entries(new_table4_addr);
    new_table4_addr->entries[PAGE_ENTRIES - 1].fields.address = (size_t)new_table4_addr / PAGE_SIZE;
    new_table4_addr->entries[PAGE_ENTRIES - 1].fields.present = true;
    new_table4_addr->entries[PAGE_ENTRIES - 1].fields.writable = true;

    unmap_page(temp_page, deallocate_temp_frame, true);
}

// Maps the mem regions to the new table
static inline void remap_to_new_table4(
    const page_table_t* old_table4_addr,
    page_table_t*       new_table4_addr,
    const page_t        temp_page,
    const mem_region_t  to_map[],
    const size_t        to_map_size
) {
    // To write to the new table4 we need to have a corresponding entry in the current table4
    // Map temporarily table4's address
    map_page_to_frame(
        temp_page, PAGE_FLAG_WRITABLE, (uint8_t*)old_table4_addr, allocate_temp_frame
    );
    // temp_page now holds the table4 address, so we can use it as a table4
    page_table_t* temp_page_table_ptr
        = (page_table_t*)((size_t)temp_page.fields.address * PAGE_SIZE);
    DEBUG(
        "Temp page (%p) physical address: %p\n",
        (size_t)temp_page.fields.address * PAGE_SIZE,
        get_physical_address((void*)((size_t)temp_page.fields.address * PAGE_SIZE))
    );

    // Temporarily set the last table4 entry with the new table4's address
    {
        DEBUG(
            "Last table4 entry contents (before swap): %p\n",
            TABLE4_PTR->entries[PAGE_ENTRIES - 1].bits
        );

        // Map new table4 to table4's 511th address
        TABLE4_PTR->entries[PAGE_ENTRIES - 1].fields.address  = (size_t)new_table4_addr / PAGE_SIZE;
        TABLE4_PTR->entries[PAGE_ENTRIES - 1].fields.present  = true;
        TABLE4_PTR->entries[PAGE_ENTRIES - 1].fields.writable = true;
        flush_tlb();
        DEBUG(
            "Last table4 entry contents (after swap, before remap): %p\n",
            TABLE4_PTR->entries[PAGE_ENTRIES - 1].bits
        );

        // Identity map the memory regions in the new table
        DEBUG("Identity mapping used regions onto new table\n");
        for (size_t i = 0; i < to_map_size; i++) {
            const mem_region_t* curr  = &to_map[i];
            uint64_t            flags = 0x0;

            if (curr->writable) flags |= PAGE_FLAG_WRITABLE;
            if (!curr->executable) flags |= PAGE_FLAG_NO_EXECUTE;

            for (uint8_t* frame = curr->start; frame < curr->end; frame += PAGE_SIZE)
                identity_map(flags, frame, allocate_frame);
        }

        // Restore the original recursive mapping
        temp_page_table_ptr->entries[PAGE_ENTRIES - 1].fields.address
            = (size_t)old_table4_addr / PAGE_SIZE;
        temp_page_table_ptr->entries[PAGE_ENTRIES - 1].fields.present  = true;
        temp_page_table_ptr->entries[PAGE_ENTRIES - 1].fields.writable = true;
        flush_tlb();
        DEBUG(
            "Last table4 entry contents (after swap, after remap): %p\n",
            TABLE4_PTR->entries[PAGE_ENTRIES - 1].bits
        );
    }

    DEBUG(
        "Temp page (%p) physical address: %p\n",
        (size_t)temp_page.fields.address * PAGE_SIZE,
        get_physical_address((void*)((size_t)temp_page.fields.address * PAGE_SIZE))
    );
    unmap_page(temp_page, deallocate_temp_frame, true);
}
