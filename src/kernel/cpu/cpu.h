#ifndef CPU_H
#define CPU_H

#include <stdint.h>


uint64_t read_cr3();
void     write_cr3(uint64_t value);
void     flush_tlb_page(void* virtual_page_addr);
void     flush_tlb();

#endif
