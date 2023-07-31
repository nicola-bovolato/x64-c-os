#ifndef CPU_H
#define CPU_H

#include <stdint.h>


uint64_t read_cr0();
void     write_cr0(uint64_t value);
uint64_t read_cr3();
void     write_cr3(uint64_t value);
uint64_t read_msr(uint32_t msr_addr);
void     write_msr(uint32_t msr_addr, uint64_t msr_value);

void flush_tlb_page(void* virtual_page_addr);
void flush_tlb();
void enable_nxe_bit();
void enable_write_protect_bit();

#endif
