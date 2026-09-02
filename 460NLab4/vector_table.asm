.ORIG 0x0200
.FILL x0000  ; placeholder so that x01 will map to x1200 because its at loc x0202
.FILL x1200  ; x0202  vect <- x0200 + (x01)*2 = x0202 = interrupt
.FILL x1600  ; x0204  vect <- x0200 + (x02)*2 = x0204 = protection exception
.FILL x1A00  ; x0206  vect <- x0200 + (x03)*2 = x0206 = unaligned exception
.FILL x1C00  ; x0208  vect <- x0200 + (x03)*2 = x0206 = unknown opcode exception
.END