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
  if(n < 0)
    n = 0;
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

uint64
sys_dump(void)
{
  return (uint64) dump();
}

uint64
sys_dump2(void)
{
  int  tgt_pid, regno;
  uint64 user_dst;
  struct proc *p;
  struct proc *tgt = 0;
  struct proc *me = myproc();
  int   allow = 0;

  if (argint(0, &tgt_pid) < 0 ||
      argint(1, &regno)   < 0 ||
      argaddr(2, &user_dst) < 0)
    return -1;

  for (p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if (p->pid == tgt_pid) {
      tgt = p;            
      break;
    }
    release(&p->lock);
  }
  if (!tgt)
    return -2;

  for (p = tgt; p; p = p->parent) {
    if (p == me) {
      allow = 1;
      break;
    }
  }
  if (!allow) {
    release(&tgt->lock);
    return -1;
  }

  if (regno < 2 || regno > 11) {
    release(&tgt->lock);
    return -3;
  }

  struct trapframe *tf = tgt->trapframe;
  uint64 *s_regs = &tf->s2;
  uint64 val = (uint32)s_regs[regno - 2];

  release(&tgt->lock);

  if (copyout(me->pagetable, user_dst,
            (char*)&val, sizeof(val)) < 0)
    return -4;

  return 0;
}
