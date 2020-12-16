
#include "simulator.h"




/*********************SIMULATOR FUNCTIONS*****************/



// the function return the new status of the memory  { WAITING = 0, DONE = 1, CACHE_MISS = 2 }
int LoadWord(int address, int* data,CORE *core, int prev_status) {
	CACHE *cache= &(core->cache);
	int ret_val;
	int bus_origid, bus_cmd, bus_addr, bus_data;
	switch (prev_status)
	{
	case DONE://the pervios command done -> the memory is free
		ret_val = GetDataFromCache(cache, address, data);//get the data from the cache
		if (ret_val == 1) return DONE; else return CACHE_MISS;
		break;

	case CACHE_MISS:
		//if address not on the cache, send a BusRd request on the bus
		ReadBusLines(&bus_origid, &bus_cmd, &bus_addr, &bus_data);
		if (bus_cmd == NO_COMMAND) {//check if bus is free
			BusRd(core->index, address);//update bus 
			return WAITING;
		}
		else 
			printf("ERROR:try to send bus reqeust but bus is busy!\n"); return WAITING;//need to handle the case the bus busy from another core
		break;

	case WAITING:
		ReadBusLines(&bus_origid, &bus_cmd, &bus_addr, &bus_data);
		if (bus_cmd == FLUSH) {
		*data = bus_data;
		UpdateCacheBlock(cache,SHARED);//update in cache in SHARE mode
		return DONE; }//reading from main memory complete!
		else return WAITING; //still need to wait
		break;
	}

}

// the function return the new status of the memory  { WAITING = 0, DONE = 1, CACHE_MISS = 2 }
int StoreWord(int address, int* data, CORE* core, int prev_status) {
	CACHE* cache = &(core->cache);
	int ret_val;
	int bus_origid, bus_cmd, bus_addr, bus_data;
	switch (prev_status)
	{
	case DONE:
		ret_val = GetDataFromCacheExclusive(cache, address, data);
		if (ret_val == 1) { 
			//write to cache
			return DONE; }
		else return CACHE_MISS;
		break;


	case CACHE_MISS:
		//if address not on the cache, send a BusRdX request on the bus
		ReadBusLines(&bus_origid, &bus_cmd, &bus_addr, &bus_data);
		if (bus_cmd == NO_COMMAND) {//check if bus is free
			BusRdX(core->index, address);//update bus 
			return WAITING;
		}
		else
			printf("ERROR:try to send bus reqeust but bus is busy!\n"); return WAITING;//need to handle the case the bus busy from another core
		break;


	case WAITING:
		ReadBusLines(&bus_origid, &bus_cmd, &bus_addr, &bus_data);
		if (bus_cmd == FLUSH) {
			*data = bus_data;
			UpdateCacheBlock(cache,MODIFIED);//update in cache in MODIFIED mode
			//write to cache
			return DONE;
		}//writing  complete!
		else return WAITING; //still need to wait
		break;
	}

}


void InitialCore(CORE *core,int core_index) {
	core->index = core_index;
	reset_cache(&core->cache);
}