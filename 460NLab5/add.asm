.ORIG x3000

LEA R0, LOCC000
LDW R0, R0, #0  ; R0=xC000
AND R1, R1, #0  ; R1=sum

AND R2, R2, #0
ADD R2, R2, #15
ADD R2, R2, #5  ;count = 20

LOOP LDB R3, R0, #0  ;load first byte at C000
ADD R1, R1, R3  ;add to sum
ADD R0, R0, #1  ;next byte
ADD R2, R2, #-1   ; counter-1
BRp LOOP

LEA R4, LOCC014
LDW R4, R4, #0   ;R4 = xC014
STW R1, R4, #0   ;m[C014] = sum

JMP R1 

LOCC000 .FILL xC000
LOCC014 .FILL xC014
LOCC017 .FILL xC017

.END