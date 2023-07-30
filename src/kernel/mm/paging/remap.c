#include "remap.h"
#include "../../cpu/cpu.h"
#include "../../log.h"
#include "../frameallocator/basic.h"
#include "../frameallocator/temp.h"
#include "../multiboot2.h"
#include "helpers.h"
#include "page.h"
#include "paging.h"

// Remaps the kernel and other important memory areas onto a the new table4
// The pages will now have the correct flags
void remap_kernel() {
    page_t temp_page = {.fields.address = 0xdeadbeef};

    page_table_t* new_phys_table4_ptr = (page_table_t*)allocate_frame();
    DEBUG("Physical new table4 address: %p\n", new_phys_table4_ptr);

    // Map temporarily new table4 to be able to access it
    {
        map_page_to_frame(
            TABLE4_PTR,
            temp_page,
            PAGE_FLAG_WRITABLE,
            (uint8_t*)new_phys_table4_ptr,
            allocate_temp_frame
        );
        DEBUG(
            "Temp page (%p) physical address: %p\n",
            (size_t)temp_page.fields.address * PAGE_SIZE,
            get_physical_address(TABLE4_PTR, (void*)((size_t)temp_page.fields.address * PAGE_SIZE))
        );

        // Zero new table4 entries and recursively map it
        zero_table_entries(new_phys_table4_ptr);
        new_phys_table4_ptr->entries[PAGE_ENTRIES - 1].fields.address
            = (size_t)new_phys_table4_ptr / PAGE_SIZE;
        new_phys_table4_ptr->entries[PAGE_ENTRIES - 1].fields.present  = true;
        new_phys_table4_ptr->entries[PAGE_ENTRIES - 1].fields.writable = true;

        unmap_page(TABLE4_PTR, temp_page, deallocate_temp_frame, true);
    }


    // To write to the new table4 we need to have a corresponding entry in the current table4
    {
        const page_table_t* phys_table4_ptr = (page_table_t*)read_cr3();
        DEBUG("Physical table4 address: %p\n", phys_table4_ptr);

        // Map temporarily table4's address
        {
            map_page_to_frame(
                TABLE4_PTR,
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
                    TABLE4_PTR, (void*)((size_t)temp_page.fields.address * PAGE_SIZE)
                )
            );

            // Temporarily set the last table4 entry with the new table4's address
            {
                DEBUG(
                    "Last table4 entry contents (before swap): %p\n",
                    TABLE4_PTR->entries[PAGE_ENTRIES - 1].bits
                );

                // Map new table4 to table4's 511th address
                TABLE4_PTR->entries[PAGE_ENTRIES - 1].fields.address
                    = (size_t)new_phys_table4_ptr / PAGE_SIZE;
                TABLE4_PTR->entries[PAGE_ENTRIES - 1].fields.present  = true;
                TABLE4_PTR->entries[PAGE_ENTRIES - 1].fields.writable = true;
                flush_tlb();
                DEBUG(
                    "Last table4 entry contents (after swap, before remap): %p\n",
                    TABLE4_PTR->entries[PAGE_ENTRIES - 1].bits
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
                    TABLE4_PTR->entries[PAGE_ENTRIES - 1].bits
                );
            }
            DEBUG(
                "Temp page (%p) physical address: %p\n",
                (size_t)temp_page.fields.address * PAGE_SIZE,
                get_physical_address(
                    TABLE4_PTR, (void*)((size_t)temp_page.fields.address * PAGE_SIZE)
                )
            );
            unmap_page(TABLE4_PTR, temp_page, deallocate_temp_frame, true);
        }
    }


    // TODO:
    // - Identity map the kernel (using the correct page flags for each elf section)
    // - Add a guard page below the kernel stack to prevent stack overflows
    // - Move the temporary table to the new address
    // - Use the new table as the main one
}
