#ifndef	SIMULATOR_H
#define	SIMULATOR_H


#include <stdio.h>
#include <stdlib.h>
#include "memory.h"



/*********************MACROS*****************/
typedef enum { WAITING = 0, DONE = 1, CACHE_MISS = 2 , CONFLICT_MISS=3} MEM_STATUS;
#define CORE_NUM 4//number of cores to run

/*********************STRUCTS DECLARATION	*****************/

typedef struct Core {
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
LoadWord return the status of the function after one cycle(one run of the command).
the possible return values:
0- the function is in WAITING status.namely, the function wait for the data from the main memory and need more cycles to finish.
1- the function is DONE. no need for more cycles.
2- the function had CACHE MISS in this cycle. so it's need more cycles to finish
3- the function had CONFLICT MISS in this cycle. so it's need more cycles to finish
*/
int LoadWord(int address,//the address you want to read 
			int* data,//pointer to int. when LoadWord is DONE, this pointer contain the data from the given address
			CORE* core,//pointer to the core that running this proccess
			int prev_status);//the previos status from the last run of LoadWord


/*StoreWord function is for the memory part in the pipe, this function get an address the core want to write and the data for writing.
StoreWord can take more than one cycle. depend if the address is in the cache or in the main memory.
StoreWord return the status of the function after one cycle(one run of the command), the possible return values:
0- the function is in WAITING status.namely, the function wait for the data from the main memory and need more cycles to finish.
1- the function is DONE. no need for more cycles.
2- the function had CACHE MISS in this cycle. so it's need more cycles to finish
3- the function had CONFLICT MISS in this cycle. so it's need more cycles to finish*/
int StoreWord(int address,//the address you want to read 
	int new_data,//the new data for writing in the given address
	CORE* core, //pointer to the core that running this proccess
	int prev_status);//the previos status from the last run of LoadWord

/*if the core see BusRdx/BusRd from another core. and have the most updated data of the requested address in it's cache
in MODIFIED mode, than abort the requset and flush the data to the core that sent the request
in addition, if the core see BusRdx request from another core of an address that in his cache in SHARE mode don't help the other core and invalidate the block
this function need to be for every core and need to be update every cycle*/
int Snooping(CORE* core);

/*LoadLinked call LoadWord and set watch flag to 1 */
int LoadLinked(int address, int* data, CORE* core, int prev_status, int* watch_flag);

//StoreConditinal checks if others cores used SC before by looking on watch flag status.
// if not, it calls StoreWord and set new_data to 1. otherwise it set new_data to 0.
int StoreConditional(int address, int* new_data, CORE* core, int prev_status, int* watch_flag, int* sc_status);
void update_watch_flag(int* watch_flag, CORE* core);

void get_hits_and_miss(int core_index, int* read_hit_, int* write_hit_, int* read_miss_, int* write_miss_);
void print_dsram_and_tsram_wrapper(FILE* dsram, FILE* tsram, CORE* core);
#endif 
#pragma once