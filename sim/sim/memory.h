#ifndef	MEMORY_H
#define	MEMORY_H

#include "main.h"
//#include "simulator.h"
/*this module includes all the functions and structs that belong to the main memory and the cache*/


/*********************MACROS*****************/
//sizes
#define MainMemorySize 1048576//equal to 2^20
#define CHACHE_SIZE 256

#define TAG_BITS 4096 //12 first bits

//block modes in cache
#define INVALID 0
#define SHARED 1	
#define MODIFIED 2

#define PC_MASK(PC) 0X3FF&PC

typedef enum { NO_COMMAND=0, BUSRD=1, BUSRDX=2, FLUSH=3} bus_commands;

/*********************STRUCTS*****************/

typedef struct Cache {
	int DSRAM[CHACHE_SIZE];
	int TSRAM[CHACHE_SIZE];
}CACHE;

typedef struct Core {
	reg reg_old;
	reg reg_new;
	FILE* IMEM;
	CACHE  cache;
	int index;//core index {0-3}
}CORE;


/*********************FUNCTIONS DECLARATION	*****************/
int address_in_cache(int address, CACHE* cache);
void reset_cache(CACHE *cache);
void InitialMainMemory(FILE* memin);

void GetDataFromCache(CACHE* cache, int address, int* data);

int LoadWord(int address, int* data, CACHE* cache, int core_index);
void BusRd(int core_index, int address);
int GetDataFromMainMemory( int address);
void Flush(int address, int data);
void UpdateCacheBlock(CACHE* cache);

#endif 

