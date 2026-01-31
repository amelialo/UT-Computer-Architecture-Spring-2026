/*
    Remove all unnecessary lines (including this one) 
    in this comment.
    REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS

    Name 1: Full name of the first partner 
    Name 2: Full name of the second partner
    UTEID 1: UT EID of the first partner
    UTEID 2: UT EID of the second partner
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

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
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
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
/* Purpose   : Simulate the LC-3b until HALTed                 */
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

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
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
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
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

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
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
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/


int sext(int value, int bits) {
  int sign_bit = 1 << (bits - 1); // msb mask based on number of bits
  if (value & sign_bit) {
    // if negative, sext to 32 bits
    return value | (0xFFFFFFFF << bits); 
  }
  return value; // otherwise it's positive
}

void setCC(int result) {
  // clear all condition codes first
  NEXT_LATCHES.N = 0;
  NEXT_LATCHES.Z = 0;
  NEXT_LATCHES.P = 0;

  result = Low16bits(result);

  if (result == 0) {
    NEXT_LATCHES.Z = 1;
  } 
  else if (result & 0x8000) {  //check 16th bit for 1
    NEXT_LATCHES.N = 1;
  } 
  else {
    NEXT_LATCHES.P = 1;
  }
}

void process_instruction(){
  /*  function: process_instruction
   
Process one instruction at a time
-Fetch one instruction
-Decode
-Execute
-Update NEXT_LATCHES
*/
 //fetch
int pc = CURRENT_LATCHES.PC; //current state of the processor; has the input data (current address)//pc>>1 eqivalent to pc/2 for 2byte addresses
int instr = (MEMORY[pc >> 1][1] << 8) | (MEMORY[pc >> 1][0]); //Little Endian -> OR the top/bottom 8 bits together for full instr (MEMORY is array of addresses)
instr = Low16bits(instr); //take care of the overflow

//decode
int opcode = (instr >> 12) & 0xF; //isolate the opcode by shifting and masking
NEXT_LATCHES.PC = Low16bits(pc + 2); //pc incremented for next instruction

//execute
//list of registers/immediates/labels - Components of instructions after opcode
int dr, sr1, sr2, baseR, imm5, nzp, PCoffset9, PCoffset11, offset6, amount4, trapvect8, val, address, bit4, bit5;
switch(opcode){
  case 0: //BR
    //isolate nzp
    nzp = (instr >> 9) & 0x7;
    //isolate PCoffset9;
    PCoffset9 = sext(instr & 0x1FF, 9); //sext 9 bits
    if (((nzp & 0x4) && CURRENT_LATCHES.N) || ((nzp & 0x2) && CURRENT_LATCHES.Z) || ((nzp & 0x1) && CURRENT_LATCHES.P)){
      //checks if any n z p bits are 1s
      //yes so branch is taken, need to update pc
      NEXT_LATCHES.PC = Low16bits(NEXT_LATCHES.PC + (PCoffset9 << 1)); //pc = nextpc +2*offset
    }
    break; //no branch NEXT_LATCHES.PC is just pc+2

  case 1: //ADD
    dr  = (instr >> 9) & 0x7; // right shift and get as 3 least sig bits
    sr1 = (instr >> 6) & 0x7; // right shift
    
    if (instr & 0x20) { // isolate steering bit, if 1/0
      imm5 = sext(instr & 0x1F, 5); 
      // ex) ADD R1, R2, #5
      NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] + imm5); // value in sr1 register and then add 5
    } else { // add registers
      sr2 = instr & 0x7;
      // ex) ADD R1, R2, R3
      NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] + CURRENT_LATCHES.REGS[sr2]); // value in sr1 + sr2
    }
    setCC(NEXT_LATCHES.REGS[dr]); //set condition code bits
    break;

  case 5: // AND
    dr  = (instr >> 9) & 0x7; // right shift and get as 3 least sig bits
    sr1 = (instr >> 6) & 0x7; // right shift
    
    if (instr & 0x20) { // isolate steering bit, if 1/0
      imm5 = sext(instr & 0x1F, 5); 
      // ex) AND R1, R2, #5
      NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] & imm5); // value in sr1 register and then add 5
    } else { // add registers
      sr2 = instr & 0x7;
      // ex) ADD R1, R2, R3
      NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] & CURRENT_LATCHES.REGS[sr2]); // value in sr1 + sr2
    }
    setCC(NEXT_LATCHES.REGS[dr]); //set condition code bits
    break;

  case 12: //JMP  or  RET
    baseR = (instr >> 6) & 0x7; //isolate baseR
    NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[baseR]); //PC <-- baseR
    break;
  
  case 4: // JSR/JSRR
    if(instr & 0x800){ //JSR
      PCoffset11 = sext(instr & 0x7FF, 11);
      NEXT_LATCHES.REGS[7] = NEXT_LATCHES.PC; // r7 <--  line after
      NEXT_LATCHES.PC = Low16bits(NEXT_LATCHES.PC + (PCoffset11 << 1)); //pc + offset*2
    }
    else{
      baseR = (instr >> 6) & 0x7;
      NEXT_LATCHES.REGS[7] = NEXT_LATCHES.PC; 
      NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[baseR]); 
    }
    break;

  case 2: // LDB  -  can be even or odd
    // DR = SEXT(mem[baseR + SEXT(boffset6)]) ; setcc()
    dr = (instr >> 9) & 0x7;
    baseR = (instr >> 6) & 0x7;
    offset6 = sext(instr & 0x3F, 6);
    //base + offset
    address = CURRENT_LATCHES.REGS[baseR] + offset6; //dont need to multiple by 2 bc reading bytes
    if (address & 1){
      val = MEMORY[address >> 1][1];  // divide address by 2 for row, grab high byte
    }
    else{
      val = MEMORY[address >> 1][0]; //low byte
    }
    NEXT_LATCHES.REGS[dr] = Low16bits(sext(val, 8)); // load into destination register
    setCC(NEXT_LATCHES.REGS[dr]);
    break; 

  case 6: // LDW
    // DR = MEM[BaseR + LSHF(SEXT(offset6),1)] ; setcc()
    dr = (instr >> 9) & 0x7;
    baseR = (instr >> 6) & 0x7;
    offset6 = sext(instr & 0x3F, 6);
    address = CURRENT_LATCHES.REGS[baseR] + (offset6 << 1);
    val = (MEMORY[address >> 1][1] << 8) | (MEMORY[address >> 1][0]); //combine two 8 bit numbers to 16 bit little endian
    NEXT_LATCHES.REGS[dr] = Low16bits(val);
    setCC(NEXT_LATCHES.REGS[dr]);
    break; 

  case 14: //LEA
    // DR = PC + LSHF(SEXT(PCoffset9),1);
    dr = (instr >> 9) & 0x7;
    PCoffset9 = sext(instr & 0x1FF, 9);
    NEXT_LATCHES.REGS[dr] = Low16bits(NEXT_LATCHES.PC + (PCoffset9 << 1));
    break;

  case 9:  //NOT or XOR. XOR 11111 (xor -1) is the same as NOT
    // NOT : DR = NOT(SR); setcc() - same as xor imm5 11111 - steering bit both 1
    dr = (instr >> 9) & 0x7;
    sr1 = (instr >> 6) & 0x7;
    if(instr &0x20){  // DR = SR1 XOR SEXT(imm5); setcc()
      imm5 = sext(instr & 0x1F, 5);
      NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] ^ imm5);

    }else{  //DR = SR1 XOR SR2
      sr2 = instr & 0x7;
      NEXT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] ^ CURRENT_LATCHES.REGS[sr2]); 
    }
    setCC(NEXT_LATCHES.REGS[dr]);
    break;

  case 13:  //SHF
    bit4 = (instr >> 4) & 0x01; 
    bit5 = (instr >> 5) & 0x01;
    dr = (instr >> 9) & 0x7;
    sr1 = (instr >> 6) & 0x7;
    val = CURRENT_LATCHES.REGS[sr1];
    amount4 = instr & 0xF;
    if (bit4 == 0){
      // DR = LSHF(SR, amount4)
      NEXT_LATCHES.REGS[dr] = Low16bits(val << amount4);
    }
    else{
      if(bit5 == 0){
        // DR = RSHF(SR, amount4, 0)
        NEXT_LATCHES.REGS[dr] = Low16bits(val >> amount4);
      }
      else{
        // DR = RSHF(SR, amount4, SR[15])  sr15 shifted into vacant
        NEXT_LATCHES.REGS[dr] = (Low16bits(sext(val, 16) >> amount4));
      }
    }
    setCC(NEXT_LATCHES.REGS[dr]);
    break;

  case 3: // STB
    // mem[BaseR + SEXT(boffset6)] = SR[7:0]
    sr1 = (instr >> 9) & 0x7;
    baseR = (instr >> 6) & 0x7;
    offset6 = sext(instr & 0x3F, 6);
    address = Low16bits(CURRENT_LATCHES.REGS[baseR] + offset6); //byte, dont multiple by 2
    val = CURRENT_LATCHES.REGS[sr1] & 0xFF; //get a byte
    if(address & 0x1){ //odd address
      MEMORY[address >> 1][1] = val;
    }else{  // even address
      MEMORY[address >> 1][0] = val; 
    }
    break;

  case 7:  // STW 
    // MEM[BaseR + LSHF(SEXT(offset6), 1)]
    sr1 = (instr >> 9) & 0x7;
    baseR = (instr >> 6) & 0x7;
    offset6 = sext(instr & 0x3F, 6);
    address = Low16bits(CURRENT_LATCHES.REGS[baseR] + (offset6<<1)); //word, multiple by 2
    val = CURRENT_LATCHES.REGS[sr1];
    MEMORY[address >> 1][1] = (val >> 8) & 0xFF;
    MEMORY[address >> 1][0] = val & 0xFF;
    break;

  case 15: //TRAP
    // R7 = PC+;  PC = MEM[LSHF(ZEXT(trapvect8), 1)]
    trapvect8 = instr & 0xFF; // 16 bits only, technically zero extended past
    NEXT_LATCHES.REGS[7] = NEXT_LATCHES.PC;
    address = (trapvect8 << 1); 
    NEXT_LATCHES.PC = Low16bits((MEMORY[address >> 1][1] << 8) | MEMORY[address >> 1][0]);
    break;

  default: 
    exit(1);
  }
}