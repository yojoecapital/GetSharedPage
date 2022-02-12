//shpg5test
/*
Is exec'ed in shpg4test
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
  int *a = (int *)GetSharedPage(313, 1);
  
  if(*a > 0)
  {
     printf(1, "a now points to %d\n", *a);
     *a -= 1;
     FreeSharedPage(313);
     
     char *argv[2];
     argv[0] = "shpg5test";
     exec("shpg5test", argv);
     exit();
  }
  
  exit();
  
}

