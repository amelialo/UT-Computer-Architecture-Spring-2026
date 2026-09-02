/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND2, COND1, COND0, //new bit
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    LD_USP, //add new signals in order
    LD_SSP,
    LD_VECT,
    LD_PRIV,
    GATE_SP,
    GATE_VECT,
    GATE_PSR,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    SPMUX,
    VECTMUX,
    PSRMUX,
    PRIVMUX,
    PCMUX1, PCMUX0,
    DRMUX1, DRMUX0, //new bit
    SR1MUX1, SR1MUX0, //new bit
    ADDR1MUX,
    ADDR2MUX2, ADDR2MUX1, ADDR2MUX0, //new bit
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
/* MODIFY: you have to add all your new control signals */
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND2] << 2) + (x[COND1] << 1) + x[COND0]); } //new bit
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return((x[DRMUX1] << 1) + x[DRMUX0]); } //new bit
int GetSR1MUX(int *x)        { return((x[SR1MUX1] << 1) + x[SR1MUX0]); } //new bit
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX2] << 2) + (x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); } //new bit
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }
//new signals
int GetLD_USP(int *x)        { return(x[LD_USP]); }
int GetLD_SSP(int *x)        { return(x[LD_SSP]); }
int GetLD_VECT(int *x)       { return(x[LD_VECT]); }
int GetLD_PRIV(int *x)       { return(x[LD_PRIV]); }
int GetGATE_SP(int *x)       { return(x[GATE_SP]); }
int GetGATE_VECT(int *x)     { return(x[GATE_VECT]); }
int GetGATE_PSR(int *x)      { return(x[GATE_PSR]); }
int GetSPMUX(int *x)         { return(x[SPMUX]); }
int GetVECTMUX(int *x)       { return(x[VECTMUX]); }
int GetPSRMUX(int *x)        { return(x[PSRMUX]); }
int GetPRIVMUX(int *x)       { return(x[PRIVMUX]); }

/* MODIFY: you can add more Get functions for your new control signals */

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
/* MODIFY: You may add system latches that are required by your implementation */

int PRIVILEGE; //PSR[15]
int SAVED_USP;
int VECTOR;

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
    int i;

    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
	if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
	}
	cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
	cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
    int address; /* this is a byte address */

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
    int k; 

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
	go();
	break;

    case 'M':
    case 'm':
	scanf("%i %i", &start, &stop);
	mdump(dumpsim_file, start, stop);
	break;

    case '?':
	help();
	break;
    case 'Q':
    case 'q':
	printf("Bye.\n");
	exit(0);

    case 'R':
    case 'r':
	if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
	else {
	    scanf("%d", &cycles);
	    run(cycles);
	}
	break;

    default:
	printf("Invalid Command\n");
	break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
	MEMORY[i][0] = 0;
	MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
	program_base = word >> 1;
    else {
	printf("Error: Program file is empty\n");
	exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
	/* Make sure it fits. */
	if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
		   program_filename, ii);
	    exit(-1);
	}

	/* Write the word to memory array. */
	MEMORY[program_base + ii][0] = word & 0x00FF;
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *argv[], int num_prog_files) { 
    int i;
    init_control_store(argv[1]);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(argv[i + 2]);
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */
    //CURRENT_LATCHES.SAVED_SSP = 0x3000;
    CURRENT_LATCHES.PRIVILEGE = 1; //start with user mode
    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv, argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.

   Do not modify the rdump and mdump functions.

   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/
int INT_SIGNAL = 0;

int sext(int value, int bits) {
  int sign_bit = 1 << (bits - 1); // msb mask based on number of bits
  if (value & sign_bit) {
    // if negative, sext to 32 bits
    return value | (0xFFFFFFFF << bits); 
  }
  return value; // otherwise it's positive
}

void eval_micro_sequencer() {

  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   */
    int* uinst = CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER];
    int ird = GetIRD(uinst);
    int cond = GetCOND(uinst);
    int j = GetJ(uinst);

    int EXC_SIGNAL = 0; //sets flag down each cycle
    int state = 0;

    if(CYCLE_COUNT == 299){ //interrupt once at cycle 300
        INT_SIGNAL = 1;
    }
    // PRIORITY LOGIC
    if(GetMIO_EN(uinst)){
        if(CURRENT_LATCHES.PRIVILEGE == 1 && (CURRENT_LATCHES.MAR >= 0x0000) && (CURRENT_LATCHES.MAR < 0x3000) && CURRENT_LATCHES.STATE_NUMBER != 28){ //PROTECTION EXCEPTION, ignore trap 28
            NEXT_LATCHES.EXCV = 0x02;
            EXC_SIGNAL = 1;
        }else if(GetDATA_SIZE(uinst) && (CURRENT_LATCHES.MAR & 0x01)){  //UNALIGNED ACCESS
            NEXT_LATCHES.EXCV = 0x03;
            EXC_SIGNAL = 1;
        }
    }else if(CURRENT_LATCHES.STATE_NUMBER == 32){ //DECODING UNKNWON OPCODE
        int opcode = (CURRENT_LATCHES.IR >> 12) & 0xF; //IR[15:12]
        if (opcode == 10 || opcode == 11) {
            NEXT_LATCHES.EXCV = 0x04;
            EXC_SIGNAL = 1;
        }
    }

    if(EXC_SIGNAL){
        state = 10;
    }else{ //no exception, check interrupt
        if(ird == 1){
            state = (CURRENT_LATCHES.IR >> 12) & 0xF;  //state = opcode
        }
        else{
            if(cond == 0){  //default
                state = j;
            }else if (cond == 1){ // memory ready
                state = j | (CURRENT_LATCHES.READY << 1); //.ready is 1/0 so shift left to make it +2 once ready
            }else if(cond == 2){  // branch
                state = j | (CURRENT_LATCHES.BEN << 2); // +4
            }else if (cond == 3){  // addressing mode, only in state 4
                int ir11 = (CURRENT_LATCHES.IR >> 11) & 0x01;
                state = j | ir11;  //ir11 can be 0 or 1
            }else if (cond == 4){  //PSR[15]
                state = j | (CURRENT_LATCHES.PRIVILEGE << 3);
            }else if (cond == 5){  //INT
                if (INT_SIGNAL) { //just interrupt
                    state = 36;
                    NEXT_LATCHES.INTV = 0x01;
                    INT_SIGNAL = 0; // set flag back down to prevent infinite interrupts
                } else 
                    state = j; 
            }
        }
    }

   NEXT_LATCHES.STATE_NUMBER = state;
   for (int i = 0; i < CONTROL_STORE_BITS; i++) {  // next cycles microinst
        NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[state][i];
    }
}


void cycle_memory() {
 
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */
    static int count = 0; //keep track of clock cycles
    int* uinst = CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER];

    if(GetMIO_EN(uinst) == 1){
        count++;
        if(count < 4){
            NEXT_LATCHES.READY = 0;
        }else if(count==4){ //READY bit on in cycle 4
            NEXT_LATCHES.READY = 1;
        }else if(count==5){  // CYCLE 5 - reset everything, read/write
            NEXT_LATCHES.READY = 0;
            count = 0;
            if(GetR_W(uinst)==1){ //if write... when reading, dont do anything
                int wordRow = CURRENT_LATCHES.MAR >> 1; //divide by 2 because word aligned
                int mdrVal = CURRENT_LATCHES.MDR;
                if(GetDATA_SIZE(uinst)==1){ //write a word
                    MEMORY[wordRow][0] = mdrVal & 0xFF;
                    MEMORY[wordRow][1] = (mdrVal >> 8) & 0xFF;
                }else{ //write a byte
                    //check if even or odd
                    if((CURRENT_LATCHES.MAR & 0x1) == 1) //odd address
                        MEMORY[wordRow][1] = mdrVal & 0xFF;
                    else
                        MEMORY[wordRow][0] = mdrVal & 0xFF;
                }
            } // read in latches
        }
    }else{ //MIO_EN = 0
        count = 0;
        NEXT_LATCHES.READY = 0;
    }
}




int g_marmux;
int g_pc;
int g_alu;
int g_shf;
int g_mdr;
int g_psr;
int g_sp;
int g_vect;

void eval_bus_drivers() {

  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *         Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR,
   *         Gate_SP,
   *         Gate_VECT,
   *         Gate_PSR.
   */    

    int* uinst = CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER];

    // GATE SP -----------------------------------------------------------
    if(GetSPMUX(uinst)==0)
        g_sp = CURRENT_LATCHES.SAVED_USP;
    else   // 1
        g_sp = CURRENT_LATCHES.SSP;

    // GATE VECT -----------------------------------------------------------
    g_vect = 0x0200 + (CURRENT_LATCHES.VECTOR << 1);

    // GATE PSR -----------------------------------------------------------
    g_psr = (CURRENT_LATCHES.PRIVILEGE << 15) | (CURRENT_LATCHES.N << 2) | (CURRENT_LATCHES.Z << 1) | CURRENT_LATCHES.P;

    // GATE MARMUX ----------------------------------------------------------- changed because added bits to addr2, sr1
    if(GetMARMUX(uinst)==0){ //7.0 select LSHF(ZEXT[IR[7.0]],1)  TRAP
        g_marmux = (CURRENT_LATCHES.IR & 0xFF) << 1;
    }else{
        int addr1 = 0;
        int addr2 = 0;
        // ADDR1
        if(GetADDR1MUX(uinst)==0){ //PC
            addr1 = CURRENT_LATCHES.PC;
        }else{ // BaseR = sr1out
            int reg;
            if(GetSR1MUX(uinst)==0){// IR[11.9]
                reg = (CURRENT_LATCHES.IR >> 9) & 0x7;
            }else if(GetSR1MUX(uinst)==1){//IR[8.6]
                reg = (CURRENT_LATCHES.IR >> 6) & 0x7;
            }else{
                reg = 6; // R6 stack pointer
            }
            addr1 = CURRENT_LATCHES.REGS[reg];  //get actual address within register
        }
        // ADDR2
        if(GetADDR2MUX(uinst)==0){  //ZERO
            addr2 = 0;
        }else if(GetADDR2MUX(uinst)==1){ //offset6
            addr2 = sext(CURRENT_LATCHES.IR & 0x3F, 6);
        }else if(GetADDR2MUX(uinst)==2){ //PCoffset9
            addr2 = sext(CURRENT_LATCHES.IR & 0x1FF, 9);
        }else if(GetADDR2MUX(uinst)==3){ //PCoffset11
            addr2 = sext(CURRENT_LATCHES.IR & 0x7FF, 11);
        }else if(GetADDR2MUX(uinst)==4){ // -2 
            addr2 = -2;
        }else if(GetADDR2MUX(uinst)==5){ // +2
            addr2 = 2;
        }

        if(GetLSHF1(uinst)==1){ //LSHF addr2 if 
            addr2 = addr2 << 1;
        }

        g_marmux = addr1 + addr2;
    }

    // GATE PC -----------------------------------------------------------
    g_pc = CURRENT_LATCHES.PC;

    //GATE ALU -----------------------------------------------------------
    int sr1, sr2;
    int sr1out;
    int sr2out;
    int steer = (CURRENT_LATCHES.IR >> 5) & 0x01;
    if(GetSR1MUX(uinst)==0){ //11.9
        sr1 = (CURRENT_LATCHES.IR  >> 9) & 0x7;
    }else if(GetSR1MUX(uinst)==1){ //8.6
        sr1 = (CURRENT_LATCHES.IR >> 6) & 0x7;
    }else{
        sr1 = 6;
    }
    sr1out = CURRENT_LATCHES.REGS[sr1]; //get address

    if(steer == 1) //imm5
        sr2out = sext(CURRENT_LATCHES.IR & 0x1F, 5);
    else{
        sr2 = CURRENT_LATCHES.IR & 0x7;
        sr2out = CURRENT_LATCHES.REGS[sr2];
    }
    if(GetALUK(uinst)==0) //ADD
        g_alu = sr1out + sr2out;
    else if(GetALUK(uinst)==1) //AND
        g_alu = sr1out & sr2out;
    else if(GetALUK(uinst)==2) //XOR
        g_alu = sr1out ^ sr2out;
    else if(GetALUK(uinst)==3) //PASS A
        g_alu = sr1out;

    // GATE SHF -----------------------------------------------------------
    steer = (CURRENT_LATCHES.IR >> 4) & 0x3;
    int amount4 = CURRENT_LATCHES.IR & 0xF;
    int val = CURRENT_LATCHES.REGS[sr1];

    if(steer == 0){  //LSHF
        g_shf = val << amount4;
    }else if(steer == 1){ //RSHFL
        g_shf = (val & 0xFFFF) >> amount4;
    }else if(steer == 3){ //RSHFA
        g_shf = sext(val & 0xFFFF, 16) >> amount4;
    }

    //GATE MDR -----------------------------------------------------------
    if(GetDATA_SIZE(uinst)==1){  // word
        g_mdr = CURRENT_LATCHES.MDR;
    }else{  //byte
        if((CURRENT_LATCHES.MAR & 0x01)==1){ //odd, higher byte
            g_mdr = sext((CURRENT_LATCHES.MDR >> 8) & 0xFF, 8); //g_mdr = sext(MEMORY[g_pc >> 1][1], 8);
        }else{ //even lower byte
            g_mdr = sext(CURRENT_LATCHES.MDR & 0xFF, 8); //g_mdr = sext(MEMORY[g_pc >> 1][0], 8);
        }
    }
    g_marmux = Low16bits(g_marmux);
    g_pc = Low16bits(g_pc);
    g_alu = Low16bits(g_alu);
    g_shf = Low16bits(g_shf);
    g_mdr = Low16bits(g_mdr);
    g_psr = Low16bits(g_psr); // add new gates
    g_sp = Low16bits(g_sp);
    g_vect = Low16bits(g_vect);
}



void drive_bus() {

  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
   */       
    int* uinst = CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER];
    if(GetGATE_MARMUX(uinst)){
        BUS = g_marmux;
    }else if(GetGATE_PC(uinst)){
        BUS = g_pc;
    }else if(GetGATE_ALU(uinst)){
        BUS = g_alu;
    }else if(GetGATE_SHF(uinst)){
        BUS = g_shf;
    }else if(GetGATE_MDR(uinst)){
        BUS = g_mdr;
    }else if(GetGATE_SP(uinst)){
        BUS = g_sp;
    }else if(GetGATE_VECT(uinst)){
        BUS = g_vect;
    }else if(GetGATE_PSR(uinst)){
        BUS = g_psr;
    }else{
        BUS = 0;  //so not floating
    }
    BUS = Low16bits(BUS);
}


void latch_datapath_values() {

  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */       
 //check all the load signals
   int* uinst = CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER];

    // LD.USP ----------------------------------
    if(GetLD_USP(uinst)){
        NEXT_LATCHES.SAVED_USP = CURRENT_LATCHES.REGS[6]; //Saved_USP <- R6
    }

    // LD.SSP ----------------------------------
    if(GetLD_SSP(uinst)){
        NEXT_LATCHES.SSP = CURRENT_LATCHES.REGS[6]; //Saved_SSP <- R6
    }

    // LD.VECT ----------------------------------
    if(GetLD_VECT(uinst)){
        if(GetVECTMUX(uinst)){  //INTV
            NEXT_LATCHES.VECTOR = CURRENT_LATCHES.INTV;  //1 in state 36 for interrupts
        }else
            NEXT_LATCHES.VECTOR = CURRENT_LATCHES.EXCV;  //if exception detected in 18, it'll go to state 10 and load excv
    }

    // LD.PRIV ----------------------------------
    if (GetLD_PRIV(uinst)) {
        if (GetPRIVMUX(uinst)) {
            NEXT_LATCHES.PRIVILEGE = (BUS >> 15) & 0x1; // Restore PSR[15] from the BUS when RTI
        } else {
            NEXT_LATCHES.PRIVILEGE = 0; //supervisor mode to handle
        }
    }

    // LD.MAR ----------------------
    if(GetLD_MAR(uinst)){
        NEXT_LATCHES.MAR = Low16bits(BUS);
    }
    // LD.MDR -------------------------------
    if(GetLD_MDR(uinst)){
        if(GetMIO_EN(uinst)){ //read memory
            if(CURRENT_LATCHES.READY){
                int row = CURRENT_LATCHES.MAR >> 1;
                NEXT_LATCHES.MDR = (MEMORY[row][1] << 8) | MEMORY[row][0];
            }else{ 
                NEXT_LATCHES.MDR = CURRENT_LATCHES.MDR;
            }

        }else{
            if(GetDATA_SIZE(uinst)) // full word
                NEXT_LATCHES.MDR = Low16bits(BUS);
            else //lower 8 bits 
                NEXT_LATCHES.MDR = ((BUS & 0xFF) << 8) | (BUS & 0xFF);
        }
        NEXT_LATCHES.MDR = Low16bits(NEXT_LATCHES.MDR);
    }

    //LD.IR -----------------------------------
    if(GetLD_IR(uinst)){
        NEXT_LATCHES.IR = Low16bits(BUS);
    }

    //LD.BEN --------------------------------------------
    if(GetLD_BEN(uinst)){
        int ir11 = (CURRENT_LATCHES.IR >> 11) & 0x1;
        int ir10 = (CURRENT_LATCHES.IR >> 10) & 0x1;
        int ir9 = (CURRENT_LATCHES.IR >> 9) & 0x1;
        if((ir11 && CURRENT_LATCHES.N) || (ir10 && CURRENT_LATCHES.Z) || (ir9 && CURRENT_LATCHES.P))
            NEXT_LATCHES.BEN = 1; //only in state 32
        else
            NEXT_LATCHES.BEN = 0;
    }

    //LD.REG --------------------------------------------- change for R6
    if(GetLD_REG(uinst)){
        int dr;
        if(GetDRMUX(uinst)==0) //11.9
            dr = (CURRENT_LATCHES.IR >> 9) & 0x7;
        else if(GetDRMUX(uinst)==1) // r7
            dr = 7;
        else    
            dr = 6;
        NEXT_LATCHES.REGS[dr] = Low16bits(BUS);
    }

    //LD.CC ----------------------------------------------- update for new logic
    if(GetLD_CC(uinst)){
        if(GetPSRMUX(uinst)==0){ //regular logic
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.Z = 0;
            NEXT_LATCHES.P = 0;
            if(BUS==0)
                NEXT_LATCHES.Z = 1;
            else if((BUS >> 15) & 0x1)
                NEXT_LATCHES.N = 1;
            else
                NEXT_LATCHES.P = 1;  
        }else{ // restore after RTI
            NEXT_LATCHES.N = (BUS >> 2) & 0x1;
            NEXT_LATCHES.Z = (BUS >> 1) & 0x1;
            NEXT_LATCHES.P = BUS & 0x1;
        }
    }

    // LD.PC -----------------------------------------------
    if(GetLD_PC(uinst)){
        if(GetPCMUX(uinst)==0)  //PC
            NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
        else if(GetPCMUX(uinst)==1) // BUS
            NEXT_LATCHES.PC = Low16bits(BUS);
        else if(GetPCMUX(uinst)==2){  //ADDER
            int addr1 = 0;
            int addr2 = 0;
            // ADDR1
            if(GetADDR1MUX(uinst)==0) //PC
                addr1 = CURRENT_LATCHES.PC;
            else{ // BaseR = sr1out
                int reg;
                if(GetSR1MUX(uinst)==0)// IR[11.9]
                    reg = (CURRENT_LATCHES.IR >> 9) & 0x7;
                else if(GetSR1MUX(uinst)==1)//IR[8.6]
                    reg = (CURRENT_LATCHES.IR >> 6) & 0x7;
                else    // R6
                    reg = 6;
                addr1 = CURRENT_LATCHES.REGS[reg];  //get actual address within register
            }
            // ADDR2
            if(GetADDR2MUX(uinst)==0){  //ZERO
                addr2 = 0;
            }else if(GetADDR2MUX(uinst)==1){ //offset6
                addr2 = sext(CURRENT_LATCHES.IR & 0x3F, 6);
            }else if(GetADDR2MUX(uinst)==2){ //PCoffset9
                addr2 = sext(CURRENT_LATCHES.IR & 0x1FF, 9);
            }else if(GetADDR2MUX(uinst)==3){ //PCoffset11
                addr2 = sext(CURRENT_LATCHES.IR & 0x7FF, 11);
            }else if(GetADDR2MUX(uinst)==4){ // -2
                addr2 = -2;
            }else if(GetADDR2MUX(uinst)==5){ // +2
                addr2 = 2;
            }

            if(GetLSHF1(uinst)==1) //LSHF addr2 if 
                addr2 = addr2 << 1;

            NEXT_LATCHES.PC = addr1 + addr2;

        }else if(GetPCMUX(uinst)==3){  // PC - 2
            NEXT_LATCHES.PC = CURRENT_LATCHES.PC - 2;
        }
        NEXT_LATCHES.PC = Low16bits(NEXT_LATCHES.PC);
    }
}