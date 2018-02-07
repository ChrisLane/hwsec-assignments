#include "crypto.h"

/**
 * The addRoundKey step XORs each bit of the state with the bit of the key in the same position.
 *
 * @param s     The state.
 * @param key   The round key.
 */
static void add_round_key(uint8_t s[CRYPTO_IN_SIZE], const uint8_t key[CRYPTO_KEY_SIZE]) {
    // Loop for the size of the state
    for (uint8_t i = 0; i < CRYPTO_IN_SIZE; i++) {
        // XOR state bit with key bit in position.
        s[i] ^= key[i];
    }
}

/**
 * Values of the S box.
 */
static const uint8_t sbox[256] = {
        0xCC, 0xC5, 0xC6, 0xCB, 0xC9, 0xC0, 0xCA, 0xCD, 0xC3, 0xCE, 0xCF, 0xC8, 0xC4, 0xC7, 0xC1, 0xC2,
        0x5C, 0x55, 0x56, 0x5B, 0x59, 0x50, 0x5A, 0x5D, 0x53, 0x5E, 0x5F, 0x58, 0x54, 0x57, 0x51, 0x52,
        0x6C, 0x65, 0x66, 0x6B, 0x69, 0x60, 0x6A, 0x6D, 0x63, 0x6E, 0x6F, 0x68, 0x64, 0x67, 0x61, 0x62,
        0xBC, 0xB5, 0xB6, 0xBB, 0xB9, 0xB0, 0xBA, 0xBD, 0xB3, 0xBE, 0xBF, 0xB8, 0xB4, 0xB7, 0xB1, 0xB2,
        0x9C, 0x95, 0x96, 0x9B, 0x99, 0x90, 0x9A, 0x9D, 0x93, 0x9E, 0x9F, 0x98, 0x94, 0x97, 0x91, 0x92,
        0x0C, 0x05, 0x06, 0x0B, 0x09, 0x00, 0x0A, 0x0D, 0x03, 0x0E, 0x0F, 0x08, 0x04, 0x07, 0x01, 0x02,
        0xAC, 0xA5, 0xA6, 0xAB, 0xA9, 0xA0, 0xAA, 0xAD, 0xA3, 0xAE, 0xAF, 0xA8, 0xA4, 0xA7, 0xA1, 0xA2,
        0xDC, 0xD5, 0xD6, 0xDB, 0xD9, 0xD0, 0xDA, 0xDD, 0xD3, 0xDE, 0xDF, 0xD8, 0xD4, 0xD7, 0xD1, 0xD2,
        0x3C, 0x35, 0x36, 0x3B, 0x39, 0x30, 0x3A, 0x3D, 0x33, 0x3E, 0x3F, 0x38, 0x34, 0x37, 0x31, 0x32,
        0xEC, 0xE5, 0xE6, 0xEB, 0xE9, 0xE0, 0xEA, 0xED, 0xE3, 0xEE, 0xEF, 0xE8, 0xE4, 0xE7, 0xE1, 0xE2,
        0xFC, 0xF5, 0xF6, 0xFB, 0xF9, 0xF0, 0xFA, 0xFD, 0xF3, 0xFE, 0xFF, 0xF8, 0xF4, 0xF7, 0xF1, 0xF2,
        0x8C, 0x85, 0x86, 0x8B, 0x89, 0x80, 0x8A, 0x8D, 0x83, 0x8E, 0x8F, 0x88, 0x84, 0x87, 0x81, 0x82,
        0x4C, 0x45, 0x46, 0x4B, 0x49, 0x40, 0x4A, 0x4D, 0x43, 0x4E, 0x4F, 0x48, 0x44, 0x47, 0x41, 0x42,
        0x7C, 0x75, 0x76, 0x7B, 0x79, 0x70, 0x7A, 0x7D, 0x73, 0x7E, 0x7F, 0x78, 0x74, 0x77, 0x71, 0x72,
        0x1C, 0x15, 0x16, 0x1B, 0x19, 0x10, 0x1A, 0x1D, 0x13, 0x1E, 0x1F, 0x18, 0x14, 0x17, 0x11, 0x12,
        0x2C, 0x25, 0x26, 0x2B, 0x29, 0x20, 0x2A, 0x2D, 0x23, 0x2E, 0x2F, 0x28, 0x24, 0x27, 0x21, 0x22
};

/**
 * Values of the P box.
 */
static const uint8_t pbox[64] = {
        0, 16, 32, 48, 1, 17, 33, 49,
        2, 18, 34, 50, 3, 19, 35, 51,
        4, 20, 36, 52, 5, 21, 37, 53,
        6, 22, 38, 54, 7, 23, 39, 55,
        8, 24, 40, 56, 9, 25, 41, 57,
        10, 26, 42, 58, 11, 27, 43, 59,
        12, 28, 44, 60, 13, 29, 45, 61,
        14, 30, 46, 62, 15, 31, 47, 63
};

/**
 * Get values of applying the sbox to the state.
 * 
 * @param s The state.
 */
static void sbox_layer(uint8_t s[CRYPTO_IN_SIZE]) {
    for (uint8_t i = 0; i < CRYPTO_IN_SIZE; i++) {
        s[i] = sbox[s[i]];
    }
}

/**
 * Get values of applying the pbox to the state.
 * 
 * @param s The state.
 */
static void pbox_layer(uint8_t s[CRYPTO_IN_SIZE]) {
    // Initialise output to 0
    uint8_t out[CRYPTO_IN_SIZE] = {0};

    // Loop through size of input
    for (uint8_t byt = 0; byt < CRYPTO_IN_SIZE; byt++) {
        // Loop through each bit in the byt
        for (uint8_t bit = 0; bit < 8; bit++) {
            // Get pbox result after inputting current bit
            uint8_t output_bit = pbox[(byt << 3) + bit];
            // Take byte from input, shifting by current bit number, using bitwise AND
            uint8_t input_bit = (uint8_t) ((s[byt] >> bit) & 0x1);
            // Move input bit to output bit position and set output
            //__asm__("BIS.B %1, %0": "=r" (out[output_bit >> 3]) : "r" (input_bit));
            out[output_bit >> 3] |= input_bit << (output_bit % 8);
        }
    }

    // Copy result back to s
    for (uint8_t i = 0; i < CRYPTO_IN_SIZE; i++)
        s[i] = out[i];
}

static void update_round_key(uint8_t key[CRYPTO_KEY_SIZE], const uint8_t r) {
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

void crypto_func(uint8_t pt[CRYPTO_IN_SIZE], uint8_t key[CRYPTO_KEY_SIZE]) {

    for (uint8_t i = 1; i < 32; i++) {
        // Note +2 offset on key since output of key schedule are upper 8 byte
        add_round_key(pt, key + 2);
        sbox_layer(pt);
        pbox_layer(pt);
        update_round_key(key, i);
    }

    // Note +2 offset on key since output of key schedule are upper 8 byte
    add_round_key(pt, key + 2);
}
