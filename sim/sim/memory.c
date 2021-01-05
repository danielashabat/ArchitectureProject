#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>

#include "memory.h"

static int MainMemory[MainMemorySize] = { 0 };
static Bus_Reg bus_reg_old;
static Bus_Reg bus_reg_new;

static int watch_bit = 0;
static int watch_origid = 0;// who sent the watch_bit

/*********************BUS FUNCTIONS*****************/
void sample_bus(){
	bus_reg_old.bus_origid=bus_reg_new.bus_origid;
	bus_reg_old.bus_cmd=bus_reg_new.bus_cmd;
	bus_reg_old.bus_addr=bus_reg_new.bus_addr;
	bus_reg_old.bus_data=bus_reg_new.bus_data;
	bus_reg_old.bus_mode=bus_reg_new.bus_mode;
	bus_reg_old.timer = bus_reg_new.timer;
}




int bus_is_busy_in_next_cycle() {
	return (bus_reg_new.bus_cmd || bus_reg_new.bus_mode);
}

void ReadBusLines(int * bus_origid,int * bus_cmd, int *bus_addr, int *bus_data){
	*bus_origid = bus_reg_old.bus_origid;
	*bus_cmd = bus_reg_old.bus_cmd;
	*bus_addr = bus_reg_old.bus_addr;
	*bus_data = bus_reg_old.bus_data;
}


void BusRd(int core_index, int address) {
	//update bus
	bus_reg_new.bus_origid = core_index;
	bus_reg_new.bus_cmd = BUSRD;
	bus_reg_new.bus_addr = address;
	bus_reg_new.bus_data = 0;
}

void BusRdX(int core_index, int address) {
	//update bus
	bus_reg_new.bus_origid = core_index;
	bus_reg_new.bus_cmd = BUSRDX;
	bus_reg_new.bus_addr = address;
	bus_reg_new.bus_data = 0;
}

void Flush(int address, int data, int bus_origid) {
	//update bus
	bus_reg_new.bus_origid = bus_origid; 
	bus_reg_new.bus_cmd = FLUSH;
	bus_reg_new.bus_addr = address;
	bus_reg_new.bus_data = data;
}

void InitialBus() {
	bus_reg_new.bus_origid = 0;//3 bits
	bus_reg_new.bus_cmd = 0;//2 bit
	bus_reg_new.bus_addr = 0;//20 bits
	bus_reg_new.bus_data = 0;//32 bits
	bus_reg_new.bus_mode = 0;
	bus_reg_new.timer = 0;

	bus_reg_old.bus_origid = 0;//3 bits
	bus_reg_old.bus_cmd = 0;//2 bit
	bus_reg_old.bus_addr = 0;//20 bits
	bus_reg_old.bus_data = 0;//32 bits
	bus_reg_old.bus_mode = 0;
	bus_reg_old.timer = 0;
}


void read_watch_bit(int *bit,int *origid) {
	*bit= watch_bit;
	*origid = watch_origid;
}

void set_watch_bit(int origid) {
	printf("setting watch bit to 1 from core: %d!\n",origid);
	watch_bit = 1;
	watch_origid = origid;
}

void unset_watch_bit() {
	watch_bit = 0;
	printf("setting watch bit to 0!\n");
}


/*********************MAIN MEMORY FUNCTIONS*****************/


void InitialMainMemory(FILE* memin) {
	int i = 0;
	while (!feof(memin) && (i < MainMemorySize)) {
		fscanf(memin, "%x\r\n", MainMemory + i);
		i++;
	}
}

void print_memout(FILE* memout) {
	for (size_t i = 0; i < MainMemorySize; i++)
	{
		fprintf(memout, "%08x \n", MainMemory[i]);
	}
}

void write_block_to_main_memory(CACHE* cache, int index) {
	int data = cache->DSRAM[index];
	int tag_bits = TAG_BITS(cache->TSRAM[index]);
	int address = tag_bits << 12 | index;
	Flush(address, data, cache->id);
}

void update_main_memory(int cycle, FILE *bustrace_file) {
	//check for new request
	if (bus_reg_old.bus_cmd == BUSRD || bus_reg_old.bus_cmd == BUSRDX) {
		printf("-BUS request in cycle %d - command:%d, address: 0x%08x from core: %d\n", cycle, bus_reg_old.bus_cmd, bus_reg_old.bus_addr, bus_reg_old.bus_origid);
		fprintf(bustrace_file, "%d %d %d %05x %08x \n", cycle, bus_reg_old.bus_origid, bus_reg_old.bus_cmd, bus_reg_old.bus_addr, bus_reg_old.bus_data);
		bus_reg_new.bus_mode = 1;//set bus mode to busy
		bus_reg_new.timer = 1;
		bus_reg_new.bus_cmd = 0;
		return;
	}
	//check for flush request from one of the cores-> need to update main memory
	if (bus_reg_old.bus_cmd == FLUSH && (bus_reg_old.bus_origid != 4)) {
		printf("-FLUSH request  in cycle %d - data: 0x%08x, address: 0x%08x from core: %d\n", cycle, bus_reg_old.bus_data, bus_reg_old.bus_addr, bus_reg_old.bus_origid);
		fprintf(bustrace_file,"%d %d %d %05x %08x \n", cycle, bus_reg_old.bus_origid, bus_reg_old.bus_cmd, bus_reg_old.bus_addr, bus_reg_old.bus_data);
		bus_reg_new.bus_mode = 2;//set bus mode to busy
		bus_reg_new.timer = 0;
		bus_reg_new.bus_cmd = 0;
		return;
	}

	//handle with busRd/BuesRdX/FLUSH
	if (bus_reg_old.bus_mode != 0) {
		if (bus_reg_old.timer == 63) {//bus finish reading from memory after 64 cycles (decrease -1 because flops delay)
			if (bus_reg_old.bus_mode == 1) {//only for busrd/busrsx requests
				Flush(bus_reg_old.bus_addr, MainMemory[bus_reg_old.bus_addr], 4);
			}
			else {
				MainMemory[bus_reg_old.bus_addr] = bus_reg_old.bus_data;//update the main memory from bus lines
				printf("finish updating the main memory in cycle: %d!\n", cycle); 
			}//finsih update the main memory 
			bus_reg_new.bus_mode = 0;//set bus mode to free
		}
		else {
			bus_reg_new.timer++;//+1 timer
			bus_reg_new.bus_mode = bus_reg_old.bus_mode;
			bus_reg_new.bus_addr = bus_reg_old.bus_addr;
			bus_reg_new.bus_data = bus_reg_old.bus_data;
		}
		return;
	}
	if ((bus_reg_old.bus_cmd == FLUSH) && (bus_reg_old.bus_origid == 4)) {//if flush from bus in the last cycle
		bus_reg_new.bus_mode = 0;//set bus mode to free
		bus_reg_new.bus_cmd = 0;
		printf("-FLUSH  in cycle %d - data: 0x%08x, address: 0x%08x \n", cycle, bus_reg_old.bus_data, bus_reg_old.bus_addr);
		fprintf(bustrace_file,"%d %d %d %05x %08x \n", cycle, bus_reg_old.bus_origid, bus_reg_old.bus_cmd, bus_reg_old.bus_addr, bus_reg_old.bus_data);

	}
}

/*********************CACHE FUNCTIONS*****************/
/*cache: the cache is in size of 256 rows (TSRAM and DSRAM),
pysical address is 20 bits long. we divide the bits in the following way: [19:8]represent the tag block, [7:0] represent the index of block 
TSRAM: each row has 32 bits : [13:12]is MSI bits (state of address), [11:0] is Tag bits . (rest of the bits are zero)
DSRAM:each row is 32 bits long, represent the data of the address from the main memory(the number of line is the index of the address) 

*/

void print_dsram_and_tsram(FILE* dsram,FILE *tsram, CACHE* cache) {
	for (int i = 0; i < CHACHE_SIZE; i++)
	{
		fprintf(dsram, "%08x \n", cache->DSRAM[i]);
		fprintf(tsram, "%08x \n", cache->TSRAM[i]);
	}
}

//update cache block in the given address and data in 
void UpdateCache(CACHE* cache ,int address,int data, int new_mode) {

	int index = address % CHACHE_SIZE;// index is the the first 8 bits in address
	int tag = address / CHACHE_SIZE;//get the tag of the address

	//update TSRAM
	cache->TSRAM[index] = (new_mode << 12)| (tag);

	//update DSRAM
	cache->DSRAM[index] = data;

	printf("cache update from bus lines in index: %d, new row in TSRAM: %08x,new row in DSRAM: %08x \n", index, cache->TSRAM[index], cache->DSRAM[index]);
}

/*return 1 if cache hit, otherwise return*/
int GetDataFromCache(CACHE* cache, int address, int* data) {
	int mode;
	if (address_in_cache(address,cache,&mode)) {
		int index = address % CHACHE_SIZE;// index is the the first 8 bits in address
		*data = cache->DSRAM[index];//assign to data pointer to the data taken from cache 
		//printf("read hit!address:0x%08x data: 0x%08x\n", address, *data);
		return 1;//cache hit
	}
	printf("read miss! address: 0x%08x\n", address);
	return 0;//cache miss
}

//check if data in cache in MODIFIED mode
//return 1 if the data in cache in M mode, otherewise return 0
int GetDataFromCacheExclusive(CACHE* cache, int address, int* data) {
	int mode;
	if (address_in_cache(address, cache, &mode)) {
		if (mode == MODIFIED) {
			int index = address % CHACHE_SIZE;// index is the the first 8 bits in address
			*data = cache->DSRAM[index];//assign to data pointer to the data taken from cache 
			//printf("write hit!address:0x%08x data: 0x%08x\n", address, *data);
			return 1;//cache hit
		}
	}
	//printf("write miss! address: 0x%08x\n", address);
	return 0;//cache miss
}

int address_in_cache(int address, CACHE *cache , int *mode) {
	int index = address % CHACHE_SIZE;// index is the the first 8 bits in address
	int tag = address/CHACHE_SIZE;//get the tag of the address

	int tag_in_cache = TAG_BITS(cache->TSRAM[index]);//get the Tag bits (12 first bits) from cache in the same index  
	*mode = MSI_BITS(cache->TSRAM[index]);// get the state of address

	//debug message
	//printf("the tag in cache is %x, and the tag of the address is: %x, the state is %x", tag_in_cache, tag,state_of_address);

	if ((tag == tag_in_cache) && *mode != INVALID) {//compare the tags and verify the state is in shared or modified mode
		return 1; //address in cache
	}
	return 0; //address not in cache
}

void reset_cache(CACHE *cache, int id) {
	for (int i = 0; i < CHACHE_SIZE; i++)
	{
		cache->TSRAM[i] = (unsigned int) 0;
		cache->DSRAM[i] = (unsigned int)0;
	}
	cache->id = id;
}

/*this function is used in StoreWord for writing a new data to a specific address
this functon assume the relevant block's address is already in cache in MODIFIED state.(TSRAM need to be checked previous using this function))*/
void WriteToCache(CACHE *cache, int address, int data) {

	int index = address % CHACHE_SIZE;// index is the the first 8 bits in address
	int tag = address / CHACHE_SIZE;//get the tag of the address
	int tag_in_cache = TAG_BITS(cache->TSRAM[index]);
	int mode_in_cache = MSI_BITS(cache->TSRAM[index]);
	//check tag and mode in TSRAM
	if ((tag_in_cache != tag) || (mode_in_cache != MODIFIED))
	{
		printf("ERROR: can't write to cache! block is not in cache or not in 'M' mode \n");
		return;
	}

		//update DSRAM
	cache->DSRAM[index] = data;
	//printf("write cache successed to block in index: %d,new row in DSRAM: %08x \n", index, cache->DSRAM[index]);
	return;
}

