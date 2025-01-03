#ifndef LC3_H
#define LC3_H

#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

/* Registers */
enum
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,    /* program counter */
    R_COND,  /* conditon register */
    R_COUNT
};

/* Code Condition Flags*/
enum
{
    FL_POS = 1 << 0, /* P */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2, /* N */
};

/* Op Codes*/
enum
{
    OP_BR = 0, /* branch */
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
    OP_RES,    /* unused */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

/* Memory Mapped Registers */
enum
{
    MR_KBSR = 0xFE00, /* keyboard status register */
    MR_KBDR = 0xFE02  /* keyboard data register */
};

#define MEMORY_MAX (1 << 16)    /* 65536 locations */

extern uint16_t memory[MEMORY_MAX];
extern uint16_t reg[R_COUNT];

void disable_input_buffering();
void restore_input_buffering();
uint16_t check_key();
void handle_interrupt(int signal);
uint16_t sign_extend(uint16_t x, int bit_count);
uint16_t swap16(uint16_t x);
void update_flags(uint16_t r);
void execute_instruction(uint16_t instr);

#endif