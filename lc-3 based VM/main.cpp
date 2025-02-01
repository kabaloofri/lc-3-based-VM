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
int LD(uint16_t instrutcion);
int LDI(uint16_t instruction);
int LDR(uint16_t instruction);
int LEA(uint16_t instruction);
int ST(uint16_t instrutcion);
int STI(uint16_t instrutcion);
int STR(uint16_t instrutcion);
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

// adds 2 registers or a register and an immedate and stores the result in another register.
// Usage:
// ADD destReg firstReg secondReg/Immediate
// Example:
// ADD R0 R1 5 - adds R1 and 5 and stores them in R0
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



// does bitwise and between 2 registers or a register and an immediate
// Usage:
// AND destReg reg reg/5BitImmediate
// Example:
// AND R0 R1 R2 - does bitwise AND between R1 and R2 and stores in R0
// AND R0 R1 10101 - does bitwise AND between R1 and 10101, and stores in R0
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




// does bitwise NOT on register or and stores result in another register
// Usage:
// NOT destReg reg
// Example:
// NOT R0 R1 - does bitwise NOT on R1 and stores in R0
int NOT(uint16_t instruction)
{
    uint16_t destReg = (instruction >> 9) & 0b111;
    uint16_t r1 = (instruction >> 6) & 0b111;

    reg[destReg] = ~reg[r1];

    update_flags(destReg);
    return GOOD;
}

// branch- like LOOP in 8086, jumps to memory location (label) if the flag(s) specified were on
// Usage: 
// BR label/memoryLocation
// Example:
// BRn LOOP(101010101) - if N flag is on then jump to LOOP(101010101)
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




// jumps to a certain location in memory
// Usage:
// JMP register
// Example:
// JMP R0 - jumps to the location in the memory that is in R0
// special case: JMP R7 is .RET (see .h file in regs)
int JMP(uint16_t instruction)
{
    uint16_t destReg = (instruction >> 6) & 0b111;
    reg[R_PC] = reg[1];
    return GOOD;
}


//jump to subroutine- jumps to a subroutine (function) - puts pc curr location in R7 and jumps to location
// Usage:
// JSR label/regiter
// Example:
// JSR R0 - jumps to R0
// JSR label(101010101) - jumps to label(101010101)
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



// Load Indirect: loads a vaule from a memory location to a register.
// Usage:
// LDI destReg 9bitMemoryLocation
// Example:
// LDI R0 101010101 - puts the value of the location in the memory of 101010101 
// so if in 101010101 there is 5, it will put whatever is in memory 5 in R0
int LDI(uint16_t instruction)
{
    uint16_t destReg = (instruction >> 9) & 0b111; //destenation register
    uint16_t pcOffset = sign_extend(instruction & 0b111111111, 9); //get the pc offset and extend 9 bit to 16

    reg[destReg] = mem_read(mem_read(reg[R_PC] + pcOffset));
    update_flags(destReg);
    return GOOD;
}




