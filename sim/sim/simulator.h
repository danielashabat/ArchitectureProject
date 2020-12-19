#ifndef	SIMULATOR_H
#define	SIMULATOR_H


#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "main.h"


/*********************MACROS*****************/
typedef enum { WAITING = 0, DONE = 1, CACHE_MISS = 2 } MEM_STATUS;


/*********************STRUCTS*****************/
typedef struct Core {
	reg reg_old;
	reg reg_new;
	FILE* IMEM;
	CACHE  cache;
	int id;//core index {0-3}
}CORE;

/*********************FUNCTIONS DECLARATION	*****************/

/*this function initial a new core. it reset it's cache and set the Id core*/
void InitialCore(CORE* core,//need to be a valid pointer to core 
				int id_core);// a unique id that going to be set to the core

/*********************MEM FUNCTIONS*****************/

/*LoadWord function is for the memory part in the pipe, this function get an address the core want to read and than assign it 
to data pointer.
LoadWord can take more than one cycle. depend if the address is in the cache or in the main memory.
LoadWord return the status of the function after one cycle(one run of the command), the possible return values:
0- the function is in WAITING status.namely, the function wait for the data from the main memory and need more cycles to finish.
1- the function is DONE. no need for more cycles.
2- the function had CACHE MISS in this cycle. so it's need more cycles to finish*/
int LoadWord(int address,//the address you want to read 
			int* data,//pointer to int. when LoadWord is DONE, this pointer contain the data from the given address
			CORE* core,//pointer to the core that running this proccess
			int prev_status);//the previos status from the last run of LoadWord


/*StoreWord function is for the memory part in the pipe, this function get an address the core want to write and the data for writing.
StoreWord can take more than one cycle. depend if the address is in the cache or in the main memory.
StoreWord return the status of the function after one cycle(one run of the command), the possible return values:
0- the function is in WAITING status.namely, the function wait for the data from the main memory and need more cycles to finish.
1- the function is DONE. no need for more cycles.
2- the function had CACHE MISS in this cycle. so it's need more cycles to finish*/
int StoreWord(int address,//the address you want to read 
	int new_data,//the new data for writing in the given address
	CORE* core, //pointer to the core that running this proccess
	int prev_status);//the previos status from the last run of LoadWord


#endif 
#pragma once