#ifndef	MAIN_H
#define MAIN_H

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
#define ARGC_NUM 2//need to include  'sim.exe'

typedef struct {
	int reg[REG_NUM];
	int pc;
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

} reg;

void Reset_Reg(reg* r);
void Simulator(FILE* imem1, reg* r1_o, reg* r1_n);
void Sampling_Reg(reg* r_o, reg* r_n);
void FETCH(FILE* imem, reg* r_o, reg* r_n);
void Jump_to_PC(FILE* f, int PC);
void DECODE(reg* r_o, reg* r_n); //not support on stalling yet
void EXE(reg* r_o, reg* r_n);
void ALU(int* aluout, int alu0, int alu1, int opcode);
void printr(reg* r);
void MEM(reg* r_o, reg* r_n);
void WB(reg* r_o, reg* r_n);





#endif 
