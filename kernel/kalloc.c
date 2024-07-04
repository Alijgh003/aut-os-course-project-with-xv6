// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#include "kalloc.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

int total_memory = 0;

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

//PHASE3: how many processes are claiming of each page_table
struct {
  struct spinlock lock;
  int pages_refs_num[PHYSTOP >> PGSHIFT];
} kref;

int
get_kref_index(uint64 pa){
  int r = pa >> PGSHIFT;
  return r;
}

// PHASE3: utility functions to modify kref
void
increase_kref(void *pa){//is called in uvmcopy function
  acquire(&kref.lock);
  int index = get_kref_index((uint64)pa);
  kref.pages_refs_num[index]++;
  release(&kref.lock);
}
void
decrease_kref(void *pa){//is called in kfree function
  acquire(&kref.lock);
  uint64 index = get_kref_index((uint64) pa);
  if(kref.pages_refs_num[index] > 0){
    kref.pages_refs_num[index]--;
  }else{
    kref.pages_refs_num[index] = 0;
  }
  release(&kref.lock);
}
void
set_kref(void *pa,int n){//is called in kalloc function
  acquire(&kref.lock);
  int index = get_kref_index((uint64)pa);
  kref.pages_refs_num[index] = n;
  release(&kref.lock);
}
int
get_num_of_pages(void *pa){
  int index = get_kref_index((uint64)pa);
  return kref.pages_refs_num[index];
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&kref.lock, "kref");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    total_memory += PGSIZE;
    set_kref((void*) p,0);
    kfree(p);
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa) 
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  //PHASE3: decrease num of references to the current page
  decrease_kref(pa);
  if(get_num_of_pages(pa) == 0){
      // Fill with junk to catch dangling refs.
      memset(pa, 1, PGSIZE);

      r = (struct run*)pa;

      acquire(&kmem.lock);
      r->next = kmem.freelist;
      kmem.freelist = r;
      release(&kmem.lock);
  }
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    //PHASE3: set created page references to 1;
    set_kref((void*)r,1);
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  
  return (void*)r;
}

uint64
get_free_mem()
{
  struct run *r;
  int n = 0;
  r= kmem.freelist;
  while(r){
    n++;
    r= r->next;
  }
  return n * PGSIZE;
}
