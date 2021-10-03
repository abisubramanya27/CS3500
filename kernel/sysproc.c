#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

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
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  // For lab 4 section 2
  // backtrace();

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

// For Lab 4 section 3
uint64
sys_pcbread(void)
{
  // procstate int to string mapping
  const char* procstate_str[] = {
    [UNUSED]   "UNUSED",
    [USED]     "USED",
    [SLEEPING] "SLEEPING",
    [RUNNABLE] "RUNNABLE",
    [RUNNING]  "RUNNING",
    [ZOMBIE]   "ZOMBIE"
  };
  
  struct proc* p = myproc();
  printf("Process PID: %d\n", p->pid);
  printf("Process name: %s\n", p->name);
  printf("Process state: %s (%d)\n", procstate_str[p->state], p->state);
  printf("Process killed: %s\n", p->killed ? "yes" : "no");
  if (p->killed) printf("Process exit status: %d\n", p->xstate);
  if (p->parent == 0) printf("Process parent: --\n");
  else printf("Process parent PID: %d | name: %s\n", p->parent->pid, p->parent->name);
  printf("Process kstack address: %p\n", p->kstack);
  printf("Process memory size: %d Bytes\n", p->sz);
  printf("Process pagetable address: %p\n", p->pagetable);
  printf("Process trapframe address: %p\n", p->trapframe);

  return 0;
}
