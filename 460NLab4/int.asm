.ORIG 0x1200
ADD R6, R6, #-2  ;R6<-R6-2
STW R0, R6, #0   ; push R0 to SP  m[R6] = R0
ADD R6, R6, #-2  ;R6<-R6-2
STW R1, R6, #0   ;push R1 to SP
LEA R0, LOC4000
LDW R0, R0, #0  ; R0 = x4000
LDW R1, R0, #0   ; R1 = m[x4000]
ADD R1, R1, #1  ;m[x4000] increments by 1
STW R1, R0, #0   ; m[x4000] = R1
LDW R1, R6, #0  ;pop old r1 first
ADD R6, R6, #2
LDW R0, R6, #0  ; pop old r0 second
ADD R6, R6, #2

RTI

HALT
LOC4000 .FILL x4000
LOCC017 .FILL xC017

.END