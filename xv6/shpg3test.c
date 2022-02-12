//shpg3test

/*
First process A requests 5 shared pages with key 500 and gets pages with PFNs of a, b, c, d, e
Process A writes on all 5 pages using 5120 integers
   -> each integer is 4 bytes so a 4096 byte page can hold 1024 integers
   -> 5 pages can hold 5120 integers
Process A forks process B who still has access to the shared pages of PFNs a, b, c, d, e (inherited by A)
Process B reads from these pages
Process B then requests 3 mores shared pages with the same key and thus gets back pages with PFNS c, d, e
   -> B's virtual address space has 8 pages with PFNs of a, b, c, d, e, c, d, e (the virtual pages with the same PFNs are synced)
Process B then writes on these 3 pages, multipying the integer values by 2
Process A reaps process B then reads its 5 shared pages (PFNS a, b, c, d, e) with 3 of them (PFNS c, d, e) having values overwritten by process B
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
  //process A
  int num_of_pages = 5;
  int *a = (int *)GetSharedPage(500, num_of_pages); //requests 5 pages
  for(int i = 0; i < PGSIZE * num_of_pages / sizeof(int) ; i++) //writes on 5 pages
     a[i] = i;
  for(int i = 0; i < num_of_pages; i++)
     printf(1, "Parent says: at a[%d] on page %d is %d\n", i, i * PGSIZE / sizeof(int), a[i * PGSIZE / sizeof(int)]); //reads first int on each page
     
  if(fork() == 0)
  {
     //process B
     for(int i = 0; i < num_of_pages; i++)
         printf(1, "Child says: at a[%d] on page %d is %d\n", i, i * PGSIZE / sizeof(int), a[i * PGSIZE / sizeof(int)]); //reads first int on each page inherited by parent
     
     num_of_pages = 3;
     int *b = (int *)GetSharedPage(500, num_of_pages); //requests 3 new pages synicing its first 3 with these
     for(int i = 0; i < PGSIZE * num_of_pages / sizeof(int) ; i++) //writes on ONLY these 3 pages
        b[i] *= 2;
     exit();
  }
  //process A
  wait();
  for(int i = 0; i < num_of_pages; i++)
     printf(1, "Parent says: at a[%d] on page %d is %d\n", i, i * PGSIZE / sizeof(int), a[i * PGSIZE / sizeof(int)]); //reads first int on each page
  exit();
}
