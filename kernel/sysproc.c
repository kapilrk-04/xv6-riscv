#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
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

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
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

  argint(0, &pid);
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

// uint64
// sys_sigalarm(void) {
//   int ticks;
//   uint64 addr;

//   if(argint(0, &ticks) < 0 || argaddr(1, &addr) < 0)
//     return -1;
  
//   myproc()->ticks = ticks;
//   myproc()->alarm_handler = addr;

//   return 0;
// }
// uint64 sys_sigalarm(void)
// {
//   uint64 addr;
//   int ticks;

//   if(argint(0, &ticks) < 0)
//     return -1;
//   if(argaddr(1, &addr) < 0)
//     return -1;

//   myproc()->ticks = ticks;
//   myproc()->alarm_handler = addr;

//   return 0;
// }

// uint64
// sys_sigreturn(void) {
//   struct proc *p = myproc();
//   memmove(p->trapframe, p->alarm_trapframe, PGSIZE);

//   kfree(p->alarm_trapframe);
//   p->alarm_trapframe = 0;
//   p->alarm_on = 0;
//   p->cur_ticks = 0;
//   return 0;
// }
// uint64 sys_sigreturn(void)
// {
//   struct proc *p = myproc();
//   memmove(p->trapframe, p->alarm_trapframe, PGSIZE);

//   kfree(p->alarm_trapframe);
//   p->alarm_trapframe = 0;
//   p->alarm_on = 0;
//   p->cur_ticks = 0;
//   return 0;
// }
uint64 sys_sigalarm(void){
  int ticks;
  if(argint(0, &ticks) < 0)
    return -1;
  uint64 handler;
  if(argaddr(1, &handler) < 0)
    return -1;
  myproc()->alarm_on = 0;
  myproc()->ticks = ticks;
  myproc()->cur_ticks = 0;
  myproc()->alarm_handler = handler;
  return 0; 
}

void restore()
{
  struct proc*p=myproc();

  p->alarm_trapframe->kernel_satp = p->trapframe->kernel_satp;
  p->alarm_trapframe->kernel_sp = p->trapframe->kernel_sp;
  p->alarm_trapframe->kernel_trap = p->trapframe->kernel_trap;
  p->alarm_trapframe->kernel_hartid = p->trapframe->kernel_hartid;
  *(p->trapframe) = *(p->alarm_trapframe);
}

uint64 sys_sigreturn(void){
  restore();
  myproc()->alarm_on = 0;
  //myproc()->trapframe->a0 = 0xac;
  return myproc()->trapframe->a0;
}