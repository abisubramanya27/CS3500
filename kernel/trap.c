#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

struct spinlock tickslock;
uint ticks;

extern char trampoline[], uservec[], userret[];

// in kernelvec.S, calls kerneltrap().
void kernelvec();

extern int devintr();

void
trapinit(void)
{
  initlock(&tickslock, "time");
}

// set up to take exceptions and traps while in the kernel.
void
trapinithart(void)
{
  w_stvec((uint64)kernelvec);
}

int
strcmp(char *a, char *b) 
{
  while(*a && *b && *a == *b) a++, b++;
  return *a < *b ? -1 : *a > *b ? 1 : 0;
}

void
print_trapframe(struct trapframe* tf) 
{
  printf("-- kernel_satp: %p\n", tf->kernel_satp);
  printf("-- kernel_sp: %p\n", tf->kernel_sp);
  printf("-- kernel_trap: %p\n", tf->kernel_trap);
  printf("-- epc: %p\n", tf->epc);
  printf("-- kernel_hartid: %d\n", tf->kernel_hartid);
  printf("-- ra: %p\n", tf->ra);
  printf("-- sp: %p\n", tf->sp);
  printf("-- gp: %p\n", tf->gp);
  printf("-- tp: %p\n", tf->tp);
  printf("-- t0: %p\n", tf->t0);
  printf("-- t1: %p\n", tf->t1);
  printf("-- t2: %p\n", tf->t2);
  printf("-- s0: %p\n", tf->s0);
  printf("-- a0: %p\n", tf->a0);
  printf("-- a1: %p\n", tf->a1);
  printf("-- a2: %p\n", tf->a2);
  printf("-- a3: %p\n", tf->a3);
  printf("-- a4: %p\n", tf->a4);
  printf("-- a5: %p\n", tf->a5);
  printf("-- a6: %p\n", tf->a6);
  printf("-- a7: %p\n", tf->a7);
  printf("-- s2: %p\n", tf->s2);
  printf("-- s3: %p\n", tf->s3);
  printf("-- s4: %p\n", tf->s4);
  printf("-- s5: %p\n", tf->s5);
  printf("-- s6: %p\n", tf->s6);
  printf("-- s7: %p\n", tf->s7);
  printf("-- s8: %p\n", tf->s8);
  printf("-- s9: %p\n", tf->s9);
  printf("-- s10: %p\n", tf->s10);
  printf("-- s11: %p\n", tf->s11);
  printf("-- t3: %p\n", tf->t3);
  printf("-- t4: %p\n", tf->t4);
  printf("-- t5: %p\n", tf->t5);
  printf("-- t6: %p\n", tf->t6);
}

//
// handle an interrupt, exception, or system call from user space.
// called from trampoline.S
//
void
usertrap(void)
{
  int which_dev = 0;

  if((r_sstatus() & SSTATUS_SPP) != 0)
    panic("usertrap: not from user mode");

  // send interrupts and exceptions to kerneltrap(),
  // since we're now in the kernel.
  w_stvec((uint64)kernelvec);

  struct proc *p = myproc();
  
  // save user program counter.
  p->trapframe->epc = r_sepc();
  
  if(r_scause() == 8){
    // system call

    if(p->killed)
      exit(-1);

    // sepc points to the ecall instruction,
    // but we want to return to the next instruction.
    p->trapframe->epc += 4;

    // an interrupt will change sstatus &c registers,
    // so don't enable until done with those registers.
    intr_on();

    syscall();

  } else if((which_dev = devintr()) != 0){
    // ok
  } else {
    printf("usertrap(): unexpected scause %p pid=%d\n", r_scause(), p->pid);
    printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
    p->killed = 1;
  }

  if(p->killed)
    exit(-1);

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2)
    yield();
  
  usertrapret();
}

//
// return to user space
//
void
usertrapret(void)
{
  struct proc *p = myproc();
  
  // we're about to switch the destination of traps from
  // kerneltrap() to usertrap(), so turn off interrupts until
  // we're back in user space, where usertrap() is correct.
  intr_off();

  // send syscalls, interrupts, and exceptions to trampoline.S
  w_stvec(TRAMPOLINE + (uservec - trampoline));

  // set up trapframe values that uservec will need when
  // the process next re-enters the kernel.
  p->trapframe->kernel_satp = r_satp();         // kernel page table
  p->trapframe->kernel_sp = p->kstack + PGSIZE; // process's kernel stack
  p->trapframe->kernel_trap = (uint64)usertrap;
  p->trapframe->kernel_hartid = r_tp();         // hartid for cpuid()

  // set up the registers that trampoline.S's sret will use
  // to get to user space.
  
  // set S Previous Privilege mode to User.
  unsigned long x = r_sstatus();
  x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
  x |= SSTATUS_SPIE; // enable interrupts in user mode
  w_sstatus(x);

  // set S Exception Program Counter to the saved user pc.
  w_sepc(p->trapframe->epc);

  // tell trampoline.S the user page table to switch to.
  uint64 satp = MAKE_SATP(p->pagetable);

  // jump to trampoline.S at the top of memory, which 
  // switches to the user page table, restores user registers,
  // and switches to user mode with sret.
  uint64 fn = TRAMPOLINE + (userret - trampoline);
  
  // For Lab 4 section 3 - printing child and parent trapframe for fork system calls from attack
  acquire(&p->lock);
  if (p->fork_call && !strcmp(p->name, "attack")) {
    
    if (p->trapframe->a0 == 0) {
      printf("---------- CHILD PROCESS TRAPFRAME ----------\n");
      print_trapframe(p->trapframe);
      printf("----------- END OF CHILD TRAPFRAME ----------\n\n");
    }
    if (p->trapframe->a0 > 0) {
      printf("---------- PARENT PROCESS TRAPFRAME ---------\n");
      print_trapframe(p->trapframe);
      printf("---------- END OF PARENT TRAPFRAME ----------\n\n");  
    }
  }
  // Ensuring before return fork_call flag is turned off
  // acquire(&p->lock);
  p->fork_call = 0;
  release(&p->lock);
  
 ((void (*)(uint64,uint64))fn)(TRAPFRAME, satp);
}

// interrupts and exceptions from kernel code go here via kernelvec,
// on whatever the current kernel stack is.
void 
kerneltrap()
{
  int which_dev = 0;
  uint64 sepc = r_sepc();
  uint64 sstatus = r_sstatus();
  uint64 scause = r_scause();
  
  if((sstatus & SSTATUS_SPP) == 0)
    panic("kerneltrap: not from supervisor mode");
  if(intr_get() != 0)
    panic("kerneltrap: interrupts enabled");

  if((which_dev = devintr()) == 0){
    printf("scause %p\n", scause);
    printf("sepc=%p stval=%p\n", r_sepc(), r_stval());
    panic("kerneltrap");
  }

  // give up the CPU if this is a timer interrupt.
  if(which_dev == 2 && myproc() != 0 && myproc()->state == RUNNING)
    yield();

  // the yield() may have caused some traps to occur,
  // so restore trap registers for use by kernelvec.S's sepc instruction.
  w_sepc(sepc);
  w_sstatus(sstatus);
}

void
clockintr()
{
  acquire(&tickslock);
  ticks++;
  wakeup(&ticks);
  release(&tickslock);
}

// check if it's an external interrupt or software interrupt,
// and handle it.
// returns 2 if timer interrupt,
// 1 if other device,
// 0 if not recognized.
int
devintr()
{
  uint64 scause = r_scause();

  if((scause & 0x8000000000000000L) &&
     (scause & 0xff) == 9){
    // this is a supervisor external interrupt, via PLIC.

    // irq indicates which device interrupted.
    int irq = plic_claim();

    if(irq == UART0_IRQ){
      uartintr();
    } else if(irq == VIRTIO0_IRQ){
      virtio_disk_intr();
    } else if(irq){
      printf("unexpected interrupt irq=%d\n", irq);
    }

    // the PLIC allows each device to raise at most one
    // interrupt at a time; tell the PLIC the device is
    // now allowed to interrupt again.
    if(irq)
      plic_complete(irq);

    return 1;
  } else if(scause == 0x8000000000000001L){
    // software interrupt from a machine-mode timer interrupt,
    // forwarded by timervec in kernelvec.S.

    if(cpuid() == 0){
      clockintr();
    }
    
    // acknowledge the software interrupt by clearing
    // the SSIP bit in sip.
    w_sip(r_sip() & ~2);

    return 2;
  } else {
    return 0;
  }
}

