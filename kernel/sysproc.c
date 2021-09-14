#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "processinfo.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  
  //if(growproc(n) < 0)
  //  return -1;
  
  myproc()->sz += n;
  // Allocation happens lazily
  // De-allocation alone happens normally
  if(n < 0){
     uvmdealloc(myproc()->pagetable, addr, addr + n);
  }
  
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  
  printf("<from kernel:> sleeping for %d ticks\n", n);
  
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Part 1 Lab 2
uint64
sys_echo_simple(void)
{
  int MXLEN = 100;
  char str[MXLEN+1];
  
  int ret_val = argstr(0, str, MXLEN);
  if (ret_val == -1) return -1;

  printf("%s\n", str);

  // Returns the number of characters written excluding the terminating '\n'
  return ret_val;
}

// Part 2 Lab 2
uint64
sys_echo_kernel(void)
{
  int MXLEN = 100;
  int argc = -1;
  char str[MXLEN+1];
  uint64 array_cur_addr;
  uint64 str_addr;
  int char_written = 0;
  
  if (argint(0, &argc) < 0 || argc <= 0 || argaddr(1, &array_cur_addr) < 0) 
    return argc == 0 ? 0 : -1;
  
  for (int i = 0; i < argc; i++) {
    if (fetchaddr(array_cur_addr, &str_addr) < 0 || fetchstr(str_addr, str, MXLEN) < 0) 
      return -1;
    char *sep = i == argc-1 ? "\n" : " ";
    printf("%s%s", str, sep);
    char_written += strlen(str)+1;
    array_cur_addr += sizeof(char*);
  }

  // Returns the number of characters written excluding the final '\n'
  return char_written-1;
}

// Part 3,4 Lab 2
uint64
sys_trace(void)
{
  int trace_mask = 0, print_args = 0;
  if(argint(0, &trace_mask) < 0 || argint(1, &print_args))
    return -1;
  
  myproc()->trace_mask = trace_mask;
  myproc()->print_args = print_args;
  return 0;
}

// Part 5 Lab 2
uint64
sys_get_process_info(void)
{
  uint64 addr;
  if(argaddr(0, &addr) < 0)
    return -1;

  struct proc *p = myproc();
  struct processinfo pi;

  pi.pid = p->pid;
  pi.sz = p->sz;
  
  for(int i = 0; i < 16; i++) pi.name[i] = p->name[i];

  if(copyout(p->pagetable, addr, (char *)&pi, sizeof(pi)) < 0)
    return -1;
  
  return 0;
}
