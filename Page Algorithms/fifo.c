#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;
int slot = 0;

extern struct frame *coremap;

/* Page to evict is chosen using the fifo algorithm
 * Returns the slot in the coremap that held the page that
 * was evicted.
 */

int fifo_evict(struct page *p) {
	//Evict the victim
	struct page *victim = find_page(coremap[slot].vaddr);
	victim->pframe = -1;
	//increment the slot with wrap around after the return
	int rSlot = slot;
	slot = (slot+1) % memsize;
	return rSlot;

}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void fifo_init() {
	slot = 0;
}