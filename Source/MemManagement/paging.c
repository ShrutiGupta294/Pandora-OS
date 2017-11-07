#include "paging.h"


void SystemDir_Mapper() ///will use it to manage OS directly
{
    //Map_non_identity(0,0,4095*1024*1024,system_dir);
    for(uint32_t i=0,j=0;i<0xFFFFF000;i+=0x1000,j+=0x1000) //Make the pages and page tables for the whole kernel memory (higher Half)
    {
        //printf(" %x",i);
        page_t* page;
        page=get_page(j,0,system_dir); //kernel Pages;
        pt_entry_set_frame ( page, i);
        pt_entry_add_attrib ( page, I86_PTE_PRESENT);
        pt_entry_add_attrib ( page, I86_PTE_WRITABLE);
        pt_entry_add_attrib ( page, I86_PTE_USER);
        pt_entry_del_attrib ( page, CUSTOM_PTE_AVAIL_1);
        pt_entry_del_attrib ( page, CUSTOM_PTE_AVAIL_2);
        //page++;
    }
}

void Setup_SystemDir()
{
	system_pdirCap = (Pdir_Capsule_t*)4194304;
	PageDirectory_t* dir=(PageDirectory_t*)(4194304);
	system_dir = dir;

	for(int i=0;i<1024;i++)
	{
	  dir->table_entry[i] = (table_t)(((i+1)*4096)+(4194304+8192));
	  table_t* entry = &dir->table_entry[i];
	  pd_entry_add_attrib (entry, I86_PDE_PRESENT);
	  pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
    pd_entry_add_attrib (entry, I86_PDE_USER);
	  pd_entry_del_attrib (entry, CUSTOM_PDE_AVAIL_1);
	  pd_entry_del_attrib (entry, CUSTOM_PDE_AVAIL_2);
	  //pd_entry_set_frame (entry, (uint32_t)dir->m_entries[i]);
	}
  SystemDir_Mapper();
}


inline void Map_non_identity(uint32_t phys, uint32_t virt, uint32_t size, PageDirectory_t* dir)
{
    for(uint32_t i=phys,j=virt;i<phys+size;i+=4096,j+=4096) 
    {
        //printf(" %x",i);
        page_t* page;
        page=get_page(j,1,dir); //kernel Pages;
        pt_entry_set_frame ( page, i);
        pt_entry_add_attrib ( page, I86_PTE_PRESENT);
        pt_entry_add_attrib ( page, I86_PTE_WRITABLE);
        pt_entry_add_attrib ( page, I86_PTE_USER);
        pt_entry_add_attrib ( page, CUSTOM_PTE_AVAIL_1);
        pt_entry_add_attrib ( page, CUSTOM_PTE_AVAIL_2);
        //page++;
    }
}

inline void Unmap_non_identity(uint32_t phys, uint32_t virt, uint32_t size, PageDirectory_t* dir)
{
    for(uint32_t i=phys,j=virt;i<phys+size;i+=4096,j+=4096) 
    {
        //printf(" %x",i);
        page_t* page;
        page=get_page(j,1,dir); //kernel Pages;
        pt_entry_set_frame ( page, 0);
        pt_entry_del_attrib ( page, I86_PTE_PRESENT);
        pt_entry_del_attrib ( page, I86_PTE_WRITABLE);
        pt_entry_del_attrib ( page, I86_PTE_USER);
        pt_entry_del_attrib ( page, CUSTOM_PTE_AVAIL_1);
        pt_entry_del_attrib ( page, CUSTOM_PTE_AVAIL_2);
        //page++;
    }
}


inline void Create_PTable(uint32_t phy, PageTable_t* tbl)
{
    for(uint32_t i=phy, k = 0;k < 1024;i+=4096, k++)
    {
        //printf(" %x",i);
        page_t* page = &tbl->page_entry[k];
        pt_entry_set_frame ( page, i);
        pt_entry_add_attrib ( page, I86_PTE_PRESENT);
        pt_entry_add_attrib ( page, I86_PTE_WRITABLE);
        pt_entry_add_attrib ( page, I86_PTE_USER);
        pt_entry_add_attrib ( page, CUSTOM_PTE_AVAIL_1);
        pt_entry_add_attrib ( page, CUSTOM_PTE_AVAIL_2);
        //page++;
    }
}

void flush_tlb_entry (uint32_t addr)
{
  asm volatile("cli");
  asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
  asm volatile("sti");
}

inline page_t* MapPage (void* phys, void* virt, PageDirectory_t* dir)
{
   //! get page directory
   PageDirectory_t* pageDirectory = dir;

   //! get page table
   table_t* e = &pageDirectory->table_entry [PAGE_DIRECTORY_INDEX ((uint32_t) virt) ];
   if (!*e)
   {
      //! page table not present, allocate it
      PageTable_t* table = (PageTable_t*) pmalloc();
      if (!table)
         return 0;

      //! clear page table
      //memset (table, 0, sizeof(PageTable_t));

      //! create a new entry
      table_t* entry = &pageDirectory->table_entry [PAGE_DIRECTORY_INDEX ( (uint32_t) virt) ];

      //! map in the table (Can also just do *entry |= 3) to enable these bits
      pd_entry_add_attrib (entry, I86_PDE_PRESENT);
      pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
      pd_entry_add_attrib (entry, I86_PDE_USER);
      pd_entry_set_frame (entry, (uint32_t)table);
   }

   //! get table
   PageTable_t* table = (PageTable_t*) PAGE_GET_PHYSICAL_ADDRESS ( e );

   //! get page
   page_t* page = &table->page_entry [ PAGE_TABLE_INDEX ( (uint32_t) virt) ];

   //! map it in (Can also do (*page |= 3 to enable..)
   pt_entry_set_frame ( page, (uint32_t) phys);
   pt_entry_add_attrib ( page, CUSTOM_PTE_AVAIL_1);
   pt_entry_add_attrib ( page, CUSTOM_PTE_AVAIL_2);
   pt_entry_add_attrib ( page, I86_PTE_PRESENT);
   pt_entry_add_attrib ( page, I86_PTE_USER);
   pt_entry_add_attrib ( page, I86_PTE_WRITABLE);
   return page;
}

void map(uint32_t phy,size_t size, PageDirectory_t* dir)
{
    //if(_cur_dir == system_dir) return;
    uint32_t j=phy;
    for (; j < phy+size;j+=0x1000)
    {
      //MapPage((void*)j,(void*)j,dir);
      page_t* page;
      page=get_page(j,1,dir);
      pt_entry_set_frame ( page, j);
      pt_entry_add_attrib ( page, I86_PTE_PRESENT);
      pt_entry_add_attrib ( page, I86_PTE_WRITABLE);
      pt_entry_add_attrib ( page, I86_PTE_USER);
      pt_entry_add_attrib ( page, CUSTOM_PTE_AVAIL_1);
      pt_entry_add_attrib ( page, CUSTOM_PTE_AVAIL_2);
    }
}

void map_readOnly(uint32_t phy,size_t size, PageDirectory_t* dir)
{
    //if(_cur_dir == system_dir) return;
    uint32_t j=phy;
    for (; j < phy+size;j+=0x1000)
    {
      //MapPage((void*)j,(void*)j,dir);
      page_t* page;
      page=get_page(j,1,dir);
      pt_entry_set_frame ( page, j);
      pt_entry_add_attrib ( page, I86_PTE_PRESENT);
     // pt_entry_add_attrib ( page, I86_PTE_WRITABLE);
      pt_entry_add_attrib ( page, I86_PTE_USER);
      pt_entry_add_attrib ( page, CUSTOM_PTE_AVAIL_1);
      pt_entry_add_attrib ( page, CUSTOM_PTE_AVAIL_2);
    }
}

void map_kernelOnly(uint32_t phy,size_t size, PageDirectory_t* dir)
{
    //if(_cur_dir == system_dir) return;
    uint32_t j=phy;
    for (; j < phy+size;j+=0x1000)
    {
      //MapPage((void*)j,(void*)j,dir);
      page_t* page;
      page=get_page(j,1,dir);
      pt_entry_set_frame ( page, j);
      pt_entry_add_attrib ( page, I86_PTE_PRESENT);
      pt_entry_add_attrib ( page, I86_PTE_WRITABLE);
     // pt_entry_add_attrib ( page, I86_PTE_USER);
      pt_entry_add_attrib ( page, CUSTOM_PTE_AVAIL_1);
      pt_entry_add_attrib ( page, CUSTOM_PTE_AVAIL_2);
    }
}

page_t* get_page(uint32_t addr,int make, PageDirectory_t* dir)
{

      // Turn the address into an index.
      addr /= 0x1000;
      // Find the page table containing this address.
      uint32_t table_idx = addr / 1024;

      if (dir->table_entry[table_idx]) // If this table is already assigned
      {
          //printf("1 ");
          table_t* entry = &dir->table_entry[table_idx];
          PageTable_t* table=(PageTable_t*)PAGE_GET_PHYSICAL_ADDRESS(entry);
          return &table->page_entry[addr%1024];
      }
      else if(make)
      {
          dir->table_entry[table_idx] = (table_t)phy_alloc4K();

        //  map(dir->table_entry[table_idx], 4096, dir);

          memset_fast((void*)dir->table_entry[table_idx], 0, 0x1000);
          table_t* entry = &dir->table_entry [table_idx];
          PageTable_t* table=(PageTable_t*)PAGE_GET_PHYSICAL_ADDRESS(entry);
          pd_entry_add_attrib (entry, I86_PDE_PRESENT);
          pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
          pd_entry_add_attrib (entry, I86_PDE_USER);
          //pd_entry_set_frame (entry, (uint32_t)dir->m_entries[table_idx]);
        //  dir->tables[table_idx] = tmp | 0x7; // PRESENT, RW, US.
          return &table->page_entry[addr%1024];
      }
      else
      {
          printf(" Cant get the Page! ");
          return 0;
      }
}


void switch_directory(PageDirectory_t *dir)
{
  //Get_Scheduler()->current_pdir = dir;
	asm volatile("mov %0, %%cr3":: "r"((uint32_t)dir):"memory");
}

inline page_t* get_pageEntry(uint32_t addr)
{
	uint32_t frame = addr/4096;
	uint32_t pd_off = frame/1024;
	return &((PageTable_t*)(PAGE_DIRECTORY_INDEX(_cur_dir->table_entry[pd_off])))->page_entry[frame%1024];
}

inline table_t* get_tableEntry(uint32_t addr)
{
	uint32_t frame = addr/4096;
	return &_cur_dir->table_entry[frame/1024];
}

inline PageTable_t* get_table(uint32_t addr)
{
	uint32_t frame = addr/4096;
	uint32_t pd_off = frame/1024;
	return (PageTable_t*)(PAGE_DIRECTORY_INDEX(_cur_dir->table_entry[pd_off]));
}

uint32_t __attribute__((optimize("O0"))) get_phyAddr(uint32_t addr, PageDirectory_t* dir)
{
  uint32_t frame = *(get_page(addr, 0, dir))/4096;
  frame *= 4096;
  frame += addr%4096;
  return frame;
}


void Setup_Paging()
{
	Setup_SystemDir();


  for(uint32_t i = 0; i < total_CPU_Cores - 1; i++)
  {
    *(uint32_t*)(0x3000 + (i*0x2000) + AP_startup_Code_sz + 8) = 0x4284;            // Tell other CPUs to switch to code which enables paging, see AP_Bootup.asm
  }

	switch_directory(system_dir);
  asm volatile("mov %cr0, %eax;\
                or $0x80000000, %eax;\
                mov %eax, %cr0;");

  printf("\nPaging Successfully Enabled on all CPU Cores!");
}
