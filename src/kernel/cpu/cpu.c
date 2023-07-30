#include "cpu.h"
#include "../log.h"


inline uint64_t read_cr3() {
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

inline void write_cr3(uint64_t value) { __asm__ volatile("mov %0, %%cr3" ::"r"(value) : "memory"); }

inline void flush_tlb_page(void* virtual_page_addr) {
    DEBUG("Flusing TLB for page %p\n", virtual_page_addr);
    __asm__ volatile("invlpg (%0)" ::"r"(virtual_page_addr) : "memory");
}

inline void flush_tlb() {
    DEBUG("Flushing entire TLB\n");
    write_cr3(read_cr3());
}
