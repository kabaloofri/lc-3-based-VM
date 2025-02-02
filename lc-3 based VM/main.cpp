#include <iostream>
#include <stdint.h>
#include <signal.h>
/* windows only */
#include <Windows.h>
#include <conio.h>  // _kbhit
#include "stuff.h"

uint16_t memory[MEMORY_MAX]; //literal memory (stored in an array)
uint16_t reg[R_COUNT]; //registers- every reg is just an 16 bit unsigend int (regs are r0-8, cp, flags) in list



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


int main()
{
    //because the flags are like values and the register is like only one value, then we can only use one value at a time.
    reg[R_COND] = FL_ZRO; //so we enable zero flag now

    //set pc to starting pos (0x3000 is default for little computer-3)
    int PC_START = 0x3000;
    reg[R_PC] = PC_START;

    bool running = true; //bool for running thing
    
    while (running)
    {
        //get instruction
        uint16_t instruction = mem_read(reg[R_PC]++); //get the instruction and increase pc count
        uint16_t opCode = instruction >> 12; //this takes only the 4 bit opcode: from 00001111111111111, shifts all 1s to the 

        switch (opCode)
        {
        case OP_ADD:
            ADD(instruction);
            
            break;

        case OP_AND:
            AND(instruction);
            
            break;

        case OP_NOT:
            NOT(instruction);
            
            break;

        case OP_BR:
             BR(instruction);
            
            break;

        case OP_JMP:
            JMP(instruction);
            
            break;

        case OP_JSR:
            JSR(instruction);
      
            break;

        case OP_LD:
            LD(instruction);
            
            break;

        case OP_LDI:
            LDI(instruction);
            
            break;

        case OP_LDR:
            LDR(instruction);
            
            break;

        case OP_LEA:
            LEA(instruction);
            
            break;

        case OP_ST:
            ST(instruction);
            
            break;

        case OP_STI:
            STI(instruction);
           
            break;

        case OP_STR:
            STR(instruction);
            
            break;

        case OP_TRAP:
            TRAP(instruction);
           
            break;

        case OP_RES:
            RES(instruction);
           
            break;

        case OP_RTI:
            RTI(instruction);
            break;

        default:
            errorHasAcurred(BAD_OPCODE);
            break;
        }
    }
}

uint16_t mem_read(uint16_t address)
{
    return memory[address];
}

void mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

void update_flags(uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) /* a 1 in the left-most bit indicates negative */
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

/*
If bit [5] is 0, the second source operand is obtained from SR2. If bit [5] is 1, the
second source operand is obtained by sign-extending the imm5 field to 16 bits.
In both cases, the second source operand is added to the contents of SR1 and the
result stored in DR. The condition codes are set, based on whether the result is
negative, zero, or positive.
Examples
ADD R2, R3, R4 ; R2 <- R3 + R4
ADD R2, R3, #7 ; R2 <- R3 + 7
*/
int ADD(uint16_t instruction)
{
    /* destination register (DR) */
    uint16_t destReg = (instruction >> 9) & 0b111; //use AND 111 to get only the 3 first bits
    /* first operand (SR1) */
    uint16_t r1 = (instruction >> 6) & 0b111;
    /* whether we are in immediate mode */
    uint16_t imm_flag = (instruction >> 5) & 0b1;
    
    if (imm_flag)
    {
        uint16_t imm5 = sign_extend(instruction & 0b11111, 5);
        reg[destReg] = reg[r1] + imm5;
    }
    else
    {
        uint16_t r2 = instruction & 0b111;
        reg[destReg] = reg[r1] + reg[r2];
    }

    update_flags(destReg);
    return GOOD;
}



/*
If bit [5] is 0, the second source operand is obtained from SR2. If bit [5] is 1,
the second source operand is obtained by sign-extending the imm5 field to 16
bits. In either case, the second source operand and the contents of SR1 are bitwise ANDed, and the result stored in DR. The condition codes are set, based on
whether the binary value produced, taken as a 2’s complement integer, is negative,
zero, or positive.
Examples
AND R2, R3, R4 ;R2 <- R3 AND R4
AND R2, R3, #7 ;R2 <- R3 AND 7
*/
int AND(uint16_t instruction)
{
    uint16_t destReg = (instruction >> 9) & 0b111;
    uint16_t r1 = (instruction >> 6) & 0b111;
    bool immediateMode = (instruction >> 5) & 0b1;

    if (immediateMode)
    {
        uint16_t immedate = sign_extend(instruction & 0b11111, 5);
        reg[destReg] = reg[r1] & immedate;
    }
    else
    {
        uint16_t r2 = instruction & 0b111;
        reg[destReg] = reg[r1] & reg[r2];
    }
    update_flags(destReg);
    return GOOD;
}




/*
The bit-wise complement of the contents of SR is stored in DR. The condition codes are set, based on whether the binary value produced, taken as a 2’s
complement integer, is negative, zero, or positive.
Example
NOT R4, R2 ; R4 <- NOT(R2)
*/
int NOT(uint16_t instruction)
{
    uint16_t destReg = (instruction >> 9) & 0b111;
    uint16_t r1 = (instruction >> 6) & 0b111;

    reg[destReg] = ~reg[r1];

    update_flags(destReg);
    return GOOD;
}

/*
The condition codes specified by the state of bits [11:9] are tested. If bit [11] is
set, N is tested; if bit [11] is clear, N is not tested. If bit [10] is set, Z is tested, etc.
If any of the condition codes tested is set, the program branches to the location
specified by adding the sign-extended PCoffset9 field to the incremented PC.
Examples
BRzp LOOP ; Branch to LOOP if the last result was zero or positive.
BR NEXT ; Unconditionally branch to NEXT.
*/
int BR(uint16_t instruction)
{
    uint16_t flags = (instruction >> 9) & 0b111;
    uint16_t offset = sign_extend(instruction & 0b111111111, 9);
    if (flags & reg[R_COND]) // if a condition matches one of the flags then jmp
    {
        reg[R_PC] += offset;
    }
    return GOOD;
}




/*
The program unconditionally jumps to the location specified by the contents of
the base register. Bits [8:6] identify the base register.
Examples
JMP R2 ; PC <- R2
RET ; PC <- R7
Note
The RET instruction is a special case of the JMP instruction. The PC is loaded
with the contents of R7, which contains the linkage back to the instruction
following the subroutine call instruction.
*/
int JMP(uint16_t instruction)
{
    uint16_t destReg = (instruction >> 6) & 0b111;
    reg[R_PC] = reg[1];
    return GOOD;
}


/*
First, the incremented PC is saved in R7. This is the linkage back to the calling
routine. Then the PC is loaded with the address of the first instruction of the
subroutine, causing an unconditional jump to that address. The address of the
subroutine is obtained from the base register (if bit [11] is 0), or the address is
computed by sign-extending bits [10:0] and adding this value to the incremented
PC (if bit [11] is 1).
Examples:
JSR QUEUE ; Put the address of the instruction following JSR into R7 ; Jump to QUEUE.
JSRR R3 ; Put the address following JSRR into R7; Jump to the address contained in R3.
*/
int JSR(uint16_t instruction)
{
    uint16_t jmpToSubRFlag = (instruction >> 11) & 0b1;
    reg[R_R7] = reg[R_PC]; //set R7 to pc
    if (jmpToSubRFlag)
    {
        uint16_t address = sign_extend(instruction & 0b11111111111, 11);
        reg[R_PC] += address;
    }
    else
    {
        uint16_t r0 = (instruction >> 6) & 0b111;
        reg[R_PC] = reg[r0];
    }

    return GOOD;
}



/*
An address is computed by sign-extending bits [8:0] to 16 bits and adding this
value to the incremented PC. The contents of memory at this address are loaded
into DR. The condition codes are set, based on whether the value loaded is
negative, zero, or positive.
Example:
LD R4, VALUE ; R4 <- mem[VALUE]
*/
int LD(uint16_t instruction)
{
    uint16_t destReg = (instruction >> 9) & 0b111;
    uint16_t offset = sign_extend(instruction & 0b111111111, 9);
    
    reg[destReg] = mem_read(reg[R_PC] + offset);
    update_flags(destReg);
    return GOOD;
}



/*
An address is computed by sign-extending bits [8:0] to 16 bits and adding this
value to the incremented PC. What is stored in memory at this address is the
address of the data to be loaded into DR. The condition codes are set, based on
whether the value loaded is negative, zero, or positive.
Example:
LDI R4, ONEMORE ; R4 <- mem[mem[ONEMORE]]
*/
int LDI(uint16_t instruction)
{
    uint16_t destReg = (instruction >> 9) & 0b111; //destenation register
    uint16_t pcOffset = sign_extend(instruction & 0b111111111, 9); //get the pc offset and extend 9 bit to 16

    reg[destReg] = mem_read(mem_read(reg[R_PC] + pcOffset));
    update_flags(destReg);
    return GOOD;
}


/*
An address is computed by sign-extending bits [5:0] to 16 bits and adding this
value to the contents of the register specified by bits [8:6]. The contents of memory
at this address are loaded into DR. The condition codes are set, based on whether
the value loaded is negative, zero, or positive.
Example:
LDR R0 R1 -20  ; R0 <- mem[R1 + (-20)]
*/
int LDR(uint16_t instruction)
{
    uint16_t destReg = (instruction >> 9) & 0b111; //destenation register
    uint16_t base = (instruction >> 6) & 0b111;
    uint16_t offset = sign_extend(instruction & 0b111111, 6);
    
    reg[destReg] = mem_read(base + offset);

    update_flags(destReg);
    return GOOD;
}


/*
An address is computed by sign-extending bits [8:0] to 16 bits and adding this
value to the incremented PC. This address is loaded into DR. The condition
codes are set, based on whether the value loaded is negative, zero, or positive.
Example
LEA R4, TARGET ; R4 <- address of TARGET.
*/
int LEA(uint16_t instruction)
{
    uint16_t destReg = (instruction >> 9) & 0b111;
    uint16_t offset = sign_extend(instruction & 0b111111111, 9);

    reg[destReg] = reg[R_PC] + offset;
    update_flags(destReg);
    return GOOD;
}




/*
The contents of the register specified by SR are stored in the memory location
whose address is computed by sign-extending bits [8:0] to 16 bits and adding this
value to the incremented PC.
Example:
ST R4, HERE ; mem[HERE] <- R4
*/
int ST(uint16_t instruction)
{
    uint16_t srcReg = (instruction >> 9) & 0b111;
    uint16_t offset = sign_extend(instruction & 0b111111111, 9);

    mem_write(reg[R_PC] + offset, reg[srcReg]);
    return GOOD;
}





/*
The contents of the register specified by SR are stored in the memory location
whose address is obtained as follows: Bits [8:0] are sign-extended to 16 bits and
added to the incremented PC. What is in memory at this address is the address of
the location to which the data in SR is stored.
Example
STI R4, NOT_HERE ; mem[mem[NOT_HERE]] <- R4
*/
int STI(uint16_t instruction)
{
    uint16_t srcReg = (instruction >> 9) & 0b111;
    uint16_t offset = sign_extend(instruction & 0b111111111, 9);

    mem_write(mem_read(reg[R_PC] + offset), reg[srcReg]);
    return GOOD;
}





/*
The contents of the register specified by SR are stored in the memory location
whose address is computed by sign-extending bits [5:0] to 16 bits and adding this
value to the contents of the register specified by bits [8:6].
Example
STR R4, R2, 5 ; mem[R2 + 5] <- R4
*/
int STR(uint16_t instruction)
{
    uint16_t srcReg = (instruction >> 9) & 0b111;
    uint16_t baseReg = (instruction >> 6) & 0b111;
    uint16_t offset = sign_extend(instruction & 0b111111, 6);

    mem_write(reg[baseReg] + offset, reg[srcReg]);
    return GOOD;
}




