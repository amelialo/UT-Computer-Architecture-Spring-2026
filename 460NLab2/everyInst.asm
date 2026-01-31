.ORIG x3000

ADD R1, R0, #10    ; R1 = 10 (x000A)
ADD R2, R1, #-5    ; R2 = 5  (x0005)
AND R2, R2, #1     ; R2 = 5 AND 1 = 1 (x0001)
NOT R3, R0         ; R3 = NOT(0) = -1 (xFFFF)

LSHF  R4, R2, #4   ; R4 = 1 << 4 = 16 (x0010)
RSHFL R5, R3, #4   ; R5 = xFFFF >> 4 = x0FFF
RSHFA R6, R3, #4   ; R6 = xFFFF >> 4 = xFFFF

LEA R0, DATA_VAL   ; R0 = x3026
LDW R1, R0, #0     ; R1 = x1234
STW R1, R0, #2     ; Store x1234 into R0+2

LDB R2, R0, #0     ; R2 = Load Byte from x1234 (Low byte) -> x34
STB R2, R0, #5     ; Store byte x34 into BYTE_SLOT

AND R0, R0, #0     ; Clear R0 and sets Z condition code
BRz SKIP_ADD       ; Branch if zero
ADD R1, R1, #1     ; should be skipped

SKIP_ADD JSR SUBROUTINE     ; Jump to subroutine
HALT             

SUBROUTINE ADD R7, R7, #1     ; Modify R7 just to prove we were here
RET                ; Return (JMP R7)


DATA_VAL   .FILL x1234
EMPTY_SLOT .FILL x0000
BYTE_SLOT  .FILL x0000
.END