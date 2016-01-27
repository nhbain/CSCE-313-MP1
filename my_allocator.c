/* 
    File: my_allocator.c

    Author: Nicolas Bain
            Department of Computer Science
            Texas A&M University
    Date  : <date>

    Modified: 

    This file contains the implementation of the module "MY_ALLOCATOR".

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define node_size sizeof(struct Node)

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include "my_allocator.h"


/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

typedef struct Node node;
node *Block;
node **Freelist;
unsigned int min_size;
unsigned int max_size;
unsigned int total_size;
unsigned int number_of_sizes;
void *start_address;


/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FUNCTIONS FOR MODULE MY_ALLOCATOR */
/*--------------------------------------------------------------------------*/

/* Don't forget to implement "init_allocator" and "release_allocator"! */

unsigned int calc_sizes(unsigned min, unsigned max){
  unsigned count = 1;
  while(min < max){
    max = max >>1U;
    if(max < min)
      break;
    count++;
  }
  return count;
}

unsigned int init_allocator(unsigned int _basic_block_size, unsigned int _length){
  // Determine number of sizes 
  min_size = _basic_block_size;
  max_size = _length;
  number_of_sizes = calc_sizes(min_size, _length);
  total_size = _length + (number_of_sizes * node_size);
  printf("Number of sizes: %d\n",number_of_sizes);

  Block = (node *)malloc(max_size); // allocate memory of size _length
  start_address = (void *)Block;
  Freelist = (node **)malloc(sizeof(node*)*number_of_sizes);
  Freelist[number_of_sizes - 1] = Block;
  Block->status = 0;
  Block->size = _length;
  Block->next = NULL;
  Block->previous = NULL;
  printf("Block size: %d\n", Freelist[number_of_sizes - 1]->size);
  return _length;
}

/* Release Allocator */
int release_allocator(){
  free(Freelist);
  free(Block);
  return 0;
}

int insert_list(node* block){
  int index = calc_sizes(min_size, block->size);
  index--;
  node* temp = Freelist[index];
  if(temp !=NULL){
    block->next = temp;
    block->previous = temp->previous;
    temp->previous = block;
  }
  Freelist[index] = block;
  block->status = 0;
  return 0;
}

void remove_list(node* block){
  int index = calc_sizes(min_size, block->size);
  index--;
  if((block->previous != NULL) && (block->next != NULL)){
    node* temp = block->next;
    temp->previous = block->previous;
    block->previous->next = temp;
    block->previous = NULL;
    block->next = NULL;
    block->status = 1;
  }
  else if((block->previous != NULL) && (block->next == NULL)){
    block->previous->next = NULL;
    block->previous = NULL;
    block->status = 1;
  }
  else if((block->previous == NULL) && (block->next != NULL)){
    Freelist[index] = Freelist[index]->next;
    Freelist[index]->previous = NULL;
    block->next = NULL;
    block->status = 1;
  }
  else{
    Freelist[index] = NULL;
    block->status = 1;
  }
}

extern Addr my_malloc(unsigned int _length) {
  unsigned request = _length + node_size;
  printf("request size: %d\n", request);
  float x = request;
  x = log2(x);
  x+=.99;
  request = x;
  request = pow(2,request);
  printf("request size: %d\n", request);
  if(request < min_size)
    request = min_size; 
  
  /*Find appropriate block */
  unsigned request_size = request;
  int index = calc_sizes(min_size, request_size);
  index -=1;
  node *requested_block = Freelist[index];
  while (request_size <= max_size){
    int index = calc_sizes(min_size, request_size);
    index -=1;
    requested_block = Freelist[index];
    if(requested_block == NULL){
      request_size = request_size<<1U;
    }
    
    else if((requested_block != NULL)&&(requested_block->status == 1)){
      while((requested_block->status == 1)&&(requested_block->next != NULL)){
        requested_block = requested_block->next;
      }
    
      if((requested_block->status == 1)&&(requested_block->next == NULL)){
        request = request_size<<1U;
      }
    }
    else{
      remove_list(requested_block);
      break;
    }
  }
  printf("Size: %d\n", requested_block->size);
  /* ---------------------------- */
  int cont = 0;
  unsigned int byte_test = requested_block->size;
  if(byte_test>>1U < request){
    cont = 1;
  }
  /* Split and manage Freelist */
  while((requested_block->size > request) && cont == 0){
    unsigned int byte_flip = requested_block->size;
    void *address = (void *)requested_block - start_address;
    unsigned long b = (unsigned long)address;
    byte_flip = byte_flip>>1U;
    if(byte_flip>>1U < request){
      cont = 1;
    }
    // printf("byte_flip: %d\n",byte_flip);
    void* temp = (void*)(b ^ byte_flip);
    temp+= start_address;
    node* buddy = (node *)temp;
    // printf("test 1\n");
    buddy->size = byte_flip;
    // printf("test 2\n");
    requested_block->size = byte_flip;
    // printf("test 3\n");
    insert_list(buddy);
  }
  /* ---------------------------- */

  printf("Size: %d\n", requested_block->size);
 
  requested_block->status = 1;
  return (void*)(requested_block);
}

extern int my_free(Addr _a){
  /* Find Buddy */
  node *freed_block = (node*)((void*)_a);
  remove_list(freed_block);
  unsigned int byte_flip = freed_block->size;
  printf("byte_flip_free: %d\n", byte_flip);
  unsigned long b = (unsigned long)_a - start_address;
  byte_flip = byte_flip>>1U;
  void* buddy_a = (void*)(b ^ byte_flip);
  buddy_a+= start_address;
  node* buddy = (node *)buddy_a;
  printf("buddy size: %d\n", buddy->size);
  /* Buddy is Free! */
  if(buddy->status == 0){
    remove_list(buddy);
    unsigned int resize = freed_block->size;
    resize = resize <<1U;
    // buddy comes first
    if(buddy_a < _a){
      buddy->size = resize;
      insert_list(buddy);
      if(resize < max_size){
        void *new_addr = (void *)buddy;
        int go = my_free(buddy);
      }
    }
    //buddy comes second
    else{
      freed_block->size = resize;
      insert_list(freed_block);
      if(resize < max_size){
        void *new_addr = (void *)freed_block;
        int go = my_free(new_addr);
      }
    }

  }
  /* Buddy is being used */
  else{
    insert_list(freed_block);
  }

  // free(_a);
  return 0;
}

