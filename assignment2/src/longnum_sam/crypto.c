#include "crypto.h"

void crypto_func(ln_limb_t x[CRYPTO_IN_SIZE_WORDS], ln_limb_t n[CRYPTO_IN_SIZE_WORDS],
                 ln_limb_t ln_mu[CRYPTO_IN_SIZE_WORDS + 1], const ln_limb_t exp,
                 ln_limb_t y[CRYPTO_IN_SIZE_WORDS]) {
    // Helper variables
    ln_limb_t tmp[2 * CRYPTO_IN_SIZE_WORDS], scratch[2 * CRYPTO_IN_SIZE_WORDS + 2];

    // Length of exponent in bit
    int8_t exp_len = 15;

    // Determine MSBit position
    while (exp_len > 0 && (exp & (1 << exp_len)) == 0) {
        exp_len--;
    }

    // y <- x
    ln_assign(y, x, CRYPTO_IN_SIZE_WORDS);

    // Lecture notes say "i = t - 2" here, somehow that didn't work but "i = t - 1" did
    for (int i = exp_len - 1; i >= 0; i--) {
        // x <- x^2 mod n
        ln_multiply(y, y, tmp, CRYPTO_IN_SIZE_WORDS);
        ln_mod_barrett(tmp, n, ln_mu, CRYPTO_IN_SIZE_WORDS, y, scratch, 2 * CRYPTO_IN_SIZE_WORDS + 2);

        // Check if ei == 1
        if (((exp << i) & 0x1) == 1) {
            // y <- y * x mod n
            ln_multiply(y, x, tmp, CRYPTO_IN_SIZE_WORDS);
            ln_mod_barrett(tmp, n, ln_mu, CRYPTO_IN_SIZE_WORDS, y, scratch, 2 * CRYPTO_IN_SIZE_WORDS + 2);
        }
    }
}
