
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
			BusRd(core->id, address);//update bus 
			return WAITING;
		}
		else 
			printf("ERROR:try to send bus reqeust but bus is busy!\n"); return WAITING;//need to handle the case the bus busy from another core
		break;

	case WAITING:
		ReadBusLines(&bus_origid, &bus_cmd, &bus_addr, &bus_data);
		if (bus_cmd == FLUSH) {
		*data = bus_data;
		UpdateCacheFromBus(cache,SHARED);//update in cache in SHARE mode
		return DONE; }//reading from main memory complete!
		else return WAITING; //still need to wait
		break;
	}

}

// the function return the new status of the memory  { WAITING = 0, DONE = 1, CACHE_MISS = 2 }
int StoreWord(int address, int new_data, CORE* core, int prev_status) {
	CACHE* cache = &(core->cache);
	int ret_val;
	int bus_origid, bus_cmd, bus_addr, bus_data;
	switch (prev_status)
	{
	case DONE:
		ret_val = GetDataFromCacheExclusive(cache, address, new_data);
		if (ret_val == 1) { //if the address in cache in MODIFIED mode. write to cache
			WriteToCache(cache, address, new_data);
			return DONE; }
		else return CACHE_MISS;
		break;


	case CACHE_MISS:
		//if address not on the cache, send a BusRdX request on the bus
		ReadBusLines(&bus_origid, &bus_cmd, &bus_addr, &bus_data);
		if (bus_cmd == NO_COMMAND) {//check if bus is free
			BusRdX(core->id, address);//update bus 
			return WAITING;
		}
		else
			printf("ERROR:try to send bus reqeust but bus is busy!\n"); return WAITING;//need to handle the case the bus busy from another core
		break;


	case WAITING:
		ReadBusLines(&bus_origid, &bus_cmd, &bus_addr, &bus_data);
		if (bus_cmd == FLUSH) {
			UpdateCacheFromBus(cache,MODIFIED);//update in cache in MODIFIED mode
			WriteToCache(cache, address, new_data);
			return DONE;
		}//writing  complete!
		else return WAITING; //still need to wait
		break;
	}

}


void InitialCore(CORE *core,int id_core) {
	core->id = id_core;
	reset_cache(&core->cache, id_core);
}

/*if the core see BusRd/BusRdx from another core. and have the most updated data of the requested address in it's cache
in MODIFIED mode, than abort the requset and flush the data the the core that sent the request
return 1 if it updated anoter core's cache, otherwise return 0*/
int UpdateOtherCore(CORE *core) {
	int bus_origid, bus_cmd, bus_addr, bus_data;
	int data;
	CACHE* cache = &(core->cache);
	ReadBusLines(&bus_origid, &bus_cmd, &bus_addr, &bus_data);
	if (bus_cmd == FLUSH) return 0;//if it's not BusRd/BusRdx request, end function

	//if BusRd/BusRdx request
	if (GetDataFromCacheExclusive(cache, bus_addr, &data)) {//if the most update data is in this cache in 'M' mode than Flush
		Flush(bus_addr, data, core->id);
		abort_bus();
		printf("core num: %d aborted bus and FLUSH to core:%d. address:0x%08x, data:0x%08x\n", core->id, bus_origid, bus_addr, data);
		return 1;
	}
	return 0;
}