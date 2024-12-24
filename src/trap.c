#include "trap.h"

void execute_trap(uint16_t instr)
{
    // save the PC into R7 before we start executing the trap
    reg[R_R7] = reg[R_PC];
    
    switch (instr & 0xFF)
    {   
        /*
        Read a single character from the keyboard. The character is not echoed onto the
        console. Its ASCII code is copied into R0. The high eight bits of R0 are cleared.
        */
        case TRAP_GETC:
            reg[R_R0] = (uint16_t)getchar();
            update_flags(R_R0);
            break;
        /* 
        Write a character in R0[7:0] to the console display.
        */
        case TRAP_OUT:
            putc((char)reg[R_R0], stdout);
            fflush(stdout);
            break;
        /*
        Write ASCII characters from memory (R0) to the console, ending at x0000 (not written).
        */
        case TRAP_PUTS:
            {
                uint16_t* c = memory + reg[R_R0];
                while (*c)
                {
                    putc((char)*c, stdout);
                    ++c;
                }
                fflush(stdout);
            }
            break;
        /*
        Read a character, echo it, and store ASCII in R0 (high 8 bits cleared).
        */
        case TRAP_IN:
            {
                printf("Enter a character: ");
                char c = getchar();
                putc(c, stdout);
                fflush(stdout);
                reg[R_R0] = (uint16_t)c;
                update_flags(R_R0);
            }
            break;
        /*
        Write ASCII characters from memory to the console. Each memory location contains two characters: 
        bits [7:0] are written first, then bits [15:8]. If the string has an odd length, x00 fills bits [15:8] 
        of the last location. Writing stops at x0000 in memory.
        */
        case TRAP_PUTSP:
            {
                uint16_t* c = memory + reg[R_R0];
                while (*c)
                {
                    char char1 = (*c) & 0xFF;
                    putc(char1, stdout);
                    char char2 = (*c) >> 8;
                    if (char2) putc(char2, stdout);
                    ++c;
                }
                fflush(stdout);
            }
            break;
        /*
        Halt execution 
        */
        case TRAP_HALT:
            puts("HALT");
            fflush(stdout);
            exit(0);
            break;
    }
}