#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "simulator.h"

int MainMemory[MainMemorySize] = { 0 };

int BusRd(CORE core, int address, int *data) {


	//first, check if the address exist in the cache
	GetDataFromCache(core, address, data);

	//if not in cache, go to the main memory
	GetDataFromMainMemory(core, address,data);

}

/*if succeed return 1, otherwise return 0*/
int GetDataFromCache(CORE core, int address, int* data) {}

int GetDataFromMainMemory(CORE core, int address, int* data) {
	*data = MainMemory[address];
	return 0;
}