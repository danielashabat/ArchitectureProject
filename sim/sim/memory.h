#ifndef	MEMORY_H
#define	MEMORY_H


//#include "simulator.h"
/*this module includes all the functions and structs that belong to the main memory and the cache*/


/*********************MACROS*****************/
//sizes
#define MainMemorySize 1048576//equal to 2^20
#define CHACHE_SIZE 256

#define TAG_BITS(TSRAM_ROW)  (TSRAM_ROW&0xFFF) //return 12 first bits
#define MSI_BITS(TSRAM_ROW)   (TSRAM_ROW>>12) //return TSRAM_ROW[13:12]  bits

//block modes in cache
#define INVALID 0
#define SHARED 1	
#define MODIFIED 2

#define PC_MASK(PC) 0X3FF&PC



typedef enum { NO_COMMAND = 0, BUSRD = 1, BUSRDX = 2, FLUSH = 3 } bus_commands;

/*********************STRUCTS*****************/

typedef struct Cache {
	unsigned int DSRAM[CHACHE_SIZE];
	unsigned int TSRAM[CHACHE_SIZE];
	int id;
}CACHE;

typedef struct {
	short bus_origid;//3 bits
	short bus_cmd;//2 bit
	int bus_addr;//20 bits
	int bus_data;//32 bits
	int bus_mode;// if 1: BUSY from previos command ,if 0:the bus is FREE
	int timer;// when bus is busy the timer count the numer of cycles it busy
}Bus_Reg;




/*********************FUNCTIONS DECLARATION	*****************/

//check if address in cache
int address_in_cache(int address, CACHE* cache, int *mode);
void reset_cache(CACHE* cache, int id);

/*return 1 if the data in cache in SHARED mode , otherwise return*/
int GetDataFromCache(CACHE* cache, int address, int* data);

//check if data in cache in MODIFIED mode
//return 1 if the data in cache in M mode, otherewise return 0
int GetDataFromCacheExclusive(CACHE* cache, int address, int* data);
void UpdateCache(CACHE* cache, int address, int data, int new_mode);

/*this function is used in StoreWord for writing a new data to a specific address
this functon assume the relevant block's address is already in cache in MODIFIED state.(TSRAM need to be checked previous using this function))*/
void WriteToCache(CACHE* cache, int address, int data);
void print_dsram_and_tsram(FILE* dsram, FILE* tsram, CACHE* cache);

void InitialMainMemory(FILE* memin);
void print_memout(FILE* memout);
void write_block_to_main_memory(CACHE* cache, int index);

/*commands for SC and LL functions */
void read_watch_bit(int* bit, int* origid);
void set_watch_bit(int origid);
void unset_watch_bit();
/**/

/*input: cycle- the cycle number, bustrace_file - pointer to bus trace_file.
Functionality: this function read the bus lines and check for a new command from the bus.
If there is new command. The main memory starts a new transaction on the bus lines according to the accepted command.
The result of the transaction appears after 64 cycles in flush command.in addition, it updates the bustrace_file.*/
void update_main_memory(int cycle, FILE* bustrace_file);

/*bus functions:**********/
int bus_is_busy_in_next_cycle();
void ReadBusLines(int* bus_origid, int* bus_cmd, int* bus_addr, int* bus_data);
void InitialBus();

/*sample the bus lines*/
void sample_bus();

/*Bus command functions- update the bus registers according the type of command*/
void BusRd(int core_index, int address);
void BusRdX(int core_index, int address);
void Flush(int address, int data, int bus_origid);
/*Bus command functions end*/

#endif 

