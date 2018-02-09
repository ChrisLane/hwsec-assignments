#include <stdio.h>
#include "crypto.h"

/**
 * Values of the P box.
 */
static const uint8_t pbox[64] = {
        0,  16, 32, 48, 1,  17, 33, 49,
        2,  18, 34, 50, 3,  19, 35, 51,
        4,  20, 36, 52, 5,  21, 37, 53,
        6,  22, 38, 54, 7,  23, 39, 55,
        8,  24, 40, 56, 9,  25, 41, 57,
        10, 26, 42, 58, 11, 27, 43, 59,
        12, 28, 44, 60, 13, 29, 45, 61,
        14, 30, 46, 62, 15, 31, 47, 63
};

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

#define SHORT_TO_BINARY(byte)  \
  (byte & 0x8000 ? '1' : '0'), \
  (byte & 0x4000 ? '1' : '0'), \
  (byte & 0x2000 ? '1' : '0'), \
  (byte & 0x1000 ? '1' : '0'), \
  (byte & 0x800 ? '1' : '0'), \
  (byte & 0x400 ? '1' : '0'), \
  (byte & 0x200 ? '1' : '0'), \
  (byte & 0x100 ? '1' : '0'), \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

void crypto_func(uint8_t pt[CRYPTO_IN_SIZE * BITSLICE_WIDTH], uint8_t key[CRYPTO_KEY_SIZE]) {
    // State buffer and additional backbuffer of same size (you can remove the backbuffer if you do not need it)
    bs_reg_t s[CRYPTO_IN_SIZE_BIT] = {0};
    bs_reg_t bb[CRYPTO_IN_SIZE_BIT] = {0};

    // Bring into bitslicing form (enslice)
    for (uint8_t i = 0; i < CRYPTO_IN_SIZE_BIT; ++i) {
        uint8_t by = i >> 3; // i / 8         i : 0 -> 7
        uint8_t b = (uint8_t) (i % 8);

        /*
        // This printing is to debug the correctness of the transpose operation
        printf("%02u: bm %3u, by %u, ", i, 0x1, by);
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by +   0] >> b & 0x1) << 0));
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by +   8] >> b & 0x1) << 1));
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by +  16] >> b & 0x1) << 2));
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by +  24] >> b & 0x1) << 3));
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by +  32] >> b & 0x1) << 4));
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by +  40] >> b & 0x1) << 5));
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by +  48] >> b & 0x1) << 6));
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", SHORT_TO_BINARY((pt[by +  56] >> b & 0x1) << 7));
        printf("                  ");
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by +  64] >> b & 0x1) << 8));
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by +  72] >> b & 0x1) << 9));
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by +  80] >> b & 0x1) << 10));
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by +  88] >> b & 0x1) << 11));
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by +  96] >> b & 0x1) << 12));
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by + 104] >> b & 0x1) << 13));
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by + 112] >> b & 0x1) << 14));
        printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c ",  SHORT_TO_BINARY((pt[by + 120] >> b & 0x1) << 15));
        */

        s[i] = (pt[by +   0] >> b & 0x1) <<  0 | (pt[by +   8] >> b & 0x1) <<  1 |
               (pt[by +  16] >> b & 0x1) <<  2 | (pt[by +  24] >> b & 0x1) <<  3 |
               (pt[by +  32] >> b & 0x1) <<  4 | (pt[by +  40] >> b & 0x1) <<  5 |
               (pt[by +  48] >> b & 0x1) <<  6 | (pt[by +  56] >> b & 0x1) <<  7 |
               (pt[by +  64] >> b & 0x1) <<  8 | (pt[by +  72] >> b & 0x1) <<  9 |
               (pt[by +  80] >> b & 0x1) << 10 | (pt[by +  88] >> b & 0x1) << 11 |
               (pt[by +  96] >> b & 0x1) << 12 | (pt[by + 104] >> b & 0x1) << 13 |
               (pt[by + 112] >> b & 0x1) << 14 | (pt[by + 120] >> b & 0x1) << 15;

//        printf("\t= %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", SHORT_TO_BINARY(s[i]));
    }

    for (uint8_t i = 1; i < 32; i++) {

        // Apply round-key
        
        for (int j = 0; j < CRYPTO_IN_SIZE_BIT; ++j) {
            // I have a feeling this isn't correct
            // Get the carry bit and XOR all 0s or all 1s
            s[j] ^= ((key[2 + (j >> 3)] << (j % 8)) & 0x1) ? 0xFFFF : 0;
        }

        // The sbox and pbox can be done together to reduce the amount of memory writes

        // S-Box
        for (int j = 0; j < CRYPTO_IN_SIZE_BIT; j += 4) {
            bs_reg_t *x = &s[j];
            // y0 = x0 + (x1 · x2) + x2 + x3
            bb[j + 0] = x[0] ^ (x[1] & x[2]) ^ x[2] ^ x[3];
            // y1 = (x0 · x2 · x1) + (x0 · x3 · x1) + (x3 · x1) + x1 + (x0 · x2 · x3) + (x2 · x3) + x3
            bb[j + 1] = (x[0] & x[2] & x[1]) ^ (x[0] & x[3] & x[1]) ^ (x[3] & x[1]) ^ x[1] ^ (x[0] & x[2] & x[3]) ^ (x[2] & x[3]) ^ x[3];
            // y2 = (x0 · x1) + (x0 · x3 · x1) + (x3 · x1) + x2 + (x0 · x3) + (x0 · x2 · x3) + x3 + 1
            bb[j + 2] = ~((x[0] & x[2]) ^ (x[0] & x[3] & x[1]) ^ (x[3] & x[1]) ^ x[2] ^ (x[0] & x[3]) ^ (x[0] & x[2] & x[3]) ^ x[3]);
            // y3 = (x1 · x2 · x0) + (x1 · x3 · x0) + (x2 · x3 · x0) + x0 + x1 + x1 · x2 + x3 + 1
            bb[j + 3] = ~((x[1] & x[2] & x[0]) ^ (x[1] & x[3] & x[0]) ^ (x[2] & x[3] & x[0]) ^ x[0] ^ x[1] ^ (x[1] & x[2]) ^ x[3]);
        }

        // Use the backing-buffer for sbox and move back into state with pbox

        // P-Box
        for (int j = 0; j < CRYPTO_IN_SIZE_BIT; ++j) {
            s[pbox[j]] = bb[j];
        }

        update_round_key(key, i);
    }

    // Apply round-key
    for (int i = 0; i < CRYPTO_IN_SIZE_BIT; ++i) {
        // I have a feeling this isn't correct
        // Get the carry bit and XOR all 0s or all 1s
        s[i] ^= ((key[2 + (i >> 3)] << (i % 8)) & 0x1) ? 0xFFFF : 0;
    }

    // Convert back to normal form (unslice)
    for (uint8_t i = 0; i < BITSLICE_WIDTH; ++i) {
        uint8_t blk = (uint8_t) (i << 3); // i * 8, nth block (8 bytes per block)

        // This can be optimised further by reducing the shifts
        // Mask to find the bit and shift afterwards to half the shift ops
        pt[blk + 0] = (s[0]  >> i & 0x1) << 0 | (s[1]  >> i & 0x1) << 1 |
                      (s[2]  >> i & 0x1) << 2 | (s[3]  >> i & 0x1) << 3 |
                      (s[4]  >> i & 0x1) << 4 | (s[5]  >> i & 0x1) << 5 |
                      (s[6]  >> i & 0x1) << 6 | (s[7]  >> i & 0x1) << 7;
        pt[blk + 1] = (s[8]  >> i & 0x1) << 0 | (s[9]  >> i & 0x1) << 1 |
                      (s[10] >> i & 0x1) << 2 | (s[11] >> i & 0x1) << 3 |
                      (s[12] >> i & 0x1) << 4 | (s[13] >> i & 0x1) << 5 |
                      (s[14] >> i & 0x1) << 6 | (s[15] >> i & 0x1) << 7;
        pt[blk + 2] = (s[16] >> i & 0x1) << 0 | (s[17] >> i & 0x1) << 1 |
                      (s[18] >> i & 0x1) << 2 | (s[19] >> i & 0x1) << 3 |
                      (s[20] >> i & 0x1) << 4 | (s[21] >> i & 0x1) << 5 |
                      (s[22] >> i & 0x1) << 6 | (s[23] >> i & 0x1) << 7;
        pt[blk + 3] = (s[24] >> i & 0x1) << 0 | (s[25] >> i & 0x1) << 1 |
                      (s[26] >> i & 0x1) << 2 | (s[27] >> i & 0x1) << 3 |
                      (s[28] >> i & 0x1) << 4 | (s[29] >> i & 0x1) << 5 |
                      (s[30] >> i & 0x1) << 6 | (s[31] >> i & 0x1) << 7;
        pt[blk + 4] = (s[32] >> i & 0x1) << 0 | (s[33] >> i & 0x1) << 1 |
                      (s[34] >> i & 0x1) << 2 | (s[35] >> i & 0x1) << 3 |
                      (s[36] >> i & 0x1) << 4 | (s[37] >> i & 0x1) << 5 |
                      (s[38] >> i & 0x1) << 6 | (s[39] >> i & 0x1) << 7;
        pt[blk + 5] = (s[40] >> i & 0x1) << 0 | (s[41] >> i & 0x1) << 1 |
                      (s[42] >> i & 0x1) << 2 | (s[43] >> i & 0x1) << 3 |
                      (s[44] >> i & 0x1) << 4 | (s[45] >> i & 0x1) << 5 |
                      (s[46] >> i & 0x1) << 6 | (s[47] >> i & 0x1) << 7;
        pt[blk + 6] = (s[48] >> i & 0x1) << 0 | (s[49] >> i & 0x1) << 1 |
                      (s[50] >> i & 0x1) << 2 | (s[51] >> i & 0x1) << 3 |
                      (s[52] >> i & 0x1) << 4 | (s[53] >> i & 0x1) << 5 |
                      (s[54] >> i & 0x1) << 6 | (s[55] >> i & 0x1) << 7;
        pt[blk + 7] = (s[56] >> i & 0x1) << 0 | (s[57] >> i & 0x1) << 1 |
                      (s[58] >> i & 0x1) << 2 | (s[59] >> i & 0x1) << 3 |
                      (s[60] >> i & 0x1) << 4 | (s[61] >> i & 0x1) << 5 |
                      (s[62] >> i & 0x1) << 6 | (s[63] >> i & 0x1) << 7;
    }
}