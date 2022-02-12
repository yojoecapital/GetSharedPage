//shpg6test
/*
This test makes sure it is mapping the pages in the right spots in the address space
The process requests 1 page under key 111 so sh_bit_map looks as 'A 0 0 0 0 ...' where 'A' is the 1 bit belonging to key 111. Then it writes 111 across the page.
Next, it requests 1 more page under key 222 so sh_bit_map looks as 'A B 0 0 0 ...' where 'B' is the 1 bit belonging to key 222. Then it writes 222 across the page.
Next, it calls FreeSharedPage(111) so h_bit_map looks as '0 B 0 0 0 ...'
Lastly, it requests 2 more pages under key 333 so sh_bit_map looks as '0 B C C 0 ...' where 'C' are the bits belonging to key 333. Then it writes 333 across the page. 
To prove they are placed correctly and not as  'C B C 0 0 ...' the process reads from the pages using pointer arithmetic
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
  int *a = (int *)GetSharedPage(111, 1); //request 1 page
  
  for(int i = 0; i < PGSIZE * 1 / sizeof(int) ; i++) //writes on 1 page
     a[i] = 111;
  
  int *b = (int *)GetSharedPage(222, 1); //request 1 page
  
  for(int i = 0; i < PGSIZE * 1 / sizeof(int) ; i++) //writes on 1 page
     b[i] = 222;
  
  FreeSharedPage(111);
  
  int *c = (int *)GetSharedPage(333, 2); //request 2 pages
  
  for(int i = 0; i < PGSIZE * 2 / sizeof(int) ; i++) //attempts to write on 2 pages
     c[i] = 333;
     
  for(int i = 0; i < 2; i++)
     printf(1, "At c[%d] on page %d is %d\n", i, i * PGSIZE / sizeof(int), c[i * PGSIZE / sizeof(int)]); //reads first int on each page c
  
  exit();
  
}

