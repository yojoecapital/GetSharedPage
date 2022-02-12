//shpg2test
/*
Is exec'ed in shpg1test
*/
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "syscall.h"
#include "mmu.h"

int
main(int argc, char *argv[])
{
  char *b = (char *)GetSharedPage(111, 1);
  printf(1, "Process 2 sees that process 1 wrote: %s\n", b);
  char string[20] = "CS450 is very hard!\0";
  for(int i = 0; i < 20; i++)
     b[i] = string[i];
  printf(1, "Process 2 wrote: %s\n", b);
  
  exit();
  
}



