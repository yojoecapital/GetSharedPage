//shpg1test
/*
Process A requests 1 shared page under key 111 and writes "CS450 is very easy!\0"
Process A forks B who execs shpg2test
In shpg2test, B requests the same shared page under key 111 and overwrites it with "CS450 is very hard!\0"
Process A reaps B and reads what B has overwritten the page with
*/
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "syscall.h"

int
main(int argc, char *argv[])
{
  char *a = (char *)GetSharedPage(111, 1);
  char string[20] = "CS450 is very easy!\0";
  for(int i = 0; i < 20; i++)
     a[i] = string[i];
  printf(1, "Process 1 wrote: %s\n", a);
  
  if(fork() == 0){
      char *argv[2];
      argv[0] = "shpg2test";
      exec("shpg2test", argv);
      exit();
  }
  wait();
  
  printf(1, "Process 1 sees that process 2 wrote: %s\n", a);
  
  exit();
}



