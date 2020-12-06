
#ifndef	MEMORY_H
#define	MEMORY_H
/*this module includes all the functions and structs that belong to the main memory and the cache*/


/*********************STRUCTS*****************/

typedef struct Cache {
	int DSRAM[256] = { 0 };
	int TSRAM[256] = { 0 };
}CACHE;



#endif 
#pragma once
