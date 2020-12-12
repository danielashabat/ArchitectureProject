#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "simulator.h"

int MainMemory[MainMemorySize] = {0 };

short bus_origid = 0;//3 bits
short bus_cmd = 0;//2 bit
int bus_addr = 0;//20 bits
int bus_data = 0;//32 bits


void InitialMainMemory(FILE	*memin) {
	int i = 0;
	while (!feof(memin) && (i< MainMemorySize)) {
		fscanf(memin, "%x\r\n", MainMemory + i);
			i++;
	}
}

void BusRd(int core_index, int address) {
	//update bus
	bus_origid = core_index;
	bus_cmd = BUSRD;
	bus_addr = address;
	bus_data = 0;
}


void Flush(int address, int data) {
	//update bus
	bus_origid = 4;//main memory 
	bus_cmd = FLUSH;
	bus_addr = address;
	bus_data = data;
}


//return 1 if the function finished, otherwise 0
int GetDataFromMainMemory(int address) {
	static timer = 0;
	if (timer < 64) {
		timer++;
		return 0;
	}
	else{
		int data = MainMemory[address];
		Flush(address, data);
		timer = 0;//for another requests
		return 1;
	}
}


// the function return 1 if it finished it's command,
//return 0 if not finished (need more cycles) and need to stall.
int LoadWord(int address, int* data,CACHE *cache, int core_index) {
	static timer = 0;
	
		if (address_in_cache(address, cache)) {
			GetDataFromCache(cache, address, data);//get the data from the cache
			printf("read hit!,address:%x data: %x\n",address, *data);
			return 1;
		}

		//if address not on the cache, send a BusRd request on the bus
	if (bus_cmd == NO_COMMAND) {//check if bus is free
		BusRd(core_index, address);
		printf("ReadMiss!\n");
		GetDataFromMainMemory(address);
		return 0;
	}
	if ((bus_cmd != NO_COMMAND)  && (bus_origid == core_index)) {// if bus is busy and it the same core that sent the request
			if (GetDataFromMainMemory(address)) {//check if the function finished
				*data = bus_data;//get the data from bus
				UpdateCacheBlock(cache);
				printf("bus finished read command,address:%x ,data:%x\n", bus_addr, bus_data);
				return 1;
			}
			else 
				return 0;//if not finished
		}
	
	else // the bus is busy from another core
		return 0;//need to wait the bus will be free
}





/*********************CACHE FUNCTIONS*****************/
/*cache: the cache is in size of 256 rows,
pysical address is 20 bits long. we divide the bits in the following way: [19:8]represent the tag block, [7:0] represent the index of block 
TSRAM: each row has 32 bits : [13:12]is MSI bits (state of address), [11:0] is Tag bits . (rest of the bits are zero)
DSRAM:each row is 32 bits long, represent the data of the address from the main memory(the number of line is the address) 

*/

void UpdateCacheBlock(CACHE* cache) {
	//get data from the bus
	if (bus_origid != 4) {
		printf("ERROR:the data is not from bus!");
		return;
	}
	int data = bus_data;
	int address = bus_addr;

	int index = address % CHACHE_SIZE;// index is the the first 8 bits in address
	int tag = address / CHACHE_SIZE;//get the tag of the address
	int mode = SHARED;

	//update TSRAM
	cache->TSRAM[index] = (mode << 12)| (tag);

	//update DSRAM
	cache->DSRAM[index] = data;

	printf("cache update in index: %d, TSRAM: %.4x, DSRAM: %x \n", index, cache->TSRAM[index], cache->DSRAM[index]);
}


void GetDataFromCache(CACHE* cache, int address, int* data) {
	int index = address % CHACHE_SIZE;// index is the the first 8 bits in address
	*data = cache->DSRAM[index];//assign to data pointer to the data taken from cache 
	return;
}

int address_in_cache(int address, CACHE *cache ) {
	int index = address % CHACHE_SIZE;// index is the the first 8 bits in address
	int tag = address/CHACHE_SIZE;//get the tag of the address

	int tag_in_cache = (cache->TSRAM[index]) % TAG_BITS;//get the Tag bits (12 first bits) from cache in the same index  
	int state_of_address = (cache->TSRAM[index]) / TAG_BITS;// get the state of address

	//debug message
	//printf("the tag in cache is %x, and the tag of the address is: %x, the state is %x", tag_in_cache, tag,state_of_address);

	if ((tag == tag_in_cache) && state_of_address != INVALID) {//compare the tags and verify the state is in shared or modified mode
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



