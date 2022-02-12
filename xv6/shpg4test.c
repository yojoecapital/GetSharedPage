//shpg4test
/*
Silly loop
Process A requests 1 shared page under key 313, writes integer value 20 in it
A forks B who exec's shpg5test
In shpg2test, B requests the same shared page under key 313 then decrements the integer value, prints, and frees its shared page
B execs shpg2test again and loops until the integer value is 0
A reaps B and prints the 0 integer value

To prove the pages are being freed by FreeSharedPage(), the original integer value 20 is greater than 16
exec() does not free the page so it does not remove the procs from procs_sh_page in all_shared_pages
Thus, if they were not being freed, we'd get an error returned
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
  *a = 20;
  
  if(fork() == 0){
     char *argv[2];
     argv[0] = "shpg5test";
     exec("shpg5test", argv);
     exit();
  }
  wait();
  
  printf(1, "From first process: a now points to %d\n", *a);
  
  exit();
  
}

