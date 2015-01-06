#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

/* Page to evict is chosen using the accurate LRU algorithm 
 * Returns the slot in the coremap that held the page that
 * was evicted.
 */

int lru_evict(struct page *p) {
	//The idea is to maintain a counter that will keep counting down when the page is not chosen 
	//when the page is called the new counter is put onto it.
	//to evict we simply look for the lowest counter and remove it
	int lowest = 0;
	int chosen = 0;
	int i = 0;
	for (; i< memsize; i++){
		if (coremap[i].counter < lowest){
			lowest = coremap[i].counter;
			chosen = i;
		}
		coremap[i].counter--;
	}

	struct page *victim = find_page(coremap[chosen].vaddr);
	victim->pframe = -1;
	return chosen;
	
	return 0;

}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {

}