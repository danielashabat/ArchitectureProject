#ifndef	MEMORY_H
#define	MEMORY_H


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


typedef enum { NO_COMMAND = 0, BUSRD = 1, BUSRDX = 2, FLUSH = 3 } bus_commands;

/*********************STRUCTS*****************/

typedef struct Cache {
	int DSRAM[CHACHE_SIZE];
	int TSRAM[CHACHE_SIZE];
}CACHE;

typedef struct {
	short bus_origid;//3 bits
	short bus_cmd;//2 bit
	int bus_addr;//20 bits
	int bus_data;//32 bits
	int bus_mode;// if 1: BUSY from previos command ,if 0:the bus is FREE
	int timer;// when bus is busy the timer count the numer of cycles it busy
}Bus_Reg;

/*********************GLOBAL VARS	*****************/


/*********************FUNCTIONS DECLARATION	*****************/
int address_in_cache(int address, CACHE* cache, int *mode);
void reset_cache(CACHE *cache);
int GetDataFromCache(CACHE* cache, int address, int* data);
int GetDataFromCacheExclusive(CACHE* cache, int address, int* data);
void UpdateCacheBlock(CACHE* cache, int new_mode);

void sample_bus();
void update_bus();
void ReadBusLines(int* bus_origid, int* bus_cmd, int* bus_addr, int* bus_data);
void InitialBus();
void BusRd(int core_index, int address);
void BusRdX(int core_index, int address);
void Flush(int address, int data);

void InitialMainMemory(FILE* memin);



#endif 

