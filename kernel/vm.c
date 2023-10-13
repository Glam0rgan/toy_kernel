#include "types.h"
#include "defs.h"
#include "mmu.h"
#include "memlayout.h"

// Return the address of the PTE in pml4
// that corresponds to virtual address va. If alloc!=0,
// create any required page table pages.
static u64* page_walk(u64* root,const void* va,int alloc){
  
  u64 *pml4e, *pdpte, *pde, *pte;

  // Page map level 4 index
  pml4e = &root[PML4(va)];

  if(!(*pml4e & PTE_P)){
    if(!alloc || (pml4e = (u64*)kalloc()) == 0)
      return 0;
    memset(pml4e, 0, PGSIZE);
    *pml4e[PML4X(va)] = v2p(pml4e) | PTE_P | PTE_W | PTE_U;  
  }

  // Page directory pointer index
  pdpte = &pdpte[PDPTX(va)];

  if(!(*pdpte & PTE_P)){
    if(!alloc || (pdpte = (u64*)kalloc()) == 0)
      return 0;
    memset(pdpte, 0, PGSIZE);
    *pdpte[PDPTX(va)] = v2p(pdpte) | PTE_P | PTE_W | PTE_U;  
  }

  // Page directory index
  pde = &pde[PDX(va)];

  if(!(*pde & PTE_P)){
    if(!alloc || (pde = (u64*)kalloc()) == 0)
      return 0;
    memset(pde, 0, PGSIZE);
    *pde[PDX(va)] = v2p(pde) | PTE_P | PTE_W | PTE_U;  
  }

  // Page table index
  pte = &pte[PTX(va)];

  if(!(*pte & PTE_P)){
    if(!alloc || (pte = (u64*)kalloc()) == 0)
      return 0;
    memset(pte, 0, PGSIZE);
    *pte[PTX(va)] = v2p(pte) | PTE_P | PTE_W | PTE_U;  
  }

  return pte;
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
static int mamppages(u64* pml4, void* va, u64 size, u64 pa, int perm){
  char *first, *last;
  u64* pte;

  first = (char*)ALIGN_DOWN(va, PGSIZE);
  last = (char*)ALIGN_DOWN(va, PGSIZE);
  while(true){
    if((pte = page_walk(pml4, first, 1) == 0)
      return -1;
    if(*pte & PTE_P)
      panic("remap");
    *pte = pa | perm | PTE_P;
    if(first == last)
      break;
    first += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}