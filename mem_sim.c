/***************************************************************************
 * *    Inf2C-CS Coursework 2: Cache Simulation
 * *
 * *    Instructor: Boris Grot
 * *
 * *    TA: Siavash Katebzadeh
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
/* Do not add any more header files */

/*
 * Various structures
 */
typedef enum {FIFO, LRU, Random} replacement_p;

const char* get_replacement_policy(uint32_t p) {
    switch(p) {
    case FIFO: return "FIFO";
    case LRU: return "LRU";
    case Random: return "Random";
    default: assert(0); return "";
    };
    return "";
}

typedef struct {
    uint32_t address;
} mem_access_t;

// These are statistics for the cache and should be maintained by you.
typedef struct {
    uint32_t cache_hits;
    uint32_t cache_misses;
} result_t;


/*
 * Parameters for the cache that will be populated by the provided code skeleton.
 */

replacement_p replacement_policy = FIFO;
uint32_t associativity = 0;
uint32_t number_of_cache_blocks = 0;
uint32_t cache_block_size = 0;


/*
 * Each of the variables below must be populated by you.
 */
uint32_t g_num_cache_tag_bits = 0;
uint32_t g_cache_offset_bits= 0;
result_t g_result;


/* Reads a memory access from the trace file and returns
 * 32-bit physical memory address
 */
mem_access_t read_transaction(FILE *ptr_file) {
    char buf[1002];
    char* token = NULL;
    char* string = buf;
    mem_access_t access;

    if (fgets(buf, 1000, ptr_file)!= NULL) {
        /* Get the address */
        token = strsep(&string, " \n");
        access.address = (uint32_t)strtoul(token, NULL, 16);
        return access;
    }

    /* If there are no more entries in the file return an address 0 */
    access.address = 0;
    return access;
}

void print_statistics(uint32_t num_cache_tag_bits, uint32_t cache_offset_bits, result_t* r) {
    /* Do Not Modify This Function */

    uint32_t cache_total_hits = r->cache_hits;
    uint32_t cache_total_misses = r->cache_misses;
    printf("CacheTagBits:%u\n", num_cache_tag_bits);
    printf("CacheOffsetBits:%u\n", cache_offset_bits);
    printf("Cache:hits:%u\n", r->cache_hits);
    printf("Cache:misses:%u\n", r->cache_misses);
    printf("Cache:hit-rate:%2.1f%%\n", cache_total_hits / (float)(cache_total_hits + cache_total_misses) * 100.0);
}

/*
 *
 * Add any global variables and/or functions here as needed.
 *
 */
 void replaceItem(int** cash, uint32_t tag, uint32_t index) {
     int i; //for loop
     int stopLoop = 1; //to stop loop

     if (replacement_policy == LRU || replacement_policy == FIFO) {
       for (i = 1; i < associativity * 2 && stopLoop == 1; i += 2) { // for going through the order
         if (cash[index][i] == 0) { //checks for least recently used for LRU and first in for FIFO
           cash[index][i-1] = tag; //sets to new tag
           cash[index][i] = associativity; //sets order to most recent for LRU and last in for FIFO
           stopLoop = 0; //just stops loop
         }
       }
       for (i = 1; i < associativity * 2; i+= 2) {//change order
         cash[index][i]--;//fixs order after new item was added
       }
     }
     else { //for Random
       i = rand() % associativity; // 0 <= i < associativity
       cash[index][(i*2)] = tag; //since cache stores order after tag need to times it by 2.
       //don't care about the order for random
     }

 }


int main(int argc, char** argv) {
    time_t t;
    /* Intializes random number generator */
    /* Important: *DO NOT* call this function anywhere else. */
    srand((unsigned) time(&t));
    /* ----------------------------------------------------- */
    /* ----------------------------------------------------- */

    /*
     *
     * Read command-line parameters and initialize configuration variables.
     *
     */
    int improper_args = 0;
    char file[10000];
    if (argc < 6) {
        improper_args = 1;
        printf("Usage: ./mem_sim [replacement_policy: FIFO LRU Random] [associativity: 1 2 4 8 ...] [number_of_cache_blocks: 16 64 256 1024] [cache_block_size: 32 64] mem_trace.txt\n");
    } else  {
        /* argv[0] is program name, parameters start with argv[1] */
        if (strcmp(argv[1], "FIFO") == 0) {
            replacement_policy = FIFO;
        } else if (strcmp(argv[1], "LRU") == 0) {
            replacement_policy = LRU;
        } else if (strcmp(argv[1], "Random") == 0) {
            replacement_policy = Random;
        } else {
            improper_args = 1;
            printf("Usage: ./mem_sim [replacement_policy: FIFO LRU Random] [associativity: 1 2 4 8 ...] [number_of_cache_blocks: 16 64 256 1024] [cache_block_size: 32 64] mem_trace.txt\n");
        }
        associativity = atoi(argv[2]);
        number_of_cache_blocks = atoi(argv[3]);
        cache_block_size = atoi(argv[4]);
        strcpy(file, argv[5]);
    }
    if (improper_args) {
        exit(-1);
    }
    assert(number_of_cache_blocks == 16 || number_of_cache_blocks == 64 || number_of_cache_blocks == 256 || number_of_cache_blocks == 1024);
    assert(cache_block_size == 32 || cache_block_size == 64);
    assert(number_of_cache_blocks >= associativity);
    assert(associativity >= 1);

    printf("input:trace_file: %s\n", file);
    printf("input:replacement_policy: %s\n", get_replacement_policy(replacement_policy));
    printf("input:associativity: %u\n", associativity);
    printf("input:number_of_cache_blocks: %u\n", number_of_cache_blocks);
    printf("input:cache_block_size: %u\n", cache_block_size);
    printf("\n");

    /* Open the file mem_trace.txt to read memory accesses */
    FILE *ptr_file;
    ptr_file = fopen(file,"r");
    if (!ptr_file) {
        printf("Unable to open the trace file: %s\n", file);
        exit(-1);
    }

    /* result structure is initialized for you. */
    memset(&g_result, 0, sizeof(result_t));

    /* Do not delete any of the lines below.
     * Use the following snippet and add your code to finish the task. */

    /* You may want to setup your Cache structure here. */

    int off_bits_num = log2(cache_block_size);
    g_cache_offset_bits = off_bits_num;
    int index_bits_num = log2(number_of_cache_blocks/associativity);
    if (index_bits_num == 0) {index_bits_num = 1;}
    int tag_bits_num = 32 - off_bits_num - index_bits_num;
    g_num_cache_tag_bits = tag_bits_num;
    int x = number_of_cache_blocks;
    int y = (associativity * 2) - 1;
    int **cash; //2d array representing cache. outer array represents the index and inner the block.
    //block is stored as tag then order then tag.... order depends on replacement_policy.
    cash = malloc(x * sizeof *cash);
    int pop1, pop2; //
    for (pop1 = 0; pop1 <= x; pop1++) {
      cash[pop1] = malloc(y * sizeof *cash[pop1]);
      for (pop2 = 0; pop2 <= y; pop2++) {
        cash[pop1][pop2] = -1;
      }
    }
    int full, emptyCount, i, j, stopLoop; //i and j are used in loops and the rest are booleans

    mem_access_t access;
    /* Loop unticashl the whole trace file has been read. */
    while(1) {
        access = read_transaction(ptr_file);
        if (access.address == 0)
            break;

        uint32_t tag = access.address >> (off_bits_num + index_bits_num);
        uint32_t index = access.address << tag_bits_num;
        index = index >> (tag_bits_num + off_bits_num);
        full = 1;
        stopLoop = 0;
        emptyCount = 1;


        for (i = 0; i < associativity * 2 && stopLoop == 0; i += 2) { //to loop through block
          if (cash[index][i] == -1 || cash[index][i] == tag) { //check if tags match or there is an empty space.
            //Don't need to worry about tags being after empty spaces

            if (cash[index][i] == tag) {
              g_result.cache_hits++; //adds a hit when tag is found
            }
            else {
              g_result.cache_misses++;
              cash[index][i] = tag; //assigns tag to empty space
              if (replacement_policy == FIFO) {
                for (j = 1; j < associativity * 2; j += 2) { //for counting number of empty spaces in block
                  if (cash[index][j] == -1 && j != i+1){
                    emptyCount++;
                  }
                }
                cash[index][i + 1] = associativity - emptyCount; //assigns order for when tag was placed in
              }
            }

            if (replacement_policy == LRU) {
              for (j = 1; j < associativity * 2; j += 2) {
                  if (cash[index][i+1] < cash[index][j] && cash[index][i+1] != -1) { //for matched tags. only change order of other tags that were higher
                    cash[index][j]--;
                  }
                  else if (cash[index][j] == -1 && j != i+1) { //counts empty spaces
                    emptyCount++;
                  }
              }
              cash[index][i + 1] = associativity - emptyCount; //makes new tag the most recent
            }

            full = 0; //checking if tag was matched or there was an empty space in block
            stopLoop = 1;
          }

        }

        if (full == 1) { //if tag wasn't matched and needs to clear a space in block
          g_result.cache_misses++;
          replaceItem(cash, tag, index); //fuction for removing item from the block
        }
    }
    for (i = 0; i <= x; i++) { //nested loop for populating the 2D array
      free(cash[i]);
    }
    free(cash);
    /* Do not modify code below. */
    /* Make sure that all the parameters are appropriately populated. */
    print_statistics(g_num_cache_tag_bits, g_cache_offset_bits, &g_result);

    /* Close the trace file. */
    fclose(ptr_file);
    return 0;
}
