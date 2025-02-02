#include "lc3VM.h"




int main()
{
    lc3VM* VM = new lc3VM();
    //becaus\e the flags are like values and the register is like only one value, then we can only use one value at a time.
    VM->reg[R_COND] = FL_ZRO; //so we enable zero flag now

    //set pc to starting pos (0x3000 is default for little computer-3)
    VM->reg[R_PC] = PC_START;

    bool running = true; //bool for running thing
    
    while (running)
    {
        //get instruction
        uint16_t instruction = VM->mem_read(VM->reg[R_PC]++); //get the instruction and increase pc count
        uint16_t opCode = instruction >> 12; //this takes only the 4 bit opcode: from 00001111111111111, shifts all 1s to the 

        switch (opCode)
        {
        case OP_ADD:
            VM->ADD(instruction);
            
            break;

        case OP_AND:
            VM->AND(instruction);
            
            break;

        case OP_NOT:
            VM->NOT(instruction);
            
            break;

        case OP_BR:
             VM->BR(instruction);
            
            break;

        case OP_JMP:
            VM->JMP(instruction);
            
            break;

        case OP_JSR:
            VM->JSR(instruction);
      
            break;

        case OP_LD:
            VM->LD(instruction);
            
            break;

        case OP_LDI:
            VM->LDI(instruction);
            
            break;

        case OP_LDR:
            VM->LDR(instruction);
            
            break;

        case OP_LEA:
            VM->LEA(instruction);
            
            break;

        case OP_ST:
            VM->ST(instruction);
            
            break;

        case OP_STI:
            VM->STI(instruction);
           
            break;

        case OP_STR:
            VM->STR(instruction);
            
            break;

        case OP_TRAP:
            VM->TRAP(instruction);
           
            break;

        case OP_RES:
            VM->RES(instruction);
           
            break;

        case OP_RTI:
            VM->RTI(instruction);
            break;

        default:
            VM->errorHasAcurred(BAD_OPCODE);
            break;
        }
    }
}
