#include "lc3.h"
#include "memory.h"
#include "trap.h"


#define PC_START 0x3000

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("Error: No image file provided. Please specifiy an image file.\n");
        printf("lc3vm [path\\to\\image] ...\n");
        exit(1);
    }
    
    for (int j = 1; j < argc; ++j)
    {
        if (!read_image(argv[j]))
        {
            printf("Failed to load image: %s\n", argv[j]);
            exit(2);
        }
    }

    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    reg[R_COND] = FL_ZRO;
    reg[R_PC] = PC_START;

    int running = 1;
    while (running)
    {
        /* FETCH */
        uint16_t instr = mem_read(reg[R_PC]++);
        execute_instruction(instr);
    }
    restore_input_buffering();
}