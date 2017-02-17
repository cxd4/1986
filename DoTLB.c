//DoTLB.c
//thanks to anarko for the docs!

/*
-----------------------------------------------------------------
| TLBP      | Translation Lookaside Buffer Probe                |
|-----------|---------------------------------------------------|
|  010000   |CO|      0000 0000 0000 0000 000       | 001000 (8)|
------6------1-------------------19-----------------------6------
 Format:  TLBP
 Purpose: The Index register is loaded with the address of the TLB entry
          whose contents match the contents of the EntryHi register. If no
          TLB entry matches, the high-order bit of the Index register is set.
          The architecture does not specify the operation of memory references
          associated with the instruction immediately after a TLBP instruction,
          nor is the operation specified if more than one TLB entry matches.
*/
void tlbp() {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| TLBR      | Translation Lookaside Buffer Read                 |
|-----------|---------------------------------------------------|
|  010000   |CO|      0000 0000 0000 0000 000       | 000001 (1)|
------6------1-------------------19-----------------------6------
 Format:  TLBR
 Purpose: The G bit (which controls ASID matching) read from the TLB is
          written into both of the EntryLo0 and EntryLo1 registers.
          The EntryHi and EntryLo registers are loaded with the contents of
          the TLB entry pointed at by the contents of the TLB Index register.
          The operation is invalid (and the results are unspecified) if the
          contents of the TLB Index register are greater than the number of
          TLB entries in the processor.
*/
void tlbr() {
#ifdef _DEBUG
#endif
}


/*
-----------------------------------------------------------------
| TLBWI     | Translation Lookaside Buffer Write Index          |
|-----------|---------------------------------------------------|
|  010000   |CO|      0000 0000 0000 0000 000       | 000010 (2)|
------6------1-------------------19-----------------------6------
 Format:  TLBWI
 Purpose: The G bit of the TLB is written with the logical AND of the G bits
          in the EntryLo0 and EntryLo1 registers. The TLB entry pointed at by
          the contents of the TLB Index register is loaded with the contents
          of the EntryHi and EntryLo registers. The operation is invalid (and
          the results are unspecified) if the contents of the TLB Index
          register are greater than the number of TLB entries in the processor.
*/
void tlbwi() {
#ifdef _DEBUG
#endif
}

/*
-----------------------------------------------------------------
| TLBWR     | Translation Lookaside Buffer Write Random         |
|-----------|---------------------------------------------------|
|  010000   |CO|      0000 0000 0000 0000 000       | 000110 (6)|
------6------1-------------------19-----------------------6------
 Format:  TLBWR
 Purpose: The G bit of the TLB is written with the logical AND of the G bits
          in the EntryLo0 and EntryLo1 registers. The TLB entry pointed at by
          the contents of the TLB Random register is loaded with the contents
          of the EntryHi and EntryLo registers
*/
void tlbwr() {
#ifdef _DEBUG
#endif
}


/*
-----------------------------------------------------------------
| ERET      | Return from Exception                             |
|-----------|---------------------------------------------------|
|  010000   |CO|      0000 0000 0000 0000 000       |011000 (24)|
------6------1-------------------19-----------------------6------
 Format:  ERET
 Purpose: ERET is the R4300 instruction for returning from an interrupt,
          exception, or error trap. Unlike a branch or jump instruction,
          ERET does not execute the next instruction.
*/
void eret() {
#ifdef _DEBUG
#endif
}

