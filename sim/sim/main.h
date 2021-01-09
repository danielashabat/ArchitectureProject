#ifndef	MAIN_H
#define MAIN_H

#include "simulator.h"


#define ADD 0
#define SUB 1
#define AND 2
#define OR 3
#define XOR 4
#define MUL 5
#define SLL 6
#define SRA 7
#define SRL 8
#define BEQ 9
#define BNE 10
#define BLT 11
#define BGT 12
#define BLE 13
#define BGE 14
#define JAL 15
#define LW 16
#define SW 17
#define LL 18
#define SC 19
#define HALT 20

#define REG_NUM 16
#define ARGC_NUM 28//need to include  'sim.exe'

 


/*********************STRUCTS*****************/


typedef struct {
	int reg[REG_NUM];
	int pc_FF;
	int pc_FD;
	int pc_DE;
	int pc_EM;
	int pc_MW;
	int inst;
	int alu0;//first input to ALU
	int alu1;//second input to ALU
	int aluout;//output of ALU
	int rs_DE;// DEC/EXE
	int rs_EM;// EXE/MEM
	int rs_MW;// MEM/WB
	int rt_DE;
	int rt_EM;
	int rt_MW;
	int rd_DE;
	int rd_EM;
	int rd_MW;
	int imm;
	int opcode_DE;
	int opcode_EM;
	int opcode_MW;
	int data;
	int status; //status of memory, 0 if avalable, 1 otherwise.
	int sc_status;
	int address;
} Reg;

#define SIGNED_EXT_IMM(IMM) ((IMM>>11)&1) ? (int) (IMM|0xFFFFF000) : IMM;

int Stall_Data_Hazard(Reg* r_o, Reg* r_n);
int Changing_opcode_list(int opcode);
void Reset_Reg(Reg* r);
void Simulator(FILE* imem[], FILE* core_trace[], FILE* stats[], FILE* dsram[], FILE* tsram[], FILE* memin, FILE* memout, FILE* bustrace, FILE* regout[]);
int Checking_halt_for_all(int a[], int num);
void Sampling_Reg(Reg* r_o, Reg* r_n);
void FETCH(FILE* imem, Reg* r_o, Reg* r_n);
void Jump_to_PC(FILE* f, int PC);
void DECODE(Reg* r_o, Reg* r_n, int* stall_counter);
void EXE(Reg* r_o, Reg* r_n);
void ALU(int* aluout, int alu0, int alu1, int opcode);
void printr(Reg* r);
void MEM(Reg* r_o, Reg* r_n, CORE* core, int* stall_counter, int* watch_flag, int* instruction_counter);
int WB(Reg* r_o, Reg* r_n); // , int* instruction_counter
void BranchResulotion(Reg* r_o, Reg* r_n);
void Print_Core_Trace(FILE* f, Reg* r, int cycle);
void Stall_Memory(Reg* r_o, Reg* r_n);
//int Check_for_Stalling_Decode(Reg* r);
void print_stats(int core_index, FILE* stats_file, int cycles, int instructions, int decode_stalls, int memory_stalls);
void print_reg(FILE* F, Reg* r);



#endif 
