#ifndef MEM_H
#define MEM_H 

#include "library.h"

void mem_init();
uint32_t pop_frameStack();
void push_frameStack(uint32_t fr);
uint32_t phy_alloc4K();
uint32_t kmalloc(size_t size);
uint32_t pmem_4k(int size);
void kfree(void* addr);

#endif 