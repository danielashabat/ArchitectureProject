#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>

#include "memory.h"

int MainMemory[MainMemorySize] = { 0 };
Bus_Reg bus_reg_old;
Bus_Reg bus_reg_new;


/*********************BUS FUNCTIONS*****************/
void sample_bus(){
	update_bus();
	bus_reg_old.bus_origid=bus_reg_new.bus_origid;
	bus_reg_old.bus_cmd=bus_reg_new.bus_cmd;
	bus_reg_old.bus_addr=bus_reg_new.bus_addr;
	bus_reg_old.bus_data=bus_reg_new.bus_data;
	bus_reg_old.bus_mode=bus_reg_new.bus_mode;
	bus_reg_old.timer = bus_reg_new.timer;
}

void update_bus() {
	//check for new request
	if (bus_reg_old.bus_cmd == BUSRD || bus_reg_old.bus_cmd == BUSRDX) {
		printf("-BUS request- command:%d, address: 0x%08x from core: %d\n", bus_reg_old.bus_cmd, bus_reg_old.bus_addr, bus_reg_old.bus_origid);
		bus_reg_new.bus_mode = 1;//set bus mode to busy
		bus_reg_new.timer = 0;
		bus_reg_new.bus_cmd = 0;
		return;
	}

	if (bus_reg_old.bus_mode == 1) {
		if (bus_reg_old.timer == 64) {//bus finish readimg from memory after 64 cycles
			Flush(bus_reg_old.bus_addr, MainMemory[bus_reg_old.bus_addr]);
			printf("-FLUSH - data :%d, address: %d \n", MainMemory[bus_reg_old.bus_addr], bus_reg_old.bus_addr);
			bus_reg_new.bus_mode = 0;//set bus mode to free
		}
		else {
			bus_reg_new.timer++;//+1 timer
			bus_reg_new.bus_mode = 1;
			bus_reg_new.bus_addr = bus_reg_old.bus_addr;
		}
	}
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

void Flush(int address, int data) {
	//update bus
	bus_reg_new.bus_origid = 4;//main memory 
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

/*********************MAIN MEMORY FUNCTIONS*****************/


void InitialMainMemory(FILE* memin) {
	int i = 0;
	while (!feof(memin) && (i < MainMemorySize)) {
		fscanf(memin, "%x\r\n", MainMemory + i);
		i++;
	}
}

/*********************CACHE FUNCTIONS*****************/
/*cache: the cache is in size of 256 rows,
pysical address is 20 bits long. we divide the bits in the following way: [19:8]represent the tag block, [7:0] represent the index of block 
TSRAM: each row has 32 bits : [13:12]is MSI bits (state of address), [11:0] is Tag bits . (rest of the bits are zero)
DSRAM:each row is 32 bits long, represent the data of the address from the main memory(the number of line is the address) 

*/

//update cache block by reading te bus lines
void UpdateCacheBlock(CACHE* cache , int new_mode) {
	//get data from the bus
	if (bus_reg_old.bus_origid != 4) {
		printf("ERROR:the data is not from bus!");
		return;
	}
	int data = bus_reg_old.bus_data;
	int address = bus_reg_old.bus_addr;

	int index = address % CHACHE_SIZE;// index is the the first 8 bits in address
	int tag = address / CHACHE_SIZE;//get the tag of the address

	//update TSRAM
	cache->TSRAM[index] = (new_mode << 12)| (tag);

	//update DSRAM
	cache->DSRAM[index] = data;

	printf("cache update in index: %d, new row in TSRAM: %08x,new row in DSRAM: %08x \n", index, cache->TSRAM[index], cache->DSRAM[index]);
}

/*return 1 if cache hit, otherwise return*/
int GetDataFromCache(CACHE* cache, int address, int* data) {
	int mode;
	if (address_in_cache(address,cache,&mode)) {
		int index = address % CHACHE_SIZE;// index is the the first 8 bits in address
		*data = cache->DSRAM[index];//assign to data pointer to the data taken from cache 
		printf("read hit!address:0x%08x data: 0x%08x\n", address, *data);
		return 1;//cache hit
	}
	printf("read miss! address: 0x%08x\n", address);
	return 0;//cache miss
}

//check if data in cache in MODIFIED mode
int GetDataFromCacheExclusive(CACHE* cache, int address, int* data) {
	int mode;
	if (address_in_cache(address, cache, &mode)) {
		if (mode == MODIFIED) {
			int index = address % CHACHE_SIZE;// index is the the first 8 bits in address
			*data = cache->DSRAM[index];//assign to data pointer to the data taken from cache 
			printf("write hit!address:0x%08x data: 0x%08x\n", address, *data);
			return 1;//cache hit
		}
	}
	printf("write miss! address: 0x%08x\n", address);
	return 0;//cache miss
}

int address_in_cache(int address, CACHE *cache , int *mode) {
	int index = address % CHACHE_SIZE;// index is the the first 8 bits in address
	int tag = address/CHACHE_SIZE;//get the tag of the address

	int tag_in_cache = (cache->TSRAM[index]) % TAG_BITS;//get the Tag bits (12 first bits) from cache in the same index  
	*mode = (cache->TSRAM[index]) / TAG_BITS;// get the state of address

	//debug message
	//printf("the tag in cache is %x, and the tag of the address is: %x, the state is %x", tag_in_cache, tag,state_of_address);

	if ((tag == tag_in_cache) && *mode != INVALID) {//compare the tags and verify the state is in shared or modified mode
		return 1; //address in cache
	}
	return 0; //address not in cache
}

void reset_cache(CACHE *cache) {
	for (int i = 0; i < CHACHE_SIZE; i++)
	{
		cache->TSRAM[i] = 0;
		cache->DSRAM[i] = 0;
	}
}



