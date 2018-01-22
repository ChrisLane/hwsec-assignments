#ifndef __CRYPTO_H
#define __CRYPTO_H

#include <msp430.h>
#include <string.h>
#include <driverlib.h>

#include <longnum.h>

// Define basic parameters
#define CRYPTO_IN_SIZE      128                                 // in byte
#define CRYPTO_IN_SIZE_WORDS  (CRYPTO_IN_SIZE/BYTES_PER_LIMB)   // in words
// Block size in bit
#define CRYPTO_IN_SIZE_BIT (CRYPTO_IN_SIZE * 8)

// The function to test
void crypto_func(ln_limb_t ln_a[CRYPTO_IN_SIZE_WORDS], ln_limb_t ln_b[CRYPTO_IN_SIZE_WORDS],
                 ln_limb_t ln_c[CRYPTO_IN_SIZE_WORDS]);

#endif