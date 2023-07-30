#include "paging.h"
#include "../../cpu/cpu.h"
#include "../../log.h"
#include "helpers.h"


#define LOWER_HALF_TOP_ADDRESS     (0x0000800000000000 - 1)
#define HIGHER_HALF_BOTTOM_ADDRESS 0xffff800000000000


void* get_physical_address(page_table_t* table4_ptr, void* virtual) {

    if ((size_t) virtual > LOWER_HALF_TOP_ADDRESS
        && (size_t) virtual < HIGHER_HALF_BOTTOM_ADDRESS) {
        // Invalid address
        PANIC("Virtual address not in allowed memory");
    }

    size_t offset = (size_t) virtual % PAGE_SIZE;

    page_t page = {.fields.address = (size_t) virtual / PAGE_SIZE};

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
    page_t page = {.fields.address = (size_t)frame_ptr / PAGE_SIZE};
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
    flush_tlb_page((void*)page.bits);

    deallocate_frame(frame_ptr);
}
