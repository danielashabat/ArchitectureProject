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

int LoadWord(int address, int* data, CORE* core, int prev_status);
int StoreWord(int address, int new_data, CORE* core, int prev_status);

void InitialCore(CORE* core, int id_core);
#endif 
#pragma once