// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
  void *addr = (void *) utf->utf_fault_va;
  uint32_t err = utf->utf_err;
  int r;

  // Check that the faulting access was (1) a write, and (2) to a
  // copy-on-write page.  If not, panic.
  // Hint:
  //   Use the read-only page table mappings at uvpt
  //   (see <inc/memlayout.h>).

  // LAB 4: Your code here.
  if (!(err & FEC_WR)) {
    panic("PageFault was not caused by a write");
  }
  if (!(err & FEC_PR)) {
    panic("Faulting address is not mapped");
  } 

  pde_t pde = uvpd[PDX(addr)];
  pte_t pte = uvpt[PGNUM(addr)];
  if (!(pte & PTE_COW)) {
    panic("Tried to write to a read-only page");
  }
  // Allocate a new page, map it at a temporary location (PFTEMP),
  // copy the data from the old page to the new page, then move the new
  // page to the old page's address.
  // Hint:
  //   You should make three system calls.

  // LAB 4: Your code here.
  if ((r = sys_page_alloc(0, PFTEMP, PTE_P|PTE_U|PTE_W)) < 0)
    panic("sys_page_alloc: %e", r);
  memmove(PFTEMP, addr, PGSIZE);
  if ((r = sys_page_map(0, PFTEMP, 0, addr, PTE_P|PTE_U|PTE_W)) < 0)
    panic("sys_page_map: %e", r);
  if ((r = sys_page_unmap(0, PFTEMP)) < 0)
    panic("sys_page_unmap: %e", r);
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, void *va, int perm)
{
  int r;
  int updatedPerms;
  if (!(perm & PTE_W) && !(perm & PTE_COW)) {//Page is read-only
    updatedPerms = perm;
  } else {//Page is writable or copy-on-write
    updatedPerms = (perm & (~PTE_W)) | PTE_COW;
  }

  if ((r = sys_page_map(0, va, envid, va, updatedPerms)) < 0) {
    panic("sys_page_map: %e", r);
  }
  if (updatedPerms & PTE_COW) { 
    //Change parent's page perms to have PTE_COW 
    if ((r = sys_page_map(0, va, 0, va, updatedPerms)) < 0) {
      panic("sys_page_map: %e", r);
    }
  }
  return 0;
}

static void
dup_or_share(envid_t dstenv, void *va, int perm)
{
	int r;
	if (!(perm & PTE_P) || !(perm & PTE_U)) {
		return;
	}
	// Page is read-only
	if (!(perm & PTE_W)) {
		if ((r = sys_page_map(0, va, dstenv, UTEMP, perm)) < 0)
			panic("sys_page_map: %e", r);
	} else {
		if ((r = duppage(dstenv, va, perm)) < 0)
			panic("sys_page_map: %e", r);
	}
}

envid_t
fork_v0(void)
{
	envid_t envid;
	uint8_t *addr;
	int r;

	// Allocate a new child environment.
	// The kernel will initialize it with a copy of our register state,
	// so that the child will appear to have called sys_exofork() too -
	// except that in the child, this "fake" call to sys_exofork()
	// will return 0 instead of the envid of the child.
	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	if (envid == 0) {
		// We're the child.
		// The copied value of the global variable 'thisenv'
		// is no longer valid (it refers to the parent!).
		// Fix it and return 0.
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
	// We're the parent.
	// Eagerly copy our entire address space into the child.
	// This is NOT what you should do in your fork implementation.
	for (addr = 0; (unsigned int) addr < UTOP; addr += PGSIZE) {
		// if mapped
		// obtain permissions
		pde_t pde = uvpd[PDX(addr)];
		if (pde & PTE_P) {
			pte_t pte = uvpt[PGNUM(addr)];
			if (pte & PTE_P) {
				dup_or_share(envid, addr, (int) pte & PTE_SYSCALL);
			}
		}
	}

	// Start the child environment running
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);

	return envid;
}


//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	// LAB 4: Your code here.
  envid_t envid;
  uint8_t *addr;
  int r;
  extern void (*_pgfault_handler)(struct UTrapframe *utf);

  //Set pgfault handler for the parent
  set_pgfault_handler(pgfault);

  // Allocate a new child environment.
  // The kernel will initialize it with a copy of our register state,
  // so that the child will appear to have called sys_exofork() too -
  // except that in the child, this "fake" call to sys_exofork()
  // will return 0 instead of the envid of the child.
  envid = sys_exofork();
  if (envid < 0)
    panic("sys_exofork: %e", envid);
  if (envid == 0) {
    // We're the child.
    // The copied value of the global variable 'thisenv'
    // is no longer valid (it refers to the parent!).
    // Fix it and return 0.
    thisenv = &envs[ENVX(sys_getenvid())];
    //Global variable _pgfault_handler is set thanks to the parent. Reset it
    //and then set pgfault handler for the child
    _pgfault_handler = 0;
    set_pgfault_handler(pgfault);
    return 0;
  }
  // We're the parent.
  for (addr = 0; (unsigned int) addr < UTOP; addr += PGSIZE) {
    if ((unsigned int) addr == (UXSTACKTOP - PGSIZE))
      continue; //do not map exception stack page 

    pde_t pde = uvpd[PDX(addr)];
    if (pde & PTE_P) {
      pte_t pte = uvpt[PGNUM(addr)];
      if (pte & PTE_P) {
        duppage(envid, addr, (int) pte & PTE_SYSCALL);
      }
    }
  }

  // Start the child environment running
  if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
    panic("sys_env_set_status: %e", r);

  return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}