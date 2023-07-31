#include "cpu.h"
#include "../log.h"


#define EFER_MSR 0xC0000080

#define NXE_BIT           0x800
#define WRITE_PROTECT_BIT 0x10000

inline uint64_t read_cr0() {
    uint64_t value;
    __asm__ volatile("mov %%cr0, %0" : "=r"(value));
    return value;
}

inline void write_cr0(uint64_t value) { __asm__ volatile("mov %0, %%cr0" ::"r"(value) : "memory"); }

inline uint64_t read_cr3() {
    uint64_t value;
    __asm__ volatile("mov %%cr3, %0" : "=r"(value));
    return value;
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

inline uint64_t read_msr(uint32_t msr_addr) {
    register uint32_t msr __asm__("ecx") = msr_addr;
    register uint32_t high __asm__("edx");
    register uint32_t low __asm__("eax");

    __asm__ volatile("rdmsr" : "=r"(high), "=r"(low) : "r"(msr) :);

    return (uint64_t)high << 32 | (uint64_t)low;
}

inline void write_msr(uint32_t msr_addr, uint64_t msr_value) {
    register uint32_t msr __asm__("ecx")  = msr_addr;
    register uint32_t high __asm__("edx") = (uint32_t)(msr_value >> 32);
    register uint32_t low __asm__("eax")  = (uint32_t)msr_value;

    __asm__ volatile("wrmsr" ::"r"(high), "r"(low), "r"(msr) :);
}

inline void enable_nxe_bit() {
    DEBUG("Enabling NXE bit in EFER register\n");
    write_msr(EFER_MSR, read_msr(EFER_MSR) | NXE_BIT);
}

inline void enable_write_protect_bit() {
    DEBUG("Enabling write protect bit in CR0 register\n");
    write_cr0(read_cr0() | WRITE_PROTECT_BIT);
}
