#include "types.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

//data

//struct *shared_page* is defined in defs.h
//array of shared_page, *all_shared_pages*, is declared in defs.h

//functions




//init_shared_page zeros out all the metadata in the array all_shared_pages
//is used in userinit() of proc.c
void init_shared_page(void)
{
   int pg_index; //page index
   for(pg_index = 0; pg_index < MAXSHPAGES; pg_index++)
   {
      all_shared_pages[pg_index].phy_add = 0;
      all_shared_pages[pg_index].key = 0;
      all_shared_pages[pg_index].page_number = 0;
      memset(all_shared_pages[pg_index].procs_sh_page, 0, sizeof all_shared_pages[pg_index].procs_sh_page);
      memset(all_shared_pages[pg_index].vir_add_of_page, 0, sizeof all_shared_pages[pg_index].vir_add_of_page);
   }
}

//takes a key_int key, page_number, and pos
//searches for a page with a corresponding key & page_number in the all_shared_pages array
//if found it adds the page to the calling process's virtual address through sh_bit_map at pos then returns the virtual address
//else it tries to create a new page; it searches the all_shared_pages array for a free page (a page with phy_add == 0) and kallocs a page
//if kalloc works, it adds the proc to the shared_page procs_sh_page array, adds the page to the calling process's virtual address through sh_bit_map at pos then returns the virtual address
//if all else fails, it returns -1
int find_shared_page(key_int key, int page_number, int pos)
{
   //cprintf("Key is %d and Page Number is %d\n", key, page_number);
   int pg_index;
   struct proc* calling_proc = myproc();
   char* mem = 0;
   int virtual_address = 0;
   
   //search the all_shared_pages array for a page that has a similar key and the same page number
   for(pg_index = 0; pg_index < MAXSHPAGES; pg_index++){

      if(all_shared_pages[pg_index].key == key && all_shared_pages[pg_index].phy_add != 0
      && all_shared_pages[pg_index].page_number == page_number) //page_numbers must also match!
      {
         //if found, then get the virtual address
         mem = P2V(all_shared_pages[pg_index].phy_add);
         
         //append the process to the array of procs_sh_page
         int proc_index;
         for(proc_index = 0; proc_index < MAXPROCSSHARE; proc_index++)
         {
            if(all_shared_pages[pg_index].procs_sh_page[proc_index] == 0){
               all_shared_pages[pg_index].procs_sh_page[proc_index] = calling_proc;
               break;
            }
         }
         //else (for loop) too many processes sharing the page
         if(proc_index == MAXPROCSSHARE)
         {
            cprintf("ERROR: too many processes sharing this page\n");
            return -1;
         }
         
         //update sh_bit_map at the given pos
         short newShort = 1 << (7-pos%(sizeof(short)*4)); //NEW 
         calling_proc->sh_bit_map[pos/ (sizeof(short)*4) ] |= newShort;
         //get the virtual address
         virtual_address = KERNBASE - PGSIZE * (pos + 1); //NEW 
         
         //save the virtual address to vir_add_of_page
         all_shared_pages[pg_index].vir_add_of_page[proc_index] = virtual_address;

         //map that virtual address to a physical one thru pgdir of calling_proc - check out mappages
         //if it fails then return an error
         if( getMappages(calling_proc->pgdir, (char*)virtual_address, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0)
         {
            cprintf("ERROR: mappages() failed\n");
            return -1;
         }
         
         //finally return the addresss
         
         return virtual_address;
      }
   }
   //if it manages to get through the entire for loop above without errors or the final return, then this must be a new shared page
   for(pg_index = 0; pg_index < MAXSHPAGES; pg_index++)
   {
      if(all_shared_pages[pg_index].phy_add == 0) //if a free page is found, allocate page
      {
         if((mem = kalloc()) == 0) //if allocation did not work
         {
            cprintf("ERROR: kalloc() failed");
            return -1;
         }
         
         memset(mem, 0 , PGSIZE); //zero out the page!!!
         all_shared_pages[pg_index].key = key; //copy the key
         all_shared_pages[pg_index].page_number = page_number; //copy the page_number
         all_shared_pages[pg_index].phy_add = V2P(mem); //get the new physical address
         
         //append the process to the array of procs_sh_page
         int proc_index;
         for(proc_index = 0; proc_index < MAXPROCSSHARE; proc_index++)
         {
            if(all_shared_pages[pg_index].procs_sh_page[proc_index] == 0){
               all_shared_pages[pg_index].procs_sh_page[proc_index] = calling_proc;
               break;
            }
         }
         //else (for loop) too many processes sharing the page
         if(proc_index == MAXPROCSSHARE)
         {
            cprintf("ERROR: too many processes sharing this page\n");
            return -1;
         }
         
         //update sh_bit_map at the given pos
         short newShort = 1 << (7-pos%(sizeof(short)*4)); //NEW 
         calling_proc->sh_bit_map[pos/ (sizeof(short)*4) ] |= newShort;
         //get the virtual address
         virtual_address = KERNBASE - PGSIZE * (pos + 1); //NEW 
 
         //save the virtual address to vir_add_of_page
         all_shared_pages[pg_index].vir_add_of_page[proc_index] = virtual_address;

         //map that virtual address to a physical one thru pgdir of calling_proc - check out mappages
         //if it fails then return an error
         if( getMappages(calling_proc->pgdir, (char*)virtual_address, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0)
         {
            cprintf("ERROR: mappages() failed\n");
            return -1;
         }
 
         //finally return the addresss
         return virtual_address;
      }
   }
   
   //if it went through both for loops, then the max pages have been allocated and we maxed out
   cprintf("ERROR: MAXSHPAGES (max shared pages) reached\n");
   return -1;
}

//bitmap sh_bit_map declared proc.h
//bitmap sh_bit_map initialized in allocproc of proc.c
//search the process's bitmap sh_bit_map for an open spot
         
//takes the count of shared pages that was requested to allocate and returns the first_pos on the sh_bit_map that can fit all of them
//returns -1 if there is no spot
int where_does_fit(int count)
{
   int first_pos;
   struct proc* calling_proc = myproc();
   
   for(int pos = 0; pos < MAXSHPAGES; pos++) //go through each bit
   {
      first_pos = pos; //save first_pos
      int i;
      for(i = pos; i < count + pos; i++) //check to see if what comes next can fit the count of pages requested
         //if the calling_proc's sh_bit_map at search index is not 0 then break
         if( (calling_proc->sh_bit_map[i / (sizeof(short)*4)] >> (7-i%((sizeof(short)*4))) & 1) ){
            break;
      }
      if(i == count + pos) //else (for loop) once a pos is found that fits, return the first_pos
         return first_pos;
   }
   
   return -1; //if no pos is available return -1
}

//takes a key_int key and an int page_number, then returns 1 if successfully removed and possibly freed or -1 if invalid
//searches for a page with a corresponding key and page_number in the all_shared_pages array; removes proc calling_proc from proc_sh_pg
//if no procs were removed then the key must be invalid so it returns -1
//else, unmap from that proc's pgdir
//update the sh_bit_map bit map
//if it was the last proc that was sharing the page, then go head and kfree
int remove_shared_page(key_int key, int page_number)
{
   int pg_index;
   struct proc* calling_proc = myproc();
   
   //search all_shared_pages array
   for(pg_index = 0; pg_index < MAXSHPAGES; pg_index++)
   {
      //if the key and page_number are found
      if(all_shared_pages[pg_index].key == key && all_shared_pages[pg_index].page_number == page_number)
      {
         int proc_index;
         //search through the proc_sh_page array for the calling_proc
         for(proc_index = 0; proc_index < MAXPROCSSHARE; proc_index++)
         {
            if(all_shared_pages[pg_index].procs_sh_page[proc_index] == calling_proc)
            {
               all_shared_pages[pg_index].procs_sh_page[proc_index] = 0; //unset proc
               
               //save the virtual_address
               int virtual_address = all_shared_pages[pg_index].vir_add_of_page[proc_index];

               all_shared_pages[pg_index].vir_add_of_page[proc_index] = 0; //unset vir add
               
               //remove mapping in pgdir so the page is no longer accessable
               pte_t* pte = getWalkpgdir(calling_proc->pgdir,(void *)virtual_address, 0);
               *pte = 0;
               
               //look for the vir add in the bitmap and unset the bit
               int pos = 0;
               int pg;
               uint sh_mem_start = KERNBASE - PGSIZE;
               uint sh_mem_end = KERNBASE - MAXSHPAGES * PGSIZE;
               
               for(pg = sh_mem_start; pg >= sh_mem_end; pg -= PGSIZE, pos++)
               {
                  if( pg == virtual_address){
                     short temp = 1 << (7-pos%((sizeof(short)*4)));
	             calling_proc->sh_bit_map[pos/ (sizeof(short)*4) ] &= ~temp;
	             break;
	          }
               }
               
               //run zero_procs, if returns true then free the page
               if(zero_procs(key, page_number))
               {
                  //cprintf("Freeing page %d under key %d\n", page_number, key); //DEBUG
                  
                  all_shared_pages[pg_index].key = 0;
                  all_shared_pages[pg_index].page_number = 0;

                  kfree( P2V(all_shared_pages[pg_index].phy_add) );
                  all_shared_pages[pg_index].phy_add = 0;
               }
               
               //finally return 1 signifying a successsful removal
               return 1;
            }
         }
         if(proc_index == MAXPROCSSHARE) //else (for loop) key and page_number exist but
         {
            cprintf("ERROR: key exists but does not belong to calling process\n");
            return -1;
         }
      }
   }
   
   //the key must be invalid
   cprintf("ERROR: key does not exist\n");
   return -1;
}

//returns 1 if there are no procs sharing this page, returns 0 if there is at least on proc with this page
int zero_procs(key_int key, int page_number)
{
   for(int pg_index = 0; pg_index < MAXSHPAGES; pg_index++)
      if(all_shared_pages[pg_index].key == key && all_shared_pages[pg_index].page_number == page_number)
         for(int proc_index = 0; proc_index < MAXPROCSSHARE; proc_index++)
            if(all_shared_pages[pg_index].procs_sh_page[proc_index] != 0)
               return 0;
   return 1;
}

//takes a key and calling_proc to search all_shared_pages and returns the number of pages that it is using under said key
int get_num_sh_procs(key_int key)
{
   struct proc* calling_proc = myproc();
   int page_number = 0;
   
   for(int pg_index = 0; pg_index < MAXSHPAGES; pg_index++)
   {
      if(all_shared_pages[pg_index].key == key)
         for(int proc_index = 0; proc_index < MAXPROCSSHARE; proc_index++)
            if(all_shared_pages[pg_index].procs_sh_page[proc_index] == calling_proc)
            {
               page_number++;
               break;
            }
   }
   
    return page_number;
}

//returns the first key it finds for a calling proc; returns 0 if it can't find a key
//this is used in exit() to find all the keys of an exiting proc to free them
//exit() uses these keys to call remove_shared_page
int get_proc_key()
{
   struct proc* calling_proc = myproc();
   for(int pg_index = 0; pg_index < MAXSHPAGES; pg_index++)
      for(int proc_index = 0; proc_index < MAXPROCSSHARE; proc_index++)
         if(all_shared_pages[pg_index].procs_sh_page[proc_index] == calling_proc)
            return all_shared_pages[pg_index].key;
   return 0;
}





