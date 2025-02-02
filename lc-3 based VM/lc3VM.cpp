#include "lc3VM.h"


uint16_t lc3VM::mem_read(uint16_t address)
{
    return memory[address];
}

void lc3VM::mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

uint16_t lc3VM::sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

void lc3VM::update_flags(uint16_t r)
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

lc3VM::lc3VM()
{
    memory = new uint16_t[MEMORY_MAX];
    reg = new uint16_t[R_COUNT];
}

lc3VM::~lc3VM()
{
    delete reg;
    delete memory;
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
int lc3VM::ADD(uint16_t instruction)
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
int lc3VM::AND(uint16_t instruction)
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
int lc3VM::NOT(uint16_t instruction)
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
int lc3VM::BR(uint16_t instruction)
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
int lc3VM::JMP(uint16_t instruction)
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
int lc3VM::JSR(uint16_t instruction)
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
int lc3VM::LD(uint16_t instruction)
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
int lc3VM::LDI(uint16_t instruction)
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
int lc3VM::LDR(uint16_t instruction)
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
int lc3VM::LEA(uint16_t instruction)
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
int lc3VM::ST(uint16_t instruction)
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
int lc3VM::STI(uint16_t instruction)
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
int lc3VM::STR(uint16_t instruction)
{
    uint16_t srcReg = (instruction >> 9) & 0b111;
    uint16_t baseReg = (instruction >> 6) & 0b111;
    uint16_t offset = sign_extend(instruction & 0b111111, 6);

    mem_write(reg[baseReg] + offset, reg[srcReg]);
    return GOOD;
}