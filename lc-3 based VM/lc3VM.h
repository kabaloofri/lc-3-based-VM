#pragma once
#include <iostream>
#include <stdint.h>
#include <signal.h>
/* windows only */
#include <Windows.h>
#include <conio.h>  // _kbhit


#define MEMORY_MAX (1 << 16)
#define PC_START 0x3000

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


enum // trap routines- like mini functions stuff api
{
    TRAP_GETC = 0x20,  /* get character from keyboard, not echoed onto the terminal */
    TRAP_OUT = 0x21,   /* output a character */
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25   /* halt the program */
};

enum // all the errors
{
    GOOD = 0,
    BAD_OPCODE = 1,

};



class lc3VM
{
public:

    lc3VM();
    ~lc3VM();
	uint16_t* memory; //literal memory (stored in an array)
	uint16_t* reg; //registers- every reg is just an 16 bit unsigend int (regs are r0-8, cp, flags) in list
	//stack??

	
	int ADD(uint16_t instruction);
	int AND(uint16_t instruction);
	int NOT(uint16_t instruction);
	int BR(uint16_t instruction);
	int JMP(uint16_t instruction);
	int JSR(uint16_t instruction);
	int LD(uint16_t instruction);
	int LDI(uint16_t instruction);
	int LDR(uint16_t instruction);
	int LEA(uint16_t instruction);
	int ST(uint16_t instruction);
	int STI(uint16_t instruction);
	int STR(uint16_t instruction);
	int TRAP(uint16_t instruction);
	int RES(uint16_t instruction); //maybe switch these 2 to psh and pop from stack
	int RTI(uint16_t instruction);


	void errorHasAcurred(int code)
	{
		std::cout << "error happend! at line " << mem_read(reg[R_PC]) << " with error code " << code << std::endl;
		exit(1);
	}

	uint16_t mem_read(uint16_t address);
    void mem_write(uint16_t address, uint16_t val);
    uint16_t sign_extend(uint16_t x, int bit_count);
    void update_flags(uint16_t r);
};

