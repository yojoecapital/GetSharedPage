#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "syscall.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
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

int
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
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//shared pages sys calls

//takes a key and a count of requested pages
//returns the address of the first shared page under that key
//NOTE key can't be 0 (0 is used as a sentinal value in another function)
//NOTE count must be more than or equal to 1
int
sys_GetSharedPage(void)
{
   key_int key;
   int count;
   
    if (argint(0, &key) < 0){
       return -1;
    }
  
   if (argint(1, &count) < 0){
      return -1;
   }
   
   if(key == 0 || count < 1) //key can't be 0 and count must be >1
   {
      cprintf("ERROR: key or count are invalid values\n");
      return -1;
   }
      
   int pos;
   if((pos = where_does_fit(count)) < 0) //where_does_fit returns -1 if there is not spot
   {
      cprintf("ERROR: process address space is full\n");
      return -1;
   }
   
   int c;
   int ret = -1;
   for(c = 0; c < count; c++, pos++)
      if((ret = find_shared_page(key, c, pos)) < 0)
         break;
   return ret; 
}

//returns -1 if failed and returns count of pages freed if successful
int
sys_FreeSharedPage(void)
{
   key_int key;
   
   if (argint(0, &key) < 0){
       return -1;
    }
   
   int count = get_num_sh_procs(key);
   
   int c;
   for(c = count-1; c >= 0; c--)
      if(remove_shared_page(key, c) < 0)
         return -1;
         
   
   return count;
}
