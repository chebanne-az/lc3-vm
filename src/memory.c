#include "memory.h"

uint16_t memory[MEMORY_MAX];  /* 65536 locations */

void read_image_file(FILE* file)
{
    // origin is the start address of the program
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin); // swap to little endian

    uint16_t max_read = MEMORY_MAX - origin;
    uint16_t* p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    while (read-- > 0)
    {
        *p = swap16(*p); // swap to little endian
        ++p;
    }
}

int read_image(const char* image_path)
{
    FILE* file = fopen(image_path, "rb"); // read binary
    if (!file) { return 0; };
    read_image_file(file);
    fclose(file);
    return 1;
}

void mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val; 
}

uint16_t mem_read(uint16_t address)
{
    if (address == MR_KBSR) // keyboard status register
    {
        if (check_key())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
        else
        {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[address];
}