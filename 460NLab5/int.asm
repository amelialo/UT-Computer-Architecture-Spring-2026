.ORIG x1200
ADD R6, R6, #-2    ;push R0
STW R0, R6, #0    
ADD R6, R6, #-2    ;psuh R1
STW R1, R6, #0
ADD R6, R6, #-2    ; push R2
STW R2, R6, #0
ADD R6, R6, #-2    ; push R3
STW R3, R6, #0

LEA R0, PAGETABLE 
LDW R0, R0, #0   ; R0=page table starting address x1000
LEA R1, NUMPAGES  
LDW R1, R1, #0   ; R1 = 128 pages counter
LEA R2, MASK    
LDW R2, R2, #0  ; mask to clear reference bit 
LOOP LDW R3, R0, #0    ; R3 = pte
AND R3, R3, R2    ; R bit=0run 
STW R3, R0, #0   ; store pte back
ADD R0, R0, #2   ; next pte
ADD R1, R1, #-1   ; dec counter
BRP LOOP 

LDW R3, R6, #0    ;pop R3
ADD R6, R6, #2
LDW R2, R6, #0    ;pop R2
ADD R6, R6, #2
LDW R1, R6, #0    ;pop R1
ADD R6, R6, #2
LDW R0, R6, #0    ;pop R0
ADD R6, R6, #2

RTI   

PAGETABLE  .FILL x1000
NUMPAGES  .FILL x0080
MASK  .FILL xFFFE
.END