
#include "simulator.h"

/*********************GLOBAL VARS	*****************/

//this variables hold in index 'i' the number of 'read_hit' for core 'i' (and so on)
int read_hit[CORE_NUM] = { 0 };
int write_hit[CORE_NUM] = { 0 };
int read_miss[CORE_NUM] = { 0 };
int write_miss[CORE_NUM] = { 0 };
/*********************SIMULATOR FUNCTIONS*****************/
// the function return the new status of the memory  { WAITING = 0, DONE = 1, CACHE_MISS = 2 }
int LoadWord(int address, int* data,CORE *core, int prev_status) {
	CACHE *cache= &(core->cache);
	int mode;
	int bus_origid, bus_cmd, bus_addr, bus_data;
	switch (prev_status)
	{
	case DONE://the pervios command done -> the memory is free
		if (address_in_cache(address, cache, &mode)) {
			GetDataFromCache(cache, address, data);
			read_hit[core->id]++;//+1 to read hit
			return DONE; }//get the data from the cache
		else if (mode == MODIFIED) { read_miss[core->id]++; return CONFLICT_MISS; }//conflict miss with a block in MODIFIED mode,  so it need to be written first to the main memory
		else { read_miss[core->id]++;  return CACHE_MISS; }
		break;

	case CONFLICT_MISS:
		if (!bus_is_busy_in_next_cycle()) {//check if bus is free
			write_block_to_main_memory(cache, address % CHACHE_SIZE);//update the current block from the cache in the main memmory
			return CACHE_MISS;
		}
		else return CONFLICT_MISS;
		break;

	case CACHE_MISS:
		if (!bus_is_busy_in_next_cycle()) {//check if bus is free
			BusRd(core->id, address);//update bus 
			return WAITING;
		}
		else {
			//printf("INFO:core %d try to send BusRd reqeust but bus is busy!\n", core->id);
			return CACHE_MISS;
		}
		break;

	case WAITING:
		ReadBusLines(&bus_origid, &bus_cmd, &bus_addr, &bus_data);
		if (bus_cmd == FLUSH) {
		*data = bus_data;
		UpdateCache(cache, bus_addr, bus_data,SHARED);//when FLUSH occurs in the bus, update cache in SHARE mode
		return DONE; }//reading from main memory complete!
		else return WAITING; //still need to wait to FLUSH
		break;
	}
	printf("ERROR: how did you get here?!");
	return -1;//default
}

// the function return the new status of the memory  { WAITING = 0, DONE = 1, CACHE_MISS = 2 }
int StoreWord(int address, int new_data, CORE* core, int prev_status) {
	CACHE* cache = &(core->cache);
	int mode, data;
	int bus_origid, bus_cmd, bus_addr, bus_data;
	switch (prev_status)
	{
	case DONE:
		if (address_in_cache(address, cache, &mode)) {
			if (mode == MODIFIED) {
				GetDataFromCacheExclusive(cache, address, &data);
				WriteToCache(cache, address, new_data);
				write_hit[core->id]++;//increase +1 to write hit
				return DONE;
			}
			else {
				write_miss[core->id]++;
				return CACHE_MISS;
			}
		}
		else if (mode == MODIFIED) {
			write_miss[core->id]++;
			return CONFLICT_MISS;
		}//conflict miss with a block in MODIFIED mode,  so it need to be written first to the main memory
		else {
			write_miss[core->id]++;
			return CACHE_MISS;
		}
		break;

	case CONFLICT_MISS:
		if (!bus_is_busy_in_next_cycle()) {//check if bus is free
			write_block_to_main_memory(cache, address % CHACHE_SIZE);//update the current block from the cache in the main memmory
			return CACHE_MISS;
		}
		else return CONFLICT_MISS;
		break;

	case CACHE_MISS:
		//if address not on the cache, send a BusRdX request on the bus
		if (!bus_is_busy_in_next_cycle()) {//check if bus is free
			BusRdX(core->id, address);//update bus 
			return WAITING;
		}
		else {
			//printf("INFO:core %d try to send BusRd reqeust but bus is busy!\n", core->id);
			return CACHE_MISS;
		}
		break;


	case WAITING:
		ReadBusLines(&bus_origid, &bus_cmd, &bus_addr, &bus_data);
		if (bus_cmd == FLUSH) {
			UpdateCache(cache, bus_addr, bus_data, MODIFIED);//update cache in MODIFIED mode
			WriteToCache(cache, bus_addr, new_data);
			return DONE;
		}//writing  complete!
		else return WAITING; //still need to wait
		break;
	}
	printf("ERROR: how did you get here?!");
	return -1;//default
}


void InitialCore(CORE *core,int id_core) {
	core->id = id_core;
	reset_cache(&core->cache, id_core);
}


int Snooping(CORE* core) {
	int bus_origid, bus_cmd, bus_addr, bus_data;
	int data, mode,index;
	CACHE* cache = &(core->cache);

	ReadBusLines(&bus_origid, &bus_cmd, &bus_addr, &bus_data);
	index = bus_addr % CHACHE_SIZE;
	if ((bus_cmd == BUSRDX) && (bus_origid != core->id)) {//if BusRdx request from another core 
		if (address_in_cache(bus_addr, cache, &mode)) {//check if address in cache 
			if (mode == MODIFIED) {//if the most update data is in this cache in 'M' mode than Flush
				GetDataFromCacheExclusive(cache, bus_addr, &data);
				Flush(bus_addr, data, core->id);//share with other cores and update main memory
				cache->TSRAM[index] = (INVALID << 12) | TAG_BITS(cache->TSRAM[index]);// change mode to INVALID
				printf("core num: %d aborted bus and FLUSH to core:%d. address:0x%08x, data:0x%08x\n", core->id, bus_origid, bus_addr, data);
				return 1;
			}
			if (mode == SHARED) {//only invaldiate the block
				printf("core num: %d invalidate block in cache of address:0x%08x\n", core->id, bus_addr);
				cache->TSRAM[index] = (INVALID << 12) | TAG_BITS(cache->TSRAM[index]);//invalidate
				return 0;
			}
		}
	}
	if ((bus_cmd == BUSRD) && (bus_origid != core->id)) {//if BusRd request from another core
		if (GetDataFromCacheExclusive(cache, bus_addr, &data)) {//if the most update data is in this cache in 'M' mode than Flush
			Flush(bus_addr, data, core->id);//share with other cores and update main memory
			cache->TSRAM[index] = (SHARED << 12) | TAG_BITS(cache->TSRAM[index]);// change mode to share
			printf("core num: %d aborted bus and FLUSH to core:%d. address:0x%08x, data:0x%08x\n", core->id, bus_origid, bus_addr, data);
			return 1;
		}
		return 0;
	}
	return 0;
}

//read watch bit from bus lines and update the watch flag if another core set the watch bit
void update_watch_flag(int* watch_flag,CORE *core) {
	int watch_bit, watch_origid;
	if (*watch_flag == 1)//check if watch flag set
	{
		 read_watch_bit(&watch_bit, &watch_origid);//read watch bit from bus
		if ((watch_bit == 1 )&& (core->id != watch_origid))//another core used the sc command
			*watch_flag = 0;//unset the watch flag
	}
	return;
}

//load link set watch flag to 1 and use LoadWord funtion
int LoadLinked(int address, int* data, CORE* core, int prev_status,int *watch_flag){
	*watch_flag = 1;
	return LoadWord(address, data, core, prev_status);
}

//StoreConditinal checks if others cores used SC before by looking on watch flag status.
//notice new data is pointer
int StoreConditional(int address, int *new_data, CORE* core, int prev_status, int *watch_flag, int * sc_status) {
	int new_status;
	if (*watch_flag == 1) {\
		if (prev_status == DONE) {//first time in SC
			set_watch_bit(core->id);
		}//set the watch bit to '1' to notify other cores 
		new_status=StoreWord(address, *new_data, core, prev_status);
		*sc_status = 1;//set R[rd] to 1 (success)
		if (new_status == DONE) {
			printf("StoreConditional finished\n");
			*watch_flag = 0;
			unset_watch_bit();
		}
		return new_status;
		
	}
	else {
		*sc_status = 0;//set R[rd] to 0 (failure)
		printf("INFO:core: %d failed to SC !\n",core->id);
		return prev_status;
	}
}

void get_hits_and_miss(int core_index, int *read_hit_,int *write_hit_,int *read_miss_,int *write_miss_) {
	*read_hit_ = read_hit[core_index];
	*write_hit_ =write_hit[core_index];
	*read_miss_ = read_miss[core_index];
	*write_miss_ = write_miss[core_index];
}

void print_dsram_and_tsram_wrapper(FILE* dsram, FILE* tsram, CORE* core) {

	print_dsram_and_tsram(dsram, tsram, &core->cache);
}