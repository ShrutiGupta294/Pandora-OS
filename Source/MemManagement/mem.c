#include "mem.h"
#include "library.h"

uint32_t* frame_stack_end = (16*1024*1024);

// Frames allocated after 512th MB to 2GB

void mem_init()
{
    for(int i = 524288; i > 131072 ; i-=4096)
    {
        push_frameStack(i);
    }
}

uint32_t pop_frameStack()
{
	uint32_t fr;
	back:
	fr = *frame_stack_end;
	--frame_stack_end;
	//printf("--A--");
	if(!fr)
	{
		printf("\nNo memory left! %x",fr);
		//while(1);
		asm volatile("hlt");
		goto back;
	}
	return fr;
}

void push_frameStack(uint32_t fr)
{
	++frame_stack_end;
	*frame_stack_end = fr;
}

uint32_t phy_alloc4K()
{
    return pop_frameStack()*4096;
}

uint32_t base_addr = (40*1024*1024);

uint32_t kmalloc(size_t size)
{
    uint32_t a = base_addr;
	base_addr += size;
	//printf("<%d>", a);
    return a;
}

uint32_t pmem_4k(int size)
{
	uint32_t tmp = ((uint32_t)kmalloc((size+1)*4096));//(uint32_t)ROUNDUP(, 4096);
	//printf("\n[%d]", tmp);
	return ROUNDUP(tmp, 4096);
}

void kfree(void* addr)
{
    return;
}
