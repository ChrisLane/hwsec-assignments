//#!/usr/bin/c
#include <stdint.h>
#include <stdio.h>

static const uint8_t sbox[16] = {
        0xC, 0x5, 0x6, 0xB,
        0x9, 0x0, 0xA, 0xD,
        0x3, 0xE, 0xF, 0x8,
        0x4, 0x7, 0x1, 0x2
};

void main() {
    printf("static const uint8_t sbox[256] = {");
    for (int i = 0; i < 16; ++i) {
        printf("\n    ");
        for (int j = 0; j < 16; ++j) {
            printf("0x%02X, ", sbox[i] << 4 | sbox[j]);
        }
    }
    printf("\n};");
}

