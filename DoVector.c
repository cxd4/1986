//DoVector.c
//Thanks to anarko for the docs!

/*
****************************************************************************
** Vector Load and Store Instructions                                     **
****************************************************************************
*/

/*
  +-----------+---------------------------------------------------+
  | LBV       | Load byte to vector                               |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00000  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  LBV $v<dest>[del], offset(base)
*/
void lbv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | LSV       | Load short (halfword) to vector                   |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00001  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  LSV $v<dest>[del], offset(base)
*/
void lsv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | LLV       | Load long (word) to vector                        |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00010  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  LLV $v<dest>[del], offset(base)
*/
void llv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | LDV       | Load double to vector                             |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00011  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  LDV $v<dest>[del], offset(base)
*/
void ldv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | LQV       | Load quadword to vector                           |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00100  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  LQV $v<dest>[del], offset(base)
*/
void lqv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | LRV       | Load rest to vector                               |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00101  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  LRV $v<dest>[del], offset(base)
*/
void lrv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | LPV       | Load packed to vector                             |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00110  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  LPV $v<dest>[del], offset(base)
*/
void lpv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | LUV       | Load unpacked to vector                           |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00111  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  LUV $v<dest>[del], offset(base)
*/
void luv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | LHV       | Load half to vector                               |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  01000  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  LHV $v<dest>[del], offset(base)
*/
void lhv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | LFV       | Load fourth to vector                             |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  01001  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  LFV $v<dest>[del], offset(base)
*/
void lfv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | LWV       | Load wrap to vector                               |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  01010  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  LWV $v<dest>[del], offset(base)
*/
void lwv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | LTV       | Load transpose to vector                          |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  01011  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  LTV $v<dest>[del], offset(base)
*/
void ltv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | SBV       | Store byte from vector                            |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00000  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  SBV $v<dest>[del], offset(base)
*/
void sbv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | SSV       | Store short (halfword) from vector                |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00001  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  SSV $v<dest>[del], offset(base)
*/
void ssv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | SLV       | Store long (word) from vector                     |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00010  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  SLV $v<dest>[del], offset(base)
*/
void slv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | SDV       | Store double (doubleword) from vector             |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00011  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  SDV $v<dest>[del], offset(base)
*/
void sdv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | SQV       | Store quad (quadword) from vector                 |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00100  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  SQV $v<dest>[del], offset(base)
*/
void sqv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | SRV       | Store rest from vector                            |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00101  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  SRV $v<dest>[del], offset(base)
*/
void srv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | SPV       | Store packed from vector                          |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00110  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  SPV $v<dest>[del], offset(base)
*/
void spv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | SUV       | Store unpacked from vector                        |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  00111  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  SUV $v<dest>[del], offset(base)
*/
void suv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | SHV       | Store half from vector                            |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  01000  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  SHV $v<dest>[del], offset(base)
*/
void shv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | SFV       | Store fourth from vector                          |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  01001  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  SFV $v<dest>[del], offset(base)
*/
void sfv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | SWV       | Store wrap from vector                            |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  01010  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  SWV $v<dest>[del], offset(base)
*/
void swv() {
#ifdef _DEBUG
#endif
}

/*
  +-----------+---------------------------------------------------+
  | STV       | Store transpose from vector                       |
  +-----------+---------+---------+---------+-------+-+-----------+
  |  110010   |  base   |  dest   |  01011  |  del  |0|   offset  |
  +-----6-----+----5----+----5----+----5----+---4---+1+-----6-----+
  Format:  STV $v<dest>[del], offset(base)
*/
void stv() {
#ifdef _DEBUG
#endif
}

/*
****************************************************************************
** Vector instructions                                                    **
****************************************************************************
*/
/*
  -----------------------------------------------------------------
  | VMULF     | Vector (Frac) Multiply                            |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  000000   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VMULF $v<dest>, $v<s1>, $v<s2>[el]
*/
void vmulf() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VMULU     | Vector (Unsigned Frac) Multiply                   |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  000001   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VMULU $v<dest>, $v<s1>, $v<s2>[el]
*/
void vmulu() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VRNDP     | Vector DCT Round (+)                              |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  000010   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VRNDP $v<dest>, $v<s1>, $v<s2>[el]
*/
void vrndp() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VMULQ     | Vector (Integer) Multiply                         |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  000011   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VMULQ $v<dest>, $v<s1>, $v<s2>[el]
*/
void vmulq() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VMUDL     | Vector low multiply                               |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  000100   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VMUDL $v<dest>, $v<s1>, $v<s2>[el]
*/
void vmudl() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VMUDM     | Vector mid-m multiply                             |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  000101   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VMUDM $v<dest>, $v<s1>, $v<s2>[el]
*/
void vmudm() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VMUDN     | Vector mid-n multiply                             |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  000110   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VMUDN $v<dest>, $v<s1>, $v<s2>[el]
*/
void vmudn() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VMUDH     | Vector high multiply                              |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  000111   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VMUDH $v<dest>, $v<s1>, $v<s2>[el]
*/
void vmudh() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VMACF     | Vector (Frac) Multiply Accumulate                 |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  001000   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VMACF $v<dest>, $v<s1>, $v<s2>[el]
*/
void vmacf() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VMACU     | Vector (Unsigned Frac) Multiply Accumulate        |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  001001   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VMACU $v<dest>, $v<s1>, $v<s2>[el]
*/
void vmacu() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VRNDN     | Vector DCT Round (-)                              |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  001010   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VRNDN $v<dest>, $v<s1>, $v<s2>[el]
*/
void vrndn() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VMACQ     | Vector (Integer) Multiply Accumulate              |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  001011   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VMACQ $v<dest>, $v<s1>, $v<s2>[el]
*/
void vmacq() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VMADL     | Vector low multiply accumulate                    |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  001100   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VMADL $v<dest>, $v<s1>, $v<s2>[el]
*/
void vmadl() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VMADM     | Vector mid-m multiply accumulate                  |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  001101   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VMADM $v<dest>, $v<s1>, $v<s2>[el]
*/
void vmadm() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VMADN     | Vector mid-n multiply accumulate                  |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  001110   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VMADN $v<dest>, $v<s1>, $v<s2>[el]
*/
void vmadn() {
#ifdef _DEBUG
#endif
}

/*
  -----------------------------------------------------------------
  | VMADH     | Vector high multiply accumulate                   |
  |-----------|---------------------------------------------------|
  |  010010   |1|  el   |   s2    |   s1    |  dest   |  001111   |
  ------6------1----4--------5---------5---------5----------6------
  Format:  VMADH $v<dest>, $v<s1>, $v<s2>[el]
*/
void vmadh() {
#ifdef _DEBUG
#endif
}

//VADD         $v<dest>, $v<s1>, $v<s2>[el]  Vector Add
void vadd() {
#ifdef _DEBUG
#endif
}

//VSUB         $v<dest>, $v<s1>, $v<s2>[el]  Vector Subtract
void vsub() {
#ifdef _DEBUG
#endif
}

//VABS         $v<dest>, $v<s1>, $v<s2>[el]  Vector Absolute Value
void vabs() {
#ifdef _DEBUG
#endif
}

//VADDC        $v<dest>, $v<s1>, $v<s2>[el]  Vector ADDC
void vaddc() {
#ifdef _DEBUG
#endif
}

//VSUBC        $v<dest>, $v<s1>, $v<s2>[el]  Vector SUBC
void vsubc() {
#ifdef _DEBUG
#endif
}


//VLT          ?                             Vector Less Than
void vlt() {
#ifdef _DEBUG
#endif
}

//VEQ          ?                             Vector Equal To
void veq() {
#ifdef _DEBUG
#endif
}

//VNE          ?                             Vector Not Equal To
void vne() {
#ifdef _DEBUG
#endif
}

//VGE          ?                             Vector Greater Than or Equal To
void vge() {
#ifdef _DEBUG
#endif
}

//VCL          ?                             Vector Clip Low
void vcl() {
#ifdef _DEBUG
#endif
}

//VCH          ?                             Vector Clip High
void vch() {
#ifdef _DEBUG
#endif
}

//VCR          ?                             Vector, 1's Complement Clip
void vcr() {
#ifdef _DEBUG
#endif
}

//VMRG         ?                             Vector Merge
void vmrg() {
#ifdef _DEBUG
#endif
}

//VAND         ?                             Vector Logical AND
void vand() {
#ifdef _DEBUG
#endif
}

//VNAND        ?                             Vector Logical NOT AND
void vnand() {
#ifdef _DEBUG
#endif
}

//VOR          ?                             Vector Logical OR
void vor() {
#ifdef _DEBUG
#endif
}

//VNOR         ?                             Vector Logical NOT OR
void vnor() {
#ifdef _DEBUG
#endif
}

//VXOR         ?                             Vector Logical Exclusive OR
void vxor() {
#ifdef _DEBUG
#endif
}

//VNXOR        ?                             Vector Logical NOT Exclusive OR
void vxnor() {
#ifdef _DEBUG
#endif
}


//VRCP         ?                             Single Precision, Lookup Source, Write Result
void vrcp() {
#ifdef _DEBUG
#endif
}

//VRCPL        ?                             Lookup Source and Previous, Write Result
void vrcpl() {
#ifdef _DEBUG
#endif
}

//VRCPH        ?                             Set Source, Write Previous Result
void vrcph() {
#ifdef _DEBUG
#endif
}

//VMOV         ?                             Vector Move
void vmov() {
#ifdef _DEBUG
#endif
}

//VRSQ         ?                             Single Precision, Lookup Source, Write Result
void vrsq() {
#ifdef _DEBUG
#endif
}

//VRSQL        ?                             Lookup Source and Previous, Write Result
void vrsql() {
#ifdef _DEBUG
#endif
}

//VRSQH        ?                             Set Source, Write Previous Result
void vrsqh() {
#ifdef _DEBUG
#endif
}

//VRSQH        ?                             Vector Saw
void vsaw() {
#ifdef _DEBUG
#endif
}
