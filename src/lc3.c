#include "lc3.h"
#include "memory.h"
#include "trap.h"

uint16_t reg[R_COUNT];

struct termios original_tio;

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) // x is negative
    { 
        x |= (0xFFFF << bit_count);
    }
    return x;
}

uint16_t swap16(uint16_t x)
{
    // swap the lower and upper bytes of x 
    // Little Endian to Big Endian and vice versa
    return (x << 8) | (x >> 8);
}

void update_flags(uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) // a 1 in the leftmost bit indicates negative
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

void execute_instruction(uint16_t instr)
{
    // extract the op code; last 4 bits 
    uint16_t op = instr >> 12;

    switch (op)
    {
        /*
        Resgister Mode
        +----+---+---+-+--+---+
        |0001|DR |SR1|0|00|SR2|
        +----+---+---+-+--+---+

        Immediate  Mode
        +----+---+---+-+------+
        |0001|DR |SR1|1| imm5 |
        +----+---+---+-+------+
        */
        case OP_ADD:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t imm_flag = (instr >> 5) & 0x1;
            
                if (imm_flag)
                {
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] + imm5;
                }
                else
                {
                    uint16_t r2 = instr & 0x7;
                    reg[r0] = reg[r1] + reg[r2];
                }
            
                update_flags(r0);
            }
            break;
        /*
        Resgister Mode
        +----+---+---+-+--+---+
        |0101|DR |SR1|0|00|SR2|
        +----+---+---+-+--+---+

        Immediate Mode
        +----+---+---+-+------+
        |0101|DR |SR1|1| imm5 |
        +----+---+---+-+------+
        */
        case OP_AND:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t imm_flag = (instr >> 5) & 0x1;
            
                if (imm_flag)
                {
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] & imm5;
                }
                else
                {
                    uint16_t r2 = instr & 0x7;
                    reg[r0] = reg[r1] & reg[r2];
                }
                update_flags(r0);
            }
            break;
        /*
        +----+---+---+-+-----+
        |1001|DR |SR |1|11111|
        +----+---+---+-+-----+
        */
        case OP_NOT:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
            
                reg[r0] = ~reg[r1];
                update_flags(r0);
            }
            break;
        /*
        +----+-+-+-+---------+
        |0000|n|z|p|PCoffset9|
        +----+-+-+-+---------+
        */
        case OP_BR:
            {
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                uint16_t cond_flag = (instr >> 9) & 0x7;
                if (cond_flag & reg[R_COND])
                {
                    reg[R_PC] += pc_offset;
                }
            }
            break;
        /*
        +----+---+ --- +--------+
        |1100|000|BaseR| 000000 |
        +----+---+ --- +--------+
        */
        case OP_JMP:
            {
                uint16_t r1 = (instr >> 6) & 0x7;
                reg[R_PC] = reg[r1];
            }
            break;
        /*
        JSR: 
        +----+-+-----------+
        |0100|1| PCoffset11|
        +----+-+-----------+

        JSRR:
        +----+-+--+ --- +------+
        |0100|0|00|BaseR|000000|
        +----+-+--+ --- +------+
        */
        case OP_JSR:
            {
                uint16_t long_flag = (instr >> 11) & 1;
                reg[R_R7] = reg[R_PC];
                if (long_flag)
                {
                    uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
                    reg[R_PC] += long_pc_offset;
                }
                else
                {
                    uint16_t r1 = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[r1];
                }
            }
            break;
        /*
        +----+---+---------+
        |0010|DR |PCoffset9|
        +----+---+---------+
        */
        case OP_LD:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                reg[r0] = mem_read(reg[R_PC] + pc_offset);
                update_flags(r0);
            }
            break;
        /*
        +----+---+---------+
        |1010|DR |PCoffset9|
        +----+---+---------+
        */
        case OP_LDI:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
                update_flags(r0);
            }
            break;
        /*
        +----+---+ --- + ------  +
        |1010|DR |BaseR|PCoffset6|
        +----+---+ --- +---------+
        */
        case OP_LDR:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset = sign_extend(instr & 0x3F, 6);
                reg[r0] = mem_read(reg[r1] + offset);
                update_flags(r0);
            }
            break;
        /*
        +----+---+---------+
        |1110|DR |PCoffset9|
        +----+---+---------+
         */
        case OP_LEA:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                reg[r0] = reg[R_PC] + pc_offset;
                update_flags(r0);
            }
            break;
        /*
        +----+---+---------+
        |0011|SR |PCoffset9|
        +----+---+---------+
        */
        case OP_ST:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                mem_write(reg[R_PC] + pc_offset, reg[r0]);
            }
            break;
        /*
        +----+---+---------+
        |1011|SR |PCoffset9|
        +----+---+---------+
        */
        case OP_STI:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
            }
            break;
        /*
        +----+---+ --- +  ------  +
        |0111|SR |BaseR|PCoffset6 |
        +----+---+ --- +  ------  +
        */
        case OP_STR:
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset = sign_extend(instr & 0x3F, 6);
                mem_write(reg[r1] + offset, reg[r0]);
            }
            break;
        /*
        +----+----+--------+
        |1111|0000|trapvect8|
        +----+----+--------+
        */
        case OP_TRAP:
            execute_trap(instr);
            break;
        case OP_RES:    /* unused */
        case OP_RTI:    /* unused */
        default:
            abort();
            break;
    }
}