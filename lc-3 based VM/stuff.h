#pragma once

#define MEMORY_MAX (1 << 16)

enum //register indexes in register file
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7, //contains the last location of the last subroutine so JMP R7 is .RET
    R_PC, /* program counter */
    R_COND,
    R_COUNT
};


enum //operation codes (opcodes) - 4 bit
{
    OP_BR = 0, /* branch - like loop*/
    OP_ADD,    /* add  */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

enum //condition flags (in the R_COND - flags reg)
{
    FL_POS = 1 << 0, /* P 001 */
    FL_ZRO = 1 << 1, /* Z 010 */
    FL_NEG = 1 << 2, /* N 100 */
};

enum // all the errors
{
    GOOD = 0,
    BAD_OPCODE = 1,

};
