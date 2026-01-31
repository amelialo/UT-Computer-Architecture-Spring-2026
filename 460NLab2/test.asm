.ORIG x3000       ; Program starts at x3000
ADD R1, R1, #5    ; R1 should become 5
ADD R2, R1, #2    ; R2 should become 7 (5 + 2)
HALT              ; TRAP x25 (Stops simulator)
.END