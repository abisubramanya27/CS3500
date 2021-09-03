#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz)
    return -1;
  if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  int err = copyinstr(p->pagetable, buf, addr, max);
  if(err < 0)
    return err;
  return strlen(buf);
}

static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  *ip = argraw(n);
  return 0;
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
int
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
  return 0;
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  if(argaddr(n, &addr) < 0)
    return -1;
  return fetchstr(addr, buf, max);
}

extern uint64 sys_chdir(void);
extern uint64 sys_close(void);
extern uint64 sys_dup(void);
extern uint64 sys_exec(void);
extern uint64 sys_exit(void);
extern uint64 sys_fork(void);
extern uint64 sys_fstat(void);
extern uint64 sys_getpid(void);
extern uint64 sys_kill(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_mknod(void);
extern uint64 sys_open(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_unlink(void);
extern uint64 sys_wait(void);
extern uint64 sys_write(void);
extern uint64 sys_uptime(void);
extern uint64 sys_echo_simple(void);
extern uint64 sys_echo_kernel(void);
extern uint64 sys_trace(void);
extern uint64 sys_get_process_info(void);

static uint64 (*syscalls[])(void) = {
[SYS_fork]              sys_fork,
[SYS_exit]              sys_exit,
[SYS_wait]              sys_wait,
[SYS_pipe]              sys_pipe,
[SYS_read]              sys_read,
[SYS_kill]              sys_kill,
[SYS_exec]              sys_exec,
[SYS_fstat]             sys_fstat,
[SYS_chdir]             sys_chdir,
[SYS_dup]               sys_dup,
[SYS_getpid]            sys_getpid,
[SYS_sbrk]              sys_sbrk,
[SYS_sleep]             sys_sleep,
[SYS_uptime]            sys_uptime,
[SYS_open]              sys_open,
[SYS_write]             sys_write,
[SYS_mknod]             sys_mknod,
[SYS_unlink]            sys_unlink,
[SYS_link]              sys_link,
[SYS_mkdir]             sys_mkdir,
[SYS_close]             sys_close,
[SYS_echo_simple]       sys_echo_simple,
[SYS_echo_kernel]       sys_echo_kernel,
[SYS_trace]             sys_trace,
[SYS_get_process_info]  sys_get_process_info
};

static char* syscall_names[] = {
[SYS_fork]              "fork",
[SYS_exit]              "exit",
[SYS_wait]              "wait",
[SYS_pipe]              "pipe",
[SYS_read]              "read",
[SYS_kill]              "kill",
[SYS_exec]              "exec",    
[SYS_fstat]             "fstat",
[SYS_chdir]             "chdir",
[SYS_dup]               "dup",
[SYS_getpid]            "getpid",
[SYS_sbrk]              "sbrk",
[SYS_sleep]             "sleep",
[SYS_uptime]            "uptime",
[SYS_open]              "open",
[SYS_write]             "write",
[SYS_mknod]             "mknod",
[SYS_unlink]            "unlink",
[SYS_link]              "link",
[SYS_mkdir]             "mkdir",
[SYS_close]             "close",
[SYS_echo_simple]       "echo_simple",
[SYS_echo_kernel]       "echo_kernel",
[SYS_trace]             "trace",
[SYS_get_process_info]  "get_process_info"
};

// Function that takes in the base address of a array of char*/string and
// the maximum number of strings in the array, and
// prints the strings in the array on a single line separated by space.
void
print_joined_str_array(uint64 base_addr, int MAX_N_STR) {
    int MXLEN = 100;
    uint64 str_addr;
    char str[MXLEN+1];

    for(int i = 0; i < MAX_N_STR; i++) {
        if(fetchaddr(base_addr, &str_addr) < 0 || str_addr == 0 || fetchstr(str_addr, str, MXLEN) < 0) break;
        printf("%s ", str);
        base_addr += sizeof(char*);
    }
    printf("\n");
}

// Function that takes in the syscall ID(num) and the pointer to current process's proc structure,
// and print the arguments to the syscall.
int
print_syscall_args(int num, struct proc* p) {
    int MXLEN = 100;
    char str1[MXLEN+1], str2[MXLEN+1];
    uint64 p1;
    int n1, n2;

    switch(num) {
        case SYS_exit:
            if(argint(0, &n1) < 0) return -1;
            printf("arg 1 -> %d\n", n1);
            break;
        case SYS_fork:
            printf("<none>\n");
            break;
        case SYS_getpid:
            printf("<none>\n");
            break;
        case SYS_wait:
            if(argaddr(0, &p1) < 0) return -1;
            printf("arg 1 -> %p\n", p1);
            break;
        case SYS_sbrk:
            if(argint(0, &n1) < 0) return -1;
            printf("arg 1 -> %d\n", n1);
            break;
        case SYS_sleep:
            if(argint(0, &n1) < 0) return -1;
            printf("arg 1 -> %d\n", n1);
            break;
        case SYS_kill:
            if(argint(0, &n1) < 0) return -1;
            printf("arg 1 -> %d\n", n1);
            break;
        case SYS_uptime:
            printf("<none>\n");
            break;
        case SYS_echo_simple:
            if(argstr(0, str1, MXLEN) < 0) return -1;
            printf("arg 1 -> %s\n", str1);
            break;
        case SYS_echo_kernel:
            if(argint(0, &n1) < 0) return -1;
            printf("arg 1 -> %d\n", n1);
            if(n1 == 0) printf("2 -> \n");
            else {
                if(argaddr(1, &p1) < 0) return -1;
                printf("arg 2 -> ");
                print_joined_str_array(p1, n1);
            }
            break;
        case SYS_exec:
            if(argstr(0, str1, MXLEN) < 0 || argaddr(1, &p1) < 0) return -1;
            printf("arg 1 -> %s\n", str1);
            printf("arg 2 -> ");
            print_joined_str_array(p1, MAXARG);
            break;
        case SYS_pipe:
            if(argaddr(0, &p1) < 0) return -1;
            printf("arg 1 -> %p\n", p1);
        case SYS_write:
            if(argint(0, &n1) < 0 || argaddr(1, &p1) < 0 || argint(2, &n2) < 0) return -1;
            printf("arg 1 -> %d\n", n1);
            printf("arg 2 -> %p\n", p1);
            printf("arg 3 -> %d\n", n2);
            break;
        case SYS_read:
            if(argint(0, &n1) < 0 || argaddr(1, &p1) < 0 || argint(2, &n2) < 0) return -1;
            printf("arg 1 -> %d\n", n1);
            printf("arg 2 -> %p\n", p1);
            printf("arg 3 -> %d\n", n2);
            break;
        case SYS_close:
            if(argint(0, &n1) < 0) return -1;
            printf("arg 1 -> %d\n", n1);
            break;
        case SYS_open:
            if(argstr(0, str1, MXLEN) < 0 || argint(1, &n1) < 0) return -1;
            printf("arg 1 -> %s\n", str1);
            printf("arg 2 -> %d\n", n1);
            break;
        case SYS_mknod:
            if(argstr(0, str1, MXLEN) < 0 || argint(1, &n1) < 0 || argint(2, &n2) < 0) return -1;
            printf("arg 1 -> %s\n", str1);
            printf("arg 2 -> %d\n", n1);
            printf("arg 3 -> %d\n", n2);
            break;
        case SYS_unlink:
            if(argstr(0, str1, MXLEN) < 0) return -1;
            printf("arg 1 -> %s\n", str1);
            break;
        case SYS_fstat:
            if(argint(0, &n1) < 0 || argaddr(1, &p1) < 0) return -1;
            printf("arg 1 -> %d\n", n1);
            printf("arg 2 -> %p\n", p1);
            break;
        case SYS_link:
            if(argstr(0, str1, MXLEN) < 0 || argstr(1, str2, MXLEN)) return -1;
            printf("arg 1 -> %s\n", str1);
            printf("arg 2 -> %s\n", str2);
            break;
        case SYS_mkdir:
            if(argstr(0, str1, MXLEN) < 0) return -1;
            printf("arg 1 -> %s\n", str1);
            break;
        case SYS_chdir:
            if(argstr(0, str1, MXLEN) < 0) return -1;
            printf("arg 1 -> %s\n", str1);
            break;
        case SYS_dup:
            if(argint(0, &n1) < 0) return -1;
            printf("arg 1 -> %d\n", n1);
            break;
        case SYS_get_process_info:
            if(argaddr(0, &p1) < 0) return -1;
            printf("arg 1 -> %p\n", p1);
            break;
        default:
            printf("Error! Invalid syscall number\n");
            return -1;
    }

    return 0;
}

void
syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    // added for part 4 of lab 2
    if ((p->trace_mask & (1<<num)) && p->print_args) {
      printf("%d: syscall %s arguments\n", p->pid, syscall_names[num]);
      if (print_syscall_args(num, p) < 0) printf("Error while retrieving arguments!\n");
    }
    p->trapframe->a0 = syscalls[num]();
    // added for part 3 of lab 2
    if ((p->trace_mask & (1<<num))) {
      printf("%d: syscall %s returned -> %d\n", p->pid, syscall_names[num], p->trapframe->a0);
    }
    // added for part 4 of lab 2
    if ((p->trace_mask & (1<<num)) && p->print_args) printf("-----------------------------------\n");
  } else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
