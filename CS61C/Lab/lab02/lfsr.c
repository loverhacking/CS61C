#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "lfsr.h"

unsigned get_bit(unsigned x,
                 unsigned n) {
    return (x >> n) & 1;
}

void lfsr_calculate(uint16_t *reg) {
    unsigned res = get_bit(*reg, 0) ^ get_bit(*reg, 2)
    ^ get_bit(*reg, 3) ^ get_bit(*reg, 5);
    *reg = *reg >> 1;
    *reg = (*reg & ~(1 << 15)) | (res << 15);
}


