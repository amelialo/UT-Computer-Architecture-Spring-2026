#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */
#define MAX_LINE_LENGTH 255
enum{DONE, OK, EMPTY_LINE};


int toNum( char * pStr ){
   char * t_ptr;
   char * orig_pStr;
   int t_length,k;
   int lNum, lNeg = 0;
   long int lNumLong;

   orig_pStr = pStr;
   if( *pStr == '#' ){				/* decimal */ 
     pStr++;
     if( *pStr == '-' ){				/* dec is negative */
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++)  //makes sure its a string of only digits
     {
       if (!isdigit(*t_ptr)){
	    printf("Error: invalid decimal operand, %s\n",orig_pStr);
	    exit(4);
       }
       t_ptr++;
     }
     lNum = atoi(pStr); // convert ascii to integer
     if (lNeg)
       lNum = -lNum;
     return lNum;
   }
   else if( *pStr == 'x' || *pStr == 'X' || (*pStr == '0' && (*(pStr+1) == 'x' || *(pStr+1) == 'X'))) //if 0th loc is 0, 1st loc must be x/X
   {
    if(*pStr == '0'){
        pStr+=2;  //move ptr twice past 0 and x
    } else{
        pStr++; //move ptr past x
    }
     if( *pStr == '-' ){				/* hex is negative */
       lNeg = 1;
       pStr++;
     }
     t_ptr = pStr;
     t_length = strlen(t_ptr);
     for(k=0;k < t_length;k++){
       if (!isxdigit(*t_ptr))
       {
	 printf("Error: invalid hex operand, %s\n",orig_pStr);
	 exit(4);
       }
       t_ptr++;
     }
     lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
     lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
     if( lNeg )
       lNum = -lNum;
     return lNum;
   }
   else
   {
	printf( "Error: invalid operand, %s\n", orig_pStr);
	exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
   }
}

int isOpcode(char* pOpcode){
    char* listOp[] = {"add", "and", "br", "brn", "brz", "brp", "brnz", "brnp", "brzp", "brnzp", "jmp", "jsr", "jsrr", 
        "ldb", "ldw", "lea", "not", "ret", "rti", "lshf", "rshfl", "rshfa", "stb", "stw", "trap", "xor", "nop", "halt"};
    for(int i=0; i<28; i++){
        if(strcmp(pOpcode, listOp[i])==0){
            return 0;
        }
    }
    return -1;
}

int readAndParse(FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4) {
    char * lRet, * lPtr;
    int i;

    if (!fgets(pLine, MAX_LINE_LENGTH, pInfile))
        return (DONE);//if file is empty when fgets fails(EOF) then its done

    /* convert entire line to lowercase */
    for (i = 0; i < strlen(pLine); i++)
        pLine[i] = tolower(pLine[i]);

    /* initialize all pointers to the end of the string (empty) */
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

    /* ignore the comments */
    lPtr = pLine;
    while (*lPtr != ';' && *lPtr != '\0' && *lPtr != '\n') //scans string until it hits a ; or a \n
        lPtr++;

    *lPtr = '\0'; // so terminates the line at the start of the comment 

    /* first pass the line to break it up based on \t, \n, commas and spaces to find 1st word */
    if (!(lPtr = strtok(pLine, "\t\n ,")))
        return (EMPTY_LINE);

    /* found a label (not opcode or . pseudo-op) */
    if (isOpcode(lPtr) == -1 && lPtr[0] != '.') {
        *pLabel = lPtr; //assigns label pointer to current address
        if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK); //get next word on same line, if no more then it's just label, return OK
    }

    *pOpcode = lPtr; // first opcode is stored to opcode string 

    if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK); //get next word, either 1st argument, or no more (eg. RET) return OK 

    *pArg1 = lPtr; //store current pointer to arg 1, a register

    if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK); //same thing for rest, only up to 4 args

    *pArg2 = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);

    *pArg3 = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);

    *pArg4 = lPtr;

    return (OK);
}


typedef struct Entry{  //struct that has the label name and address
    char label[13];
    int address;
}Entry;

Entry SymbolTable[255]; //array of structs make the symbol table
int numSym = 0;

FILE* infile = NULL;
FILE* outfile = NULL;


int getSymAddy(char* labelName){
    for(int i = 0; i < numSym; i++) {
        if(strcmp(SymbolTable[i].label, labelName) == 0) {
            return SymbolTable[i].address; 
        }
    }
    // if loop finishes, label wasnt found
    printf("Error: Undefined label %s\n", labelName);
    exit(1);
}

//**************HELPER FUNCTIONS*****************//
int add_and_xor(int opCode, char* lArg1, char* lArg2, char* lArg3){
      int instr = 0;
      instr |= (opCode << 12);
      //destination Register
      int dReg = atoi(lArg1 + 1);
      instr |= (dReg << 9);
      //source register 1
      int sReg1 = atoi(lArg2 + 1);
      instr |= (sReg1 << 6);
      //source register 2 (may be register or immediate)
      //IF register
      if (lArg3[0] == 'r' || lArg3[0] == 'R'){
        int sReg2 = atoi(lArg3 + 1);
        instr |= (sReg2 & 0x7); //keep the last 3 bits
      }else{
        //IF imm
        instr |= (1<<5); //this is the indicator for immmediate
        int imm = toNum(lArg3);
        //checking if immediate is in range
        if (imm > 15 || imm < -16){
          printf("Error: Invalid Immediate Value\n");
          exit(3);
        }
        instr |= (imm & 0x1F);
      }
        return instr;
}

int offset_base_spec(int opCode, int num, char* lArg1, char* lArg2, char* lArg3){ //num 0, 1, 2 are for 000 in Reg1 while num 4 means its input & num 3 for 1s in offset
      int instr = 0;
      instr |= (opCode << 12);
      //dRegister/sRegister
      if (num == 0 ||num == 1 || num == 2){ //0: jmp, 1: jsrr, 2: ret, 3: not
        instr |= (0<<9);
        instr |= (0<<10);
        instr |= (0<<11);
      }else{
        int Reg1 = atoi(lArg1 + 1);
        instr |= (Reg1 << 9);
      }
      //base register
      if (num == 2){ 
        instr |= (1<<6);
        instr |= (1<<7);
        instr |= (1<<8);
      }else{
      int Reg2 = atoi(lArg2 + 1);
      instr |= (Reg2 << 6);
      }
    //offset/last 6 bits
      if (num == 0 || num == 1 || num == 2){//jmp, jsrr, ret
        instr = (instr & 0xFFC0);  //make last six bits 0
      }else if (num == 3){
        instr = (instr | 0x3F); // isolate offset6
      }else{
           int offset = toNum(lArg3);
        //checking if offset is in range
        if (offset > 31 || offset < -32){
          printf("Error: Invalid Offset Value\n");
          exit(3);
        }
        instr |= (offset & 0x3F);
      }
      return instr;
   }

int lshf_rshfl_rshfa(int num, char* lArg1, char* lArg2, char* lArg3){
    int instr = 0;
    instr |= (0xD << 12);
    //destination Register
    int dReg = atoi(lArg1 + 1);
    instr |= (dReg << 9);
    //source register 1
    int sReg1 = atoi(lArg2 + 1);
    instr |= (sReg1 << 6);
//type indicator
    if (num == 0){//lshf
    instr |= (0<<4);
    instr |= (0<<5);
    }else if (num == 1){//rshfl
    instr |= (1<<4);
    instr |= (0<<5);
    }else if (num == 2){//rshfa
    instr |= (1<<4);
    instr |= (1<<5);
    }
    //amount 4
    int imm = toNum(lArg3);
    //checking if immediate is in range
    if (imm > 15 || imm < 0){
        printf("Error: Invalid Immediate Value\n");
        exit(3);
    }
    instr |= (imm & 0xF);
    return instr;
}

int main(int argc, char* argv[]) {
    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
    int lRet;
 
    int pc = -1;

    if(argc < 3){
        printf("Error: Missing arguments.\n");
        exit(1);
    }
    char *prgName   = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;

    prgName   = argv[0];  //command
    iFileName = argv[1];  //arguments
    oFileName = argv[2];

    printf("program name = '%s'\n", prgName);
    printf("input file name = '%s'\n", iFileName);
    printf("output file name = '%s'\n", oFileName);

    /* open the source file */
    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");
		 
    if (!infile) {
        printf("Error: Cannot open file %s\n", argv[1]);
        exit(4);
	}
    if (!outfile) {
        printf("Error: Cannot open file %s\n", argv[2]);
        exit(4);
    }

    //--------FIRST PASS-----------//

    do{
        lRet = readAndParse(infile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
		if( lRet != DONE && lRet != EMPTY_LINE ){ //do if lRet is OK 
			if(strcmp(lOpcode, ".orig") == 0){
                pc = toNum(lArg1); //ex .ORIG x3000, arg1 is always the hex# after .orig, change from string to num
                if (pc % 2 != 0) { //if address is a odd number / not aligned
                    printf("Error: Program not aligned\n");
                    exit(4);
                }
                continue; //.orig dont take up space, so skip incrementing by 2 bytes
            }
		

            if ( lLabel != NULL && strcmp(lLabel, "") != 0 ) { //make sure label is not an empty or invalid string
                for(int i=0; i<numSym; i++){  //check for symbol duplicate error in table
                    if(strcmp(lLabel, SymbolTable[i].label) == 0){
                        printf("Error: Symbol Duplicate.\n");
                        exit(4);
                    }
                }
                strcpy(SymbolTable[numSym].label, lLabel); //not duplicate, add it to the symbol table
                SymbolTable[numSym].address = pc;
                numSym++;
            }
            if (strcmp(lOpcode, ".end") == 0) {
                break;
            }
            else if (strcmp(lOpcode, ".fill") == 0) { //reserves 2 bytes
                pc += 2; 
            }
            else if (strcmp(lOpcode, ".blkw") == 0) { //e.g. .BLKW #10, 10 would be arg1 num words, each word is 2 bytes
                pc += toNum(lArg1)*2; 
            }
            else if (strcmp(lOpcode, ".stringz") == 0) {
                int realLen = strlen(lArg1) -2 + 1; //length - quotes + null term
                if(realLen%2!=0){ //not aligned because odd number of bytes
                    realLen+=1;
                }
                pc+=realLen;
            }
            else if (isOpcode(lOpcode) == 0) {
                 pc += 2; // all other instructions are 2 bytes
            }
            else { //if none of the above, then it's invalid
                 printf("Error: invalid opcode %s\n", lOpcode);
                 exit(2);
            }
        }

	   } while( lRet != DONE );  //until read end of file

    rewind(infile);
    //-------------------SECOND PASS-----------------//
    pc = -1; //reset pc
    lRet = OK;
    do{
         lRet = readAndParse(infile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
		if( lRet != DONE && lRet != EMPTY_LINE ){ 
			if(strcmp(lOpcode, ".orig") == 0){
                pc = toNum(lArg1); 
                fprintf(outfile, "0x%.4X\n", pc); //write to file
                continue; 
            }
            if (strcmp(lOpcode, ".end") == 0) {
                break;
            }
            else if (strcmp(lOpcode, ".fill") == 0) {
                int val = 0;
                //if .FILL x5, .FILL #5, .FILL -5, .FILL 5
                if(lArg1[0]=='x' || lArg1[0]=='#' || isdigit(lArg1[0]) || lArg1[0]=='-'){
                    val = toNum(lArg1);
                }
                else{ // .FILL LABEL, look for the label address from symbol table
                    val = getSymAddy(lArg1);
                }
                fprintf(outfile, "0x%.4X\n", val & 0xFFFF); // the 32 bit val anded with 16bit ffff
                pc += 2; 
            }
            else if (strcmp(lOpcode, ".blkw") == 0) {
                for(int i=0; i< toNum(lArg1); i++){
                    fprintf(outfile, "0x0000\n"); //print x0000 for however many spaces arg1 specifies
                }
                pc += toNum(lArg1)*2; 
            }
            else if (strcmp(lOpcode, ".stringz") == 0) { //big endian
                
                for(int i=1; i<strlen(lArg1)-1; i+=2){ //exclude "", i+=2 to store xAB then xCD for endianess
                    int char1 = lArg1[i];
                    int char2 = 0;
                    if(i+1 < strlen(lArg1)-1){ //if another character exists
                        char2 = lArg1[i+1];
                    }
                    int together = (char1<<8) | (char2 & 0xFF); //left shift char1's 8 bits ORed with rightmost 8 bits of char2
                    fprintf(outfile, "0x%.4x\n", together);
                }
                int bytesNeeded = strlen(lArg1)-1;
                if( bytesNeeded% 2 != 0){
                    bytesNeeded++;
                }
                pc+=bytesNeeded;
            }

            //CHECK FOR OPCODE INSTRUCTIONS
            else if (strcmp(lOpcode, "add") == 0) { //0001 DR(3), SR1(3), 0 00SR2/ 1 imm5
                int instr = add_and_xor(0x1, lArg1, lArg2, lArg3);
                fprintf(outfile, "0x%.4X\n", instr);
                pc +=2;

            }
            else if(strcmp(lOpcode, "and") == 0){
                int instr = add_and_xor(0x5, lArg1, lArg2, lArg3);
                fprintf(outfile, "0x%.4X\n", instr);
                pc +=2;
            }
            else if(strcmp(lOpcode, "br") == 0 || strcmp(lOpcode, "brn") == 0 || strcmp(lOpcode, "brz") == 0 || 
                    strcmp(lOpcode, "brp") == 0 || strcmp(lOpcode, "brnz") == 0 || strcmp(lOpcode, "brnp") == 0 || 
                    strcmp(lOpcode, "brzp") == 0 || strcmp(lOpcode, "brnzp") == 0){
                int instr = 0; // 0000 000 000000000
                if (strcmp(lOpcode, "br") == 0) { //same as brnzp
                    instr |= (1<<11) | (1<<10) | (1<<9); // 0000 111 000000000
                }else{
                    if(strchr(lOpcode, 'n')){ // 0000 100 000000000
                        instr |= (1<<11);
                    }
                    if(strchr(lOpcode, 'z')){ // 0000 110 000000000
                        instr |= (1<<10);
                    }
                    if(strchr(lOpcode, 'p')){ // 0000 111 000000000
                        instr |= (1<<9);
                    }
                }
                //will always be br___ label
                int val = getSymAddy(lArg1);
                int offset = (val - (pc+2)) >> 1; // shift right one bc lsb will always be 0 (even) so allows more bits
                if (offset > 255 || offset < -256) {
                    printf("Error: branch too far");
                    exit(4);
                }
                instr |= (offset & 0x1FF); //mask it to 9 bits  /// 0000 nzp offset
                fprintf(outfile, "0x%.4X\n", instr);
                pc += 2;
            }
            else if(strcmp(lOpcode, "jmp") == 0){
                int instr = offset_base_spec(0xC, 0, NULL, lArg1, NULL);
                fprintf(outfile, "0x%.4X\n", instr);
                pc +=2;
            }
            else if(strcmp(lOpcode, "jsr") == 0){
                int instr = 0; //0000 000000000000
                instr |= (0x4<<12); //0100 000000000000
                instr |= (1<<11); //0100 1 00000000000
                int val = getSymAddy(lArg1); //always JSR label
                int offset =  (val - (pc+2)) >> 1;
                if (offset > 1023 || offset < -1024) {
                    printf("Error: JSR target too far\n");
                    exit(4);
                }
                instr |= (offset & 0x7FF); //masked with 11 lsb
                fprintf(outfile, "0x%.4X\n", instr);
                pc+=2;
            }
            else if(strcmp(lOpcode, "jsrr") == 0){
                int instr = offset_base_spec(0x4, 1, NULL, lArg1, NULL);
                fprintf(outfile, "0x%.4X\n", instr);
                pc +=2;
            }
            else if(strcmp(lOpcode, "ldb") == 0){
                int instr = offset_base_spec(0x2, 4, lArg1, lArg2, lArg3);
                fprintf(outfile, "0x%.4X\n", instr);
                pc +=2;
            }\
            else if(strcmp(lOpcode, "ldw") == 0){
                int instr = offset_base_spec(0x6, 4, lArg1, lArg2, lArg3);
                fprintf(outfile, "0x%.4X\n", instr);
                pc +=2;
            }
            else if(strcmp(lOpcode, "lea") == 0){
                int instr = 0;
                instr |= (0x7 << 13);
                int dr = atoi(lArg1 + 1);
                instr |= (dr << 9);
                int val = getSymAddy(lArg2);

                int offset = (val - (pc+2)) >> 1;
                if (offset > 255 || offset < -256) {
                    printf("Error: branch too far");
                    exit(4);
                }
                instr |= (offset & 0x1FF);
                fprintf(outfile, "0x%.4X\n", instr);
                pc += 2;
            }
            else if(strcmp(lOpcode, "not") == 0){
                int instr = offset_base_spec(0x9, 3, lArg1, lArg2, lArg3);
                fprintf(outfile, "0x%.4X\n", instr);
                pc +=2;
            }
            else if(strcmp(lOpcode, "ret") == 0){
                int instr = offset_base_spec(0xC, 2, lArg1, lArg2, lArg3);
                fprintf(outfile, "0x%.4X\n", instr);
                pc +=2;
            }
            else if(strcmp(lOpcode, "rti") == 0){
                fprintf(outfile, "0x8000\n");
                pc+=2;
            }
            else if(strcmp(lOpcode, "lshf") == 0){
                int instr = lshf_rshfl_rshfa(0, lArg1, lArg2, lArg3);
                fprintf(outfile, "0x%.4X\n", instr);
                pc +=2;
            }
            else if(strcmp(lOpcode, "rshfl") == 0){
                int instr = lshf_rshfl_rshfa(1, lArg1, lArg2, lArg3);
                fprintf(outfile, "0x%.4X\n", instr);
                pc +=2;
            }
            else if(strcmp(lOpcode, "rshfa") == 0){
                int instr = lshf_rshfl_rshfa(2, lArg1, lArg2, lArg3);
                fprintf(outfile, "0x%.4X\n", instr);
                pc +=2;
            }
            else if(strcmp(lOpcode, "stb") == 0){
                int instr = offset_base_spec(0x3, 4, lArg1, lArg2, lArg3);
                fprintf(outfile, "0x%.4X\n", instr);
                pc +=2;
            }
            else if(strcmp(lOpcode, "stw") == 0){
                int instr = offset_base_spec(0x7, 4, lArg1, lArg2, lArg3);
                fprintf(outfile, "0x%.4X\n", instr);
                pc +=2;
            }
            else if(strcmp(lOpcode, "trap") == 0){
                int instr = 0;
                instr |= (0xF << 12);
                int vector = toNum(lArg1);
                if (vector > 255 || vector < 0) {
                    printf("Error: Trap vector out of bounds\n");
                    exit(3);
                }
                instr |= (vector & 0xFF); //last 8 bytes
                fprintf(outfile, "0x%.4X\n", instr);
                pc += 2;
            }
            else if (strcmp(lOpcode, "xor") == 0) { //0001 DR(3), SR1(3), 0 00SR2/ 1 imm5
                int instr = add_and_xor(0x9, lArg1, lArg2, lArg3);
                fprintf(outfile, "0x%.4X\n", instr);
                pc +=2;
            }
            else if(strcmp(lOpcode, "nop") == 0){
                fprintf(outfile, "0x0000\n");
                pc += 2;
            }
            else if(strcmp(lOpcode, "halt") == 0){ //trap x25
                fprintf(outfile, "0xF025\n");
                pc += 2;
            }
            else { //if none of the above, then it's invalid
                 printf("Error: invalid opcode %s\n", lOpcode);
                 exit(2);
            }
        }

    }while( lRet != DONE );

    fclose(infile);
    fclose(outfile);
}
