
#include "simulator.h"




/*********************SIMULATOR FUNCTIONS*****************/



// the function return the new status of the memory 
int LoadWord(int address, int* data,CORE *core, int prev_status) {
	CACHE *cache= &(core->cache);
	int ret_val;
	int bus_origid, bus_cmd, bus_addr, bus_data;
	switch (prev_status)
	{
	case DONE://the pervios command done -> the memory is free
		ret_val = GetDataFromCache(cache, address, data);//get the data from the cache
		if (ret_val == 1) return DONE; else CACHE_MISS;
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
		if (bus_cmd == FLUSH) { *data = bus_data; return DONE; }//reading from main memory complete!
		else return WAITING; //still need to wait
		break;
	}

}

//// the function return 1 if it finished it's command,
////return 0 if not finished (need more cycles) and need to stall.
//int StoreWord(int address, int data, CACHE* cache, int core_index) {
//	int mode;
//	if (address_in_cache(address, cache, &mode)) {
//		if (mode == MODIFIED) {
//			printf("write hit!, address: % x data : % x\n", address, data);
//			//writeto cache
//			return 1;
//		}
//	}
//
//	//if write miss, get address from main memory
//	if (bus_reg_old.bus_cmd == NO_COMMAND) {//check if bus is free
//		BusRdX(core_index, address);
//		printf("Write Miss!\n");
//		GetDataFromMainMemory(address);
//		return 0;
//	}
//
//	if ((bus_reg_old.bus_cmd != NO_COMMAND) && (bus_reg_old.bus_origid == core_index)) {// if bus is busy and it the same core that sent the request
//		if (GetDataFromMainMemory(address)) {//check if the function finished
//			data = bus_reg_old.bus_data;//get the data from bus
//			UpdateCacheBlock(cache);
//			printf("bus finished readx command,address:%x ,data:%x\n", bus_reg_old.bus_addr, bus_reg_old.bus_data);
//			//write to cache
//			return 1;
//		}
//		else
//			return 0;//if not finished
//	}
//
//	else // the bus is busy from another core
//		return 0;//need to wait the bus will be free
//
//}
//
