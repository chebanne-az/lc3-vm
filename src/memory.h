#ifndef MEMORY_H
#define MEMORY_H

#include "lc3.h"

void read_image_file(FILE* file);
int read_image(const char* image_path);
void mem_write(uint16_t address, uint16_t val);
uint16_t mem_read(uint16_t address);

#endif