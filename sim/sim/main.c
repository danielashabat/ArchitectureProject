#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "main.h"
#include "memory.h"


/*command line: sim.exe imem0.txt imem1.txt imem2.txt imem3.txt memin.txt memout.txt
regout0.txt regout1.txt regout2.txt regout3.txt core0trace.txt core1trace.txt
core2trace.txt core3trace.txt bustrace.txt dsram0.txt dsram1.txt dsram2.txt
dsram3.txt tsram0.txt tsram1.txt tsram2.txt tsram3.txt stats0.txt stats1.txt
stats2.txt stats3.txt
*/
int main(int argc, char* argv[]) {
	FILE* imem1 = NULL;
	reg reg1_o, reg1_n;
	
	Reset_Reg(&reg1_o);
	Reset_Reg(&reg1_n);
	if (argc != ARGC_NUM) {
		printf("ERROR: there is %d input arguments. (need %d input arguments)\n", argc, ARGC_NUM);
		return 1;
	}

	imem1 = fopen(argv[1], "r");
	if (imem1 == NULL) {
		printf("cant open\n");
		return 1;
	}

	//Daniela debug:
	//CACHE cache;
	//reset_cache(&cache);
	//cache.TSRAM[0xFF] = 0xFFFFFFF;
	//return address_in_cache(0xFFFFF, &cache);

	Simulator(imem1, &reg1_o, &reg1_n);
	fclose(imem1);
	
	return 0;
}

void Simulator(FILE* imem1, reg* r1_o, reg* r1_n)
{
	int flag1=1;
	int cycle_counter = 1;
	while (1)
	{
		FETCH(imem1, r1_o, r1_n);
		DECODE(r1_o, r1_n);
		EXE(r1_o, r1_n);
		MEM(r1_o, r1_n);
		WB(r1_o, r1_n);
		Sampling_Reg(r1_o, r1_n);
		printf("cycle %d\n", cycle_counter);
		printr(r1_o);
		
		cycle_counter++;
		if (cycle_counter == 10) break; //for test
	}
}

void Reset_Reg(reg* r)
{
	for (int i = 0; i < REG_NUM; i++)
	{
		r->reg[i] = 0;
	}
	r->pc = 0;
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
	return;
}

void Sampling_Reg(reg* r_o, reg* r_n)
{
	for (int i = 0; i < REG_NUM; i++) r_o->reg[i] = r_n->reg[i];
	r_o->pc = r_n->pc;
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
	return;
}

void FETCH(FILE *imem, reg* r_o, reg* r_n)
{
	
	Jump_to_PC(imem, r_o->pc);
	//printf("doing Fetch to pc= %d\n", r_o->pc); 
	fscanf(imem, "%08x\n", &r_n->inst);
	r_n->pc = r_o->pc + 1;
}

void DECODE(reg* r_o, reg* r_n) //not support on stalling and branch resulotion yet
{
	r_n->reg[1] = r_o->inst & 0x00000fff;
	//printf("doing DECODE to inst= %08x\n", r_o->inst);
	r_n->rt_DE = (r_o->inst & 0x0000f000) >> 12;
	r_n->rs_DE = (r_o->inst & 0x000f0000) >>16 ;
	r_n->rd_DE = (r_o->inst & 0x00f00000)>>20;
	r_n->opcode_DE = (r_o->inst & 0xff000000)>>24;
	if (r_n->rs_DE != 1) r_n->alu0 = r_o->reg[r_n->rs_DE];
	else r_n->alu0 = r_n->reg[1];
	if (r_n->rt_DE != 1) r_n->alu1 = r_o->reg[r_n->rt_DE];
	else r_n->alu1 = r_n->reg[1];

	return;
}

void EXE(reg* r_o, reg* r_n)
{
	ALU(&r_n->aluout, r_o->alu0, r_o->alu1, r_o->opcode_DE);
	r_n->rs_EM = r_o->rs_DE;
	r_n->rt_EM = r_o->rt_DE;
	r_n->rd_EM = r_o->rd_DE;
	r_n->opcode_EM = r_o->opcode_DE;
	return;
}

void MEM(reg* r_o, reg* r_n)
{
	r_n->rs_MW = r_o->rs_EM;
	r_n->rt_MW = r_o->rt_EM;
	r_n->rd_MW = r_o->rd_EM;
	r_n->opcode_MW = r_o->opcode_EM;
	r_n->data = r_o->aluout; // in the case that no memory opcode
	return;
}

void WB(reg* r_o, reg* r_n)
{
	if (r_o->opcode_MW >= ADD && r_o->opcode_MW <= SRL)
	{
		r_n->reg[r_o->rd_MW] = r_o->data;
	}
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
		*aluout = alu0 << alu1;
		break;
	//case SRA:
		//*aluout = alu0  alu1;
	case SRL:
		*aluout = alu0 >> alu1;
		break;

	}
}

void printr(reg* r)
{
	printf("reg[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]\n", r->reg[0], r->reg[1], r->reg[2], r->reg[3], r->reg[4], r->reg[5], r->reg[6], r->reg[7], r->reg[8], r->reg[9], r->reg[10],r->reg[11], r->reg[12], r->reg[13], r->reg[14], r->reg[15]); 
	printf("inst=%08x, alu0=%d, alu1=%d, aluout=%d, rs_DE=%d, rs_EM=%d, rs_MW=%d,rt_DE=%d, rt_EM=%d, rt_MW=%d, rd_DE=%d, rd_EM=%d, rd_MW=%d \n", r->inst, r->alu0, r->alu1, r->aluout, r->rs_DE, r->rs_EM, r->rs_MW, r->rt_DE, r->rt_EM, r->rt_MW, r->rd_DE, r->rd_EM, r->rd_MW);
	printf("opcode_DE=%d, opcode_EM= %d, opcode_MW=%d\n", r->opcode_DE, r->opcode_EM, r->opcode_MW);
}

