#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

extern char *fil;

extern int position;
/* Page to evict is chosen using the accurate optimal algorithm 
 * Returns the slot in the coremap that held the page that
 * was evicted.
 */

struct vAddrList{
        long int vAddr;
        struct vAddrList *next;
};

struct vAddrList *first;

int opt_evict(struct page *p) {

	//printing out the list:
	struct vAddrList * curr = first;

	int curpos = 0;
	//The position holds the current position in the trace so we start by going there
	while (curr != NULL){
		if (curpos < position){
			curpos = curpos +1;
			if (curr->next != NULL){
				//already been passed so skip to next addr
				curr = curr->next;
				continue;
			}
			else{
				//error should never occur
				break;
			}
		}
		//Addresses are now future addresses
		//1. Check if the vaddr matches a vaddr in the coremap
		int vcheck;
		for(vcheck = 0; vcheck < memsize; vcheck++){
			
			//address to page convertion
			//moved into sim.c
			//
			char str[10];
			char str2[10];
			sprintf(str, "%lx", curr->vAddr);
			sprintf(str2,"%lx", coremap[vcheck].vaddr);
			char tstr[7];
			char tstr2[7];
			strncpy(tstr, str, 7);
			strncpy(tstr2, str2, 7);
			unsigned long fin, fin2;
			fin = strtoul(tstr, NULL, 16);
			fin2 = strtoul(tstr2, NULL, 16);

			//if(strcmp(tstr, tstr2) == 0){
			if(fin == fin2){
				coremap[vcheck].counter = 1;
			}
		}
		//2. check if there is only one left, it is logically the entry that will apear again
		// in the most amount of time
		int left = memsize;
		int j;
		for (j = 0; j < memsize; j++){
			if(coremap[j].counter == 1){
				left = left - 1;
			}
		}
		if(left == 1){
			//return the victim
			for (j = 0; j < memsize; j++){
				if(coremap[j].counter == 0){
					//this is the victim
					struct page *victim = find_page(coremap[j].vaddr);
				        victim->pframe = -1;
				
					//reset the counters
					int r1;
					for(r1 = 0; r1< memsize; r1++){
						coremap[r1].counter = 0;
					}
					return j;
				}
			}
		}
		//3. there is more than one left so we must check the next address in the trace
		// increment after to check the next
		curr = curr->next;
	}
	//4. This is the case that there is are multiple that are never seen again
	//evict the first problem child
	int i;
	for(i = 0; i < memsize; i++){
		if (coremap[i].counter == 0){
			//this is the victim
			struct page *victim = find_page(coremap[i].vaddr);
                        victim->pframe = -1;

			//reset the counters
			int r2;
                        for(r2 = 0; r2< memsize; r2++){
                         	coremap[r2].counter = 0;
                        }
			return i;
		}
	}

	return 0;
}



/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	//open up the trace file
	struct vAddrList *head = NULL;	

	FILE *tfp;
	char buf[256];

	if((tfp = fopen(fil, "r")) == NULL) {
        	perror("Error opening tracefile:");
                exit(1);
        }
	//read the file
	char type;
	addr_t vaddr = 0;
	int length = 0;
	//
	while(fgets(buf, 256, tfp) != NULL) {
                if(buf[0] != '=') {
                        sscanf(buf, " %c %lx,%u", &type, &vaddr, &length);
                        if(debug)  {
                                printf("%c %lx, %u\n", type, vaddr, length);
                        }
			//store vaddr in the list
                        if (head == NULL){
				//populate the first trace vAddr
				head = (struct vAddrList*) malloc(sizeof(struct vAddrList));
				head->vAddr = vaddr;
				head->next = NULL;
				first=head;
			}
			else{
				//now add the vAddr to the existing head, and move the head
				struct vAddrList *temp = (struct vAddrList*) malloc(sizeof(struct vAddrList));
				temp->vAddr = vaddr;
				temp->next = NULL;
				head->next = temp;
				head = temp;
				
			}
                } else {
                        continue;
                }
	}	// The linked list is now complete and is started at first


}