#include <stdint.h>
#include <stdio.h>
#include "crypto.h"

int main() {
    uint8_t test[CRYPTO_IN_SIZE * BITSLICE_WIDTH] = {0};
    uint8_t key[CRYPTO_KEY_SIZE] = {0};

    crypto_func(test, key);

    for (int i = 0; i < BITSLICE_WIDTH; ++i) {
        for (int j = 0; j < CRYPTO_IN_SIZE; ++j) {
            printf("%02X ", test[(i * CRYPTO_IN_SIZE) + j]);
        }
        printf("\n");
    }
}