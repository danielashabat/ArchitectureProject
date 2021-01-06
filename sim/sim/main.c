#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "memory.h"
#include "simulator.h"

#define FILE_LEN 20
/*command line: sim.exe imem0.txt imem1.txt imem2.txt imem3.txt memin.txt memout.txt
regout0.txt regout1.txt regout2.txt regout3.txt core0trace.txt core1trace.txt
core2trace.txt core3trace.txt bustrace.txt dsram0.txt dsram1.txt dsram2.txt
dsram3.txt tsram0.txt tsram1.txt tsram2.txt tsram3.txt stats0.txt stats1.txt
stats2.txt stats3.txt
*/
int main(int argc, char* argv[]) {
	FILE* imem[CORE_NUM] = { NULL };
	FILE* core_trace[CORE_NUM] = { NULL };
	FILE* dsram[CORE_NUM] = { NULL };
	FILE* tsram[CORE_NUM] = { NULL };
	FILE *stats[CORE_NUM] = { NULL };
	FILE *regout[CORE_NUM] = { NULL };
	FILE *memin = NULL;
	FILE* memout = NULL;
	FILE* bustrace = NULL;
	int i = 0;
	char file_name[FILE_LEN] = {0};
	
	if ((argc != ARGC_NUM) && (argc != 1)) {
		printf("ERROR: there is %d input arguments. (need %d input arguments or 1 argument )\n", argc, ARGC_NUM);
		return 1;
	}

	if (argc == 1) {//default input arguments
		memin = fopen("memin.txt", "r");
		memout = fopen("memout.txt", "w");
		bustrace = fopen("bustrace.txt", "w");
		
		for (i = 0; i < CORE_NUM; i++) {
			sprintf(file_name, "imem%d.txt", i); imem[i] = fopen(file_name, "r");
			sprintf(file_name, "regout%d.txt", i); regout[i] = fopen(file_name, "w");
			sprintf(file_name, "core%dtrace.txt", i); core_trace[i] = fopen(file_name, "w");
			sprintf(file_name, "stats%d.txt", i); stats[i] = fopen(file_name, "w");
			sprintf(file_name, "dsram%d.txt", i); dsram[i] = fopen(file_name, "w");
			sprintf(file_name, "tsram%d.txt", i); tsram[i] = fopen(file_name, "w");
			
			if ((imem[i] == NULL) || (core_trace[i] == NULL )||(stats[i]==NULL)|| (dsram[i] == NULL)|| (tsram[i] == NULL) || (regout[i] == NULL)) {
				printf("cant open one of the files\n");
				return 1;
			}
		}
	}
	else {//take arguments from command line 
		memin = fopen(argv[5], "r");
		memout = fopen(argv[6], "w");
		bustrace = fopen(argv[15], "w");

		for (i = 0; i < CORE_NUM; i++) {
			 imem[i] = fopen(argv[i+1], "r");
			 regout[i] = fopen(argv[i + 7], "w");
			 core_trace[i] = fopen(argv[i + 11], "w");
			 stats[i] = fopen(argv[i + 24], "w");
			 dsram[i] = fopen(argv[i + 16], "w");
			 tsram[i] = fopen(argv[i + 20], "w");

			if ((imem[i] == NULL) || (core_trace[i] == NULL) || (stats[i] == NULL) || (dsram[i] == NULL) || (tsram[i] == NULL) || (regout[i] == NULL)) {
				printf("cant open one of the files\n");
				return 1;
			}
		}
	}

	if ((memin==NULL)||(memout==NULL)||(bustrace==NULL)) {
		printf("cant open one of the files\n");
		return 1;
	}


	Simulator(imem, core_trace,stats,dsram, tsram ,memin, memout,bustrace, regout);
	for (i = 0; i < CORE_NUM; i++) {//close files for each core
		fclose(imem[i]); fclose(core_trace[i]); fclose(stats[i]); fclose(dsram[i]); fclose(tsram[i]); fclose(regout[i]);
	}
	fclose(bustrace); fclose(memin); fclose(memout);//close general files
	return 0;
}

void Simulator(FILE* imem[],FILE *core_trace[],FILE *stats[],FILE *dsram[],FILE *tsram[],FILE *memin, FILE *memout,FILE * bustrace, FILE *regout[])
{
	Reg registers_o[CORE_NUM];
	Reg registers_n[CORE_NUM];
	int i = 0;
	int cycles[CORE_NUM] = { 0 };
	int instructions[CORE_NUM] = { 0 };
	int decode_stalls[CORE_NUM] = { 0 };
	int memory_stalls[CORE_NUM] = { 0 };
	for (i = 0; i < CORE_NUM; i++) Reset_Reg(&registers_o[i]);
	for (i = 0; i < CORE_NUM; i++) Reset_Reg(&registers_n[i]);
	int flag1=1;
	int cycle_counter = 0;
	int watch_flag[CORE_NUM] = { 0 };
	
	int continue_flag[CORE_NUM] = { 0 }; // will use for halt
	CORE cores[CORE_NUM];
	for (i = 0; i < CORE_NUM; i++) InitialCore(&cores[i], i);//reset the cores
	InitialMainMemory(memin);
	InitialBus();//reset the bus lines

	while (1)
	{	
		update_main_memory(cycle_counter, bustrace);
		for ( i = 0; i < CORE_NUM; i++)
		{
			if (continue_flag[i] == 0) {
				//printf("cycle %d\n", cycle_counter);
				FETCH(imem[i], &registers_o[i], &registers_n[i]);
				DECODE(&registers_o[i], &registers_n[i], &decode_stalls[i]);
				EXE(&registers_o[i], &registers_n[i]);
				MEM(&registers_o[i], &registers_n[i], &cores[i], &memory_stalls[i], &watch_flag[i]);
				continue_flag[i] = WB(&registers_o[i], &registers_n[i], &instructions[i]);
				if (continue_flag[i]) cycles[i] = cycle_counter;
				Print_Core_Trace(core_trace[i], &registers_o[i], cycle_counter);
				//if (Check_for_Stalling_Decode(&registers_o[i])) decode_stalls[i] = decode_stalls[i] + 1;
				Sampling_Reg(&registers_o[i], &registers_n[i]);
				update_watch_flag(&watch_flag[i], &cores[i]);
			}
			Snooping(&cores[i]);//help another cores with LW/SW
			
		}
		if (Checking_halt_for_all(continue_flag, CORE_NUM)) break;
		sample_bus();
		cycle_counter++;
		
	}
	while (bus_is_busy_in_next_cycle()) {
		update_main_memory(cycle_counter, bustrace);
		sample_bus();
		cycle_counter++;
	}
	for (i = 0; i < CORE_NUM ; i++) {
		print_stats(i, stats[i], cycles[i], instructions[i], decode_stalls[i], memory_stalls[i]);
		print_reg(regout[i], &registers_o[i]);
		print_dsram_and_tsram_wrapper(dsram[i], tsram[i], &cores[i]);
	}
	
	print_memout(memout);
}

void print_reg(FILE* F, Reg* r)
{
	fprintf(F, "%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X\n%08X \n", r->reg[2], r->reg[3], r->reg[4], r->reg[5], r->reg[6], r->reg[7], r->reg[8], r->reg[9], r->reg[10], r->reg[11], r->reg[12], r->reg[13], r->reg[14], r->reg[15]);
}

int Checking_halt_for_all(int a[], int num)
{
	for (int i = 0; i < num; i++) if (a[i] == 0) return 0;
	return 1;
}

//int Check_for_Stalling_Decode(Reg* r) {
//	if (r->pc_DE == -1 && r->pc_FD!= -1) {
//		if ((r->pc_EM == -1 && r->pc_MW == -1) || (r->pc_EM == -1 && r->pc_MW != -1) || (r->pc_EM != -1 && r->pc_MW != -1)) return 1;
//			
//	}
//	else return 0;
//	return 0;
//}





void Reset_Reg(Reg* r)
{
	for (int i = 0; i < REG_NUM; i++)
	{
		r->reg[i] = 0;
	}
	r->pc_FF=0;
	r->pc_FD=-1;
	r->pc_DE=-1;
	r->pc_EM=-1;
	r->pc_MW=-1;
	r->inst = 0;
	r->alu0 = 0;
	r->alu1 = 0;
	r->aluout = 0;
	r->rs_DE = 0;
	r->rs_EM = 0;
	r->rs_MW = 0;
	r->rt_DE = 0;
	r->rt_EM = 0;
	r->rt_MW = 0;
	r->rd_DE = 0;
	r->rd_EM = 0;
	r->rd_MW = 0;
	r->imm = 0;
	r->opcode_DE = 0;
	r->opcode_EM = 0;
	r->opcode_MW = 0;
	r->data = 0;
	r->status = DONE;
	return;
}

void Sampling_Reg(Reg* r_o, Reg* r_n)
{
	for (int i = 0; i < REG_NUM; i++) r_o->reg[i] = r_n->reg[i];
	r_o->pc_FF = r_n->pc_FF;
	r_o->pc_FD = r_n->pc_FD;
	r_o->pc_DE = r_n->pc_DE;
	r_o->pc_EM = r_n->pc_EM;
	r_o->pc_MW = r_n->pc_MW;
	r_o->inst = r_n->inst;
	r_o->alu0 = r_n->alu0;
	r_o->alu1 = r_n->alu1;
	r_o->aluout = r_n->aluout;
	r_o->rd_DE = r_n->rd_DE;
	r_o->rd_EM = r_n->rd_EM;
	r_o->rd_MW = r_n->rd_MW;
	r_o->rt_DE = r_n->rt_DE;
	r_o->rt_EM = r_n->rt_EM;
	r_o->rt_MW = r_n->rt_MW;
	r_o->rs_DE = r_n->rs_DE;
	r_o->rs_EM = r_n->rs_EM;
	r_o->rs_MW = r_n->rs_MW;
	r_o->imm = r_n->imm;
	r_o->opcode_DE= r_n->opcode_DE;
	r_o->opcode_EM = r_n->opcode_EM;
	r_o->opcode_MW = r_n->opcode_MW;
	r_o->data = r_n->data;
	r_o->status = r_n->status;
	return;
}

void FETCH(FILE *imem, Reg* r_o, Reg* r_n)
{
	if (r_o->pc_FF != -1) {
		Jump_to_PC(imem, r_o->pc_FF);
		//printf("doing Fetch to pc= %d\n", r_o->pc_FF);
		fscanf(imem, "%08x\n", &r_n->inst);
		r_n->pc_FF = r_o->pc_FF + 1;
		r_n->pc_FD = r_o->pc_FF;
	}
	else r_n->pc_FD = -1;
	
	return;
}

void DECODE(Reg* r_o, Reg* r_n, int *stall_counter)  // needs to fix halt 
{
	r_n->reg[1] = r_o->inst & 0x00000fff;
	r_o->reg[1] = r_o->inst & 0x00000fff; //test
	//printf("doing DECODE to inst= %08x\n", r_o->inst);
	r_n->rt_DE = (r_o->inst & 0x0000f000) >> 12;
	r_n->rs_DE = (r_o->inst & 0x000f0000) >>16 ;
	r_n->rd_DE = (r_o->inst & 0x00f00000)>>20;
	r_n->opcode_DE = (r_o->inst & 0xff000000)>>24;
	if (r_n->opcode_DE == 20)
	{
		r_n->pc_FF = -1;
		r_n->pc_DE = r_o->pc_FD;
		r_n->pc_FD = -1;
		return;
	}
	if (Stall_Data_Hazard(r_o, r_n)) { //if stall due to data hazard
		r_n->inst = r_o->inst; 
		r_n->pc_FF = r_o->pc_FF;
		r_n->pc_FD = r_o->pc_FD;
		r_n->pc_DE = -1;
		r_n->opcode_DE = -1;
		
		if (r_o->status==DONE) *stall_counter = *stall_counter + 1; 
		
	}
	else
	{
		if (r_n->rs_DE != 1) r_n->alu0 = r_o->reg[r_n->rs_DE];
		else r_n->alu0 = r_n->reg[1];
		if (r_n->rt_DE != 1) r_n->alu1 = r_o->reg[r_n->rt_DE];
		else r_n->alu1 = r_n->reg[1];
		if (r_n->opcode_DE >= BEQ && r_n->opcode_DE <= JAL) BranchResulotion(r_o, r_n); //handling beanch resulotion
		r_n->pc_DE = r_o->pc_FD;
	}
	
	return;
}

int Stall_Data_Hazard(Reg* r_o, Reg* r_n)  
{
	if ((r_n->opcode_DE >= ADD && r_n->opcode_DE <= SRL) || r_n->opcode_DE==LW)
	{
		if ((r_n->rs_DE>1) && (((r_n->rs_DE == r_o->rd_DE) && Changing_opcode_list(r_o->opcode_DE)) || ((r_n->rs_DE == r_o->rd_EM) && Changing_opcode_list(r_o->opcode_EM)) || ((r_n->rs_DE == r_o->rd_MW) &&Changing_opcode_list(r_o->opcode_MW)))) return 1;
		if ((r_n->rt_DE > 1) &&(((r_n->rt_DE == r_o->rd_DE) && Changing_opcode_list(r_o->opcode_DE)) || ((r_n->rt_DE == r_o->rd_EM) && Changing_opcode_list(r_o->opcode_EM)) || ((r_n->rt_DE == r_o->rd_MW) && Changing_opcode_list(r_o->opcode_MW)))) return 1;
		return 0;
	}
	if ((r_n->opcode_DE >= BEQ && r_n->opcode_DE <= BGE) || r_n->opcode_DE==SW || r_n->opcode_DE == LL || r_n->opcode_DE == SC)
	{
		if ((r_n->rs_DE > 1) && (((r_n->rs_DE == r_o->rd_DE) && Changing_opcode_list(r_o->opcode_DE)) || ((r_n->rs_DE == r_o->rd_EM) && Changing_opcode_list(r_o->opcode_EM)) || ((r_n->rs_DE == r_o->rd_MW) && Changing_opcode_list(r_o->opcode_MW)))) return 1;
		if ((r_n->rt_DE > 1) &&(((r_n->rt_DE == r_o->rd_DE) && Changing_opcode_list(r_o->opcode_DE)) || ((r_n->rt_DE == r_o->rd_EM) && Changing_opcode_list(r_o->opcode_EM)) || ((r_n->rt_DE == r_o->rd_MW) && Changing_opcode_list(r_o->opcode_MW)))) return 1;
		if ((r_n->rd_DE > 1 )&&(((r_n->rd_DE == r_o->rd_DE) && Changing_opcode_list(r_o->opcode_DE)) || ((r_n->rd_DE == r_o->rd_EM) && Changing_opcode_list(r_o->opcode_EM)) || ((r_n->rd_DE == r_o->rd_MW) && Changing_opcode_list(r_o->opcode_MW)))) return 1;
		return 0;
	}
	if (r_n->opcode_DE == JAL) {
		if ((r_n->rd_DE >1 )&&(((r_n->rd_DE == r_o->rd_DE) && Changing_opcode_list(r_o->opcode_DE)) || ((r_n->rd_DE == r_o->rd_EM) && Changing_opcode_list(r_o->opcode_EM)) || ((r_n->rd_DE == r_o->rd_MW) && Changing_opcode_list(r_o->opcode_MW)))) return 1;
		return 0;
	}
	return 0;

}

int Changing_opcode_list(int opcode)
{
	if (opcode == -1) return 0;
	if (opcode >= ADD && opcode <= SRL) return 1;
	if (opcode == LW) return 1;
	return 0;
}

void BranchResulotion(Reg* r_o, Reg* r_n)
{
	switch (r_n->opcode_DE)
	{
		case BEQ:
			if (r_o->reg[r_n->rs_DE] == r_o->reg[r_n->rt_DE]) r_n->pc_FF = ((r_o->reg[r_n->rd_DE]) & 0x000003ff);
			break;
		case BNE:
			if (r_o->reg[r_n->rs_DE] != r_o->reg[r_n->rt_DE]) r_n->pc_FF = ((r_o->reg[r_n->rd_DE]) & 0x000003ff);
			break;
		case BLT:
			if (r_o->reg[r_n->rs_DE] < r_o->reg[r_n->rt_DE]) r_n->pc_FF = ((r_o->reg[r_n->rd_DE]) & 0x000003ff);
			break;
		case BGT:
			if (r_o->reg[r_n->rs_DE] > r_o->reg[r_n->rt_DE]) r_n->pc_FF = ((r_o->reg[r_n->rd_DE]) & 0x000003ff);
			break;
		case BLE:
			if (r_o->reg[r_n->rs_DE] <= r_o->reg[r_n->rt_DE]) r_n->pc_FF = ((r_o->reg[r_n->rd_DE]) & 0x000003ff);
			break;
		case BGE:
			if (r_o->reg[r_n->rs_DE] >= r_o->reg[r_n->rt_DE]) r_n->pc_FF = ((r_o->reg[r_n->rd_DE]) & 0x000003ff);
			break;
		case JAL:
			r_n->reg[15] = r_o->pc_FF;
			r_n->pc_FF= (r_o->reg[r_n->rd_DE]) & 0x000003ff;
			break;
	
	}
	return;
}


void EXE(Reg* r_o, Reg* r_n)
{
	ALU(&r_n->aluout, r_o->alu0, r_o->alu1, r_o->opcode_DE);
	r_n->rs_EM = r_o->rs_DE;
	r_n->rt_EM = r_o->rt_DE;
	r_n->rd_EM = r_o->rd_DE;
	r_n->opcode_EM = r_o->opcode_DE;
	r_n->pc_EM = r_o->pc_DE;
	return;
}

void MEM(Reg* r_o, Reg* r_n, CORE *core, int *stall_counter, int *watch_flag)
{
	
	//LoadLinked(int address, int* data, CORE* core, int prev_status, int* watch_flag);
	//int StoreConditional(int address, int* new_data, CORE * core, int prev_status, int* watch_flag);
	
	if (r_o->opcode_EM >= LW && r_o->opcode_EM <= SC)
	{
		if (r_o->opcode_EM == LW)
		{
			if ((r_n->status =LoadWord(r_o->aluout, &r_n->data, core, r_o->status))!= DONE) //data not ready
			{
				Stall_Memory(r_o,r_n);
				*stall_counter = *stall_counter + 1;
				return;

			}
			
		}
		if (r_o->opcode_EM == SW)
		{
			if ((r_n->status = StoreWord(r_o->aluout, r_o->reg[r_o->rd_EM], core, r_o->status))!= DONE) 
			{
				
				Stall_Memory(r_o, r_n);
				*stall_counter = *stall_counter + 1;
				return;
			}
		}
		if (r_o->opcode_EM == LL)
		{
			if ((r_n->status = LoadLinked(r_o->aluout, &r_n->data, core, r_o->status, watch_flag)) != DONE) //data not ready
			{
				Stall_Memory(r_o, r_n);
				*stall_counter = *stall_counter + 1;
				return;
			}
			
		}
		if (r_o->opcode_EM == SC)
		{
			if ((r_n->status = StoreConditional(r_o->aluout, &r_o->reg[r_o->rd_EM], core, r_o->status, watch_flag)) != DONE)
			{

				Stall_Memory(r_o, r_n);
				*stall_counter = *stall_counter + 1;
				return;
			}
		}


	}
	else {
		r_n->data = r_o->aluout;
		
	} // in the case that no memory opcode
	r_n->rs_MW = r_o->rs_EM;
	r_n->rt_MW = r_o->rt_EM;
	r_n->rd_MW = r_o->rd_EM;
	r_n->opcode_MW = r_o->opcode_EM;
	r_n->pc_MW = r_o->pc_EM;
	return;
}

void Stall_Memory(Reg* r_o, Reg* r_n)
{
	r_n->inst = r_o->inst;
	r_n->pc_FF = r_o->pc_FF;
	r_n->pc_MW = -1;
	r_n->opcode_MW = -1;
	r_n->pc_EM = r_o->pc_EM;
	r_n->pc_DE = r_o->pc_DE;
	r_n->pc_FD = r_o->pc_FD;
	r_n->alu0 = r_o->alu0;
	r_n->alu1 = r_o->alu1;
	r_n->rd_EM = r_o->rd_EM;
	r_n->rt_EM = r_o->rt_EM;
	r_n->rs_EM = r_o->rs_EM;
	r_n->opcode_EM = r_o->opcode_EM;
	return;
}

int WB(Reg* r_o, Reg* r_n, int* instruction_counter)
{
	
	if (r_o->opcode_MW != -1) *instruction_counter = *instruction_counter + 1;
	if (r_o->opcode_MW == 20) return 1;
	if ((r_o->opcode_MW >= ADD && r_o->opcode_MW <= SRL)|| r_o->opcode_MW==LW)
	{
		if (r_o->rd_MW!=0 && r_o->rd_MW != 1) r_n->reg[r_o->rd_MW] = r_o->data;
	}
	
	return 0;
	
}

void Jump_to_PC(FILE* f, int PC) {
	fseek(f, PC * 10, 0);//10 bytes in line 
}

void ALU(int* aluout, int alu0, int alu1, int opcode)
{
	switch (opcode)
	{
	case ADD:
		*aluout = alu0 + alu1;
		break;
	case SUB:
		*aluout = alu0 - alu1;
		break;
	case AND:
		*aluout = alu0 & alu1;
		break;
	case OR:
		*aluout = alu0 | alu1;
		break;
	case XOR:
		*aluout = alu0 ^ alu1;
		break;
	case MUL:
		*aluout = alu0 * alu1;
		break;
	case SLL:
		*aluout = (unsigned)alu0 << alu1;
		break;
	case SRA:
		*aluout = alu0 >> alu1; //check 
	case SRL:
		*aluout = (unsigned)alu0 >> alu1;
		break;
	case LW:
	case LL:
	case SW:
	case SC:

		*aluout = alu0 + alu1;
		break;
	
		
	}
}

/*********************PRINT FUNCTIONS*****************/

void printr(Reg* r)
{
	printf("reg[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]\n", r->reg[0], r->reg[1], r->reg[2], r->reg[3], r->reg[4], r->reg[5], r->reg[6], r->reg[7], r->reg[8], r->reg[9], r->reg[10],r->reg[11], r->reg[12], r->reg[13], r->reg[14], r->reg[15]); 
	printf("inst=%08x, alu0=%d, alu1=%d, aluout=%d, rs_DE=%d, rs_EM=%d, rs_MW=%d,rt_DE=%d, rt_EM=%d, rt_MW=%d, rd_DE=%d, rd_EM=%d, rd_MW=%d \n", r->inst, r->alu0, r->alu1, r->aluout, r->rs_DE, r->rs_EM, r->rs_MW, r->rt_DE, r->rt_EM, r->rt_MW, r->rd_DE, r->rd_EM, r->rd_MW);
	printf("opcode_DE=%d, opcode_EM= %d, opcode_MW=%d\n", r->opcode_DE, r->opcode_EM, r->opcode_MW);
}

void Print_Core_Trace(FILE* f, Reg* r, int cycle)
{
	fprintf(f, "%d ", cycle);
	if (r->pc_FF != -1) fprintf(f, "%03x ", r->pc_FF);
	else fprintf(f, "--- ");
	if (r->pc_FD != -1) fprintf(f, "%03x ", r->pc_FD);
	else fprintf(f, "--- ");
	if (r->pc_DE != -1) fprintf(f, "%03x ", r->pc_DE);
	else fprintf(f, "--- ");
	if (r->pc_EM != -1) fprintf(f, "%03x ", r->pc_EM);
	else fprintf(f, "--- ");
	if (r->pc_MW != -1) fprintf(f, "%03x ", r->pc_MW);
	else fprintf(f, "--- "); //print the old
	fprintf(f, "%08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x \n", r->reg[2], r->reg[3], r->reg[4], r->reg[5], r->reg[6], r->reg[7], r->reg[8], r->reg[9], r->reg[10], r->reg[11], r->reg[12], r->reg[13], r->reg[14], r->reg[15]);
}

void print_stats(int core_index, FILE* stats_file, int cycles, int instructions, int decode_stalls, int memory_stalls) {
	int read_hit;
	int write_hit;
	int read_miss;
	int write_miss;
	get_hits_and_miss(core_index ,&read_hit, &write_hit, &read_miss, &write_miss);
	fprintf(stats_file, "cycles %d\ninstructions %d\nread hit %d\nwrite hit %d\nread_miss %d\nwrite_miss %d\ndecode_stall %d\nmem_stall %d\n", cycles+1, instructions-4, read_hit, write_hit, read_miss, write_miss, decode_stalls, memory_stalls);
}