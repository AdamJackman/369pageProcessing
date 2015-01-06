#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include "pagetable.h"

#define MAXLINE 256

int memsize = 0;
int position = 0;
int debug = 0;

int hit_count = 0;
int miss_count = 0;
int ref_count = 0;

int mHit = 0;
int baHit = 0;
int mMiss = 0;
int baMiss = 0;

//marker tracking
int before = 0;
int middle = 0;
int after = 0;
unsigned long mStart = 0x7ff0003b7 ;
unsigned long mEnd = 0x7ff0003b6;

//insert markers here
//matmul: 0x7ff0003b7 0x7ff0003b6
//blocked: 0x7ff0003b7 0x7ff0003b6
//simpleloop: 0x7ff0003c7 0x7ff0003c6


/* The algs array gives us a mapping between the name of an eviction
 * algorithm as given in a command line argument, and the function to
 * call to select the victim page.
 */
struct functions {
	char *name;
	void (*init)(void);
	int (*evict)(struct page *);
};

struct functions algs[] = {
	{"rand", rand_init, rand_evict}, 
	{"lru", lru_init, lru_evict},
	{"fifo", fifo_init, fifo_evict},
	{"clock",clock_init, clock_evict},
	{"opt", opt_init, opt_evict}
};
int num_algs = 5;

int (*evict_fcn)(struct page *) = NULL;
void (*init_fcn)() = NULL;


/* The coremap holds information about physical memory.
 * The index into coremap is the physical page number stored
 * as paddr in the page table entry (struct page).
 */
struct frame *coremap = NULL;
char *fil; 

int find_frame(struct page *p) {
	int i;
	int frame = -1;
	for(i = 0; i < memsize; i++) {
		if(!coremap[i].in_use) {
			frame = i;
			break;
		}
	}
	if(frame == -1) {
		// Didn't find a free page
		frame = evict_fcn(p);
	}
	coremap[frame].in_use = 1;
	coremap[frame].vaddr = p->vaddr;
	coremap[frame].type = p->type;
	coremap[frame].counter = 0; //for LRU
	coremap[frame].clockcounter = 1;  //implement the clock counter bit to 1 so it is protected
	return frame;
}

void access_mem(char type, addr_t vaddr) {
	ref_count ++;
	// make sure the page is in the page table
	struct page *p = pagetable_insert(vaddr, type);
	assert(p != NULL);

	// If p->pframe is -1 then the page is not in physical memory
	if(p->pframe == -1) {
		if (middle == 1){
			mMiss++;
		}
		else{
			if(after != 1){
			baMiss++;}
		}
		miss_count++;
		p->pframe = find_frame(p);
	} else {
		if (middle == 1){
			mHit++;
		}
		else{
			if(after != 1){
			baHit++;}
		}
		hit_count++;
		coremap[p->pframe].counter = 0; //for LRU
		coremap[p->pframe].clockcounter = 1; //for clock setting protected
	}
}

void replay_trace(FILE *infp) {
	char buf[MAXLINE];
	addr_t vaddr = 0;
	int length = 0;
	char type;
	before = 1;

	while(fgets(buf, MAXLINE, infp) != NULL) {
		if(buf[0] != '=') {
			sscanf(buf, " %c %lx,%u", &type, &vaddr, &length);
			//check for a marker
			if (vaddr == mStart && before == 1){
				before = 0;
				middle = 1;
			}
			else if (vaddr == mEnd && middle == 1){
				middle = 0;
				after = 1;
			}

			//re-adjust vAddr to the page len - 4096 sized address masking
			char str[10];
			sprintf(str, "%lx", vaddr);
			char tstr[7];
			strncpy(tstr, str, 7);
			//re-adjust mStart and mEnd


			//the vAddr will now cover a full page instead of direct addresses
			vaddr = strtoul(tstr, NULL, 16);	
			if(debug)  {
				printf("%c %lx, %u\n", type, vaddr, length);
			}
			access_mem(type, vaddr); 
			position = position + 1;
		} else {
			continue;
		}

	}
}


int main(int argc, char *argv[]) {
	int opt;
	char *tracefile = NULL;
	FILE *tfp = stdin;
	char *replacement_alg = NULL;
	char *usage = "USAGE: sim -f tracefile -m memorysize -a algorithm\n";

	while ((opt = getopt(argc, argv, "f:m:a:")) != -1) {
		switch (opt) {
		case 'f':
			tracefile = optarg;
			break;
		case 'm':
			memsize = (int)strtol(optarg, NULL, 10);
			break;
		case 'a':
			replacement_alg = optarg;
			break;
		default:
			fprintf(stderr, "%s", usage);
			exit(1);
		}
	}
	if(tracefile != NULL) {
		fil = tracefile;
		if((tfp = fopen(tracefile, "r")) == NULL) {
			perror("Error opening tracefile:");
			exit(1);
		}
		//udr access for opt here
	}

	if(replacement_alg == NULL) {
		fprintf(stderr, "%s", usage);
		exit(1);
	} else {
		int i;
		for (i = 0; i < num_algs; i++) {
			if(strcmp(algs[i].name, replacement_alg) == 0) {
				init_fcn = algs[i].init;
				evict_fcn = algs[i].evict;
				break;
			}
		}
		if(evict_fcn == NULL) {
			fprintf(stderr, "Error: invalid replacement algorithm - %s\n", 
					replacement_alg);
			exit(1);
		}
	}

		
	init_pagetable();
	coremap = malloc(memsize * sizeof(struct frame));

	init_fcn();
	replay_trace(tfp);
	//print_pagetable();


	printf("\n");
	printf("Hit count: %d\n", hit_count);
	printf("Miss count: %d\n", miss_count);
	printf("Total references : %d\n", ref_count);
	printf("Total Hit rate: %.1f\n", (double)hit_count/ref_count * 100);
	printf("Total Miss rate: %.1f\n\n", (double)miss_count/ref_count *100);
	printf("Middle Hit count: %d\n", mHit);
        printf("Middle Miss count: %d\n", mMiss);
        printf("Middle Hit rate: %.1f\n", (double)mHit/(mHit+mMiss));

	printf("Bef Hit Count: %d\n", baHit);
	printf("Bef Miss Count: %d\n", baMiss);
	printf("Bef Hit rate: %.1f\n", (double)baHit/(baHit+baMiss));
	
	return(0);
}