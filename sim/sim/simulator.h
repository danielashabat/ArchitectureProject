#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

/*********************MACROS*****************/
#ifndef SIMULATOR_H
#define	SIMULATOR_H

#define PC_MASK(PC) 0X3FF&PC //PC have only 10 bits

/*********************STRUCTS*****************/

typedef struct Core {
	int Reg[16] = {0};
	int PC = 0;//need to be 10 bits long 
	FILE *IMEM=NULL ;
	CACHE  cache;
}CORE;

#endif 
#pragma once