#include "ackerman.h"
#include "my_allocator.h"
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void Final(void){
	release_allocator();
}

int main(int argc, char  ** argv){

	// input parameters (basic block size, memory length)
	int b = 128;
	int s = 512000;
	int c = 0;
	
	while((c = getopt(argc, argv, "b:s:")) != -1){
		switch(c){
			case 'b':
				b = atoi(optarg);
				if(b < 32)
					b = 32;
				break;
			case 's':
				s = atoi(optarg);
				break;
		}
	}	
	printf ("basic block size = %d, total memory = %d\n", b, s);

	init_allocator(b, s);
  	unsigned int test = 2031;
 	void* mem;
  	mem = my_malloc(test);
  	int f = my_free(mem);
  	void* mem2;
  	mem2 = my_malloc(test);
  	int f2 = my_free(mem2);
	
	// ackerman_main();
	atexit(Final);
	return 0;
}
