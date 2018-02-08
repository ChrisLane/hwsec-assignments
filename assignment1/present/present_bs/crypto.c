#include "crypto.h"

/**
 * Perform next key schedule step
 * @param key   Key register to be updated
 * @param r     Round counter
 * @warning For correct function, has to be called with incremented r each time
 * @note You are free to change or optimize this function
 */
static void update_round_key(uint8_t key[CRYPTO_KEY_SIZE], const uint8_t r) {
    const uint8_t sbox[16] = {
            0xC, 0x5, 0x6, 0xB,
            0x9, 0x0, 0xA, 0xD,
            0x3, 0xE, 0xF, 0x8,
            0x4, 0x7, 0x1, 0x2,
    };

    uint8_t tmp = 0;
    const uint8_t tmp2 = key[2];
    const uint8_t tmp1 = key[1];
    const uint8_t tmp0 = key[0];

    // rotate right by 19 bit
    key[0] = key[2] >> 3 | key[3] << 5;
    key[1] = key[3] >> 3 | key[4] << 5;
    key[2] = key[4] >> 3 | key[5] << 5;
    key[3] = key[5] >> 3 | key[6] << 5;
    key[4] = key[6] >> 3 | key[7] << 5;
    key[5] = key[7] >> 3 | key[8] << 5;
    key[6] = key[8] >> 3 | key[9] << 5;
    key[7] = key[9] >> 3 | tmp0 << 5;
    key[8] = tmp0 >> 3 | tmp1 << 5;
    key[9] = tmp1 >> 3 | tmp2 << 5;

    // perform sbox lookup on MSbits
    tmp = sbox[key[9] >> 4];
    key[9] &= 0x0F;
    key[9] |= tmp << 4;

    // XOR round counter k19 ... k15
    key[1] ^= r << 7;
    key[2] ^= r >> 1;
}

void crypto_func(uint8_t pt[CRYPTO_IN_SIZE * BITSLICE_WIDTH], uint8_t key[CRYPTO_KEY_SIZE]) {
    // State buffer and additional backbuffer of same size (you can remove the backbuffer if you do not need it)
    bs_reg_t state[CRYPTO_IN_SIZE_BIT];
    bs_reg_t bb[CRYPTO_IN_SIZE_BIT];

    // Bring into bitslicing form (enslice)
    for (int i1 = 0; i1 < CRYPTO_IN_SIZE_BIT; ++i1) {
        for (int j = 0; j < BITSLICE_WIDTH; ++j) {
            state[i1] |= (pt[i1 / 8] & 0x1) << j;
        }
    }

    // S-Box
    for (int i = 0; i < CRYPTO_IN_SIZE_BIT; i += 4) {
        // y0 = x0 + (x1 · x2) + x2 + x3
        bb[i + 0] = state[0] ^ (state[1] & state[2]) ^ state[2] ^ state[3];
        // y1 = (x0 · x2 · x1) + (x0 · x3 · x1) + (x3 · x1) + x1 + (x0 · x2 · x3) + (x2 · x3) + x3
        bb[i + 1] = (state[0] & state[2] & state[1]) ^ (state[0] & state[3] & state[1]) ^ (state[3] & state[1]) ^ state[1] ^ (
                state[0] & state[2] & state[3]) ^ (state[2] & state[3]) ^ state[3];
        // y2 = (x0 · x1) + (x0 · x3 · x1) + (x3 · x1) + x2 + (x0 · x3) + (x0 · x2 · x3) + x3 + 1
        bb[i + 2] = ~((state[0] & state[2]) ^ (state[0] & state[3] & state[1]) ^ (state[3] & state[1]) ^ state[2] ^ (
                state[0] & state[3]) ^ (state[0] & state[2] & state[3]) ^ state[3]);
        // y3 = (x1 · x2 · x0) + (x1 · x3 · x0) + (x2 · x3 · x0) + x0 + x1 + x1 · x2 + x3 + 1
        bb[i + 3] = ~((state[1] & state[2] & state[0]) ^ (state[1] & state[3] & state[0]) ^ (state[2] & state[3] & state[0]) ^ state[0] ^ state[1] ^ state[1] & state[2] ^ state[3]);
    }

    // P-Box
    // {INSERT PBXXXXOOOOOOOOOOOOOOOOODDDDDDDDDDDDXXXXXXXXX HERE


    // Convert back to normal form (unslice)
    for (int i2 = 0; i2 < CRYPTO_IN_SIZE_BIT; ++i2) {
        for (int j1 = 0; j1 < BITSLICE_WIDTH; ++j1) {
            pt[i2] |= (state[i2 / 8] & 0x1) << j1;
        }
    }
}