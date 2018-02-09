#include "crypto.h"
#include "../longnum_lib/longnum.h"

/**
 * Add two LNs
 *
 * @param a	First LN
 * @param b Second LN
 * @param c Output LN, c = a + b
 * @param n Size of a, b, c in limbs
 * @return Carry of addition of most significant limbs
 */
static ln_limb_t my_ln_add(const ln_limb_t *a, const ln_limb_t *b, ln_limb_t *c, const size_t n) {
    ln_limb_t carry = 0;
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i] + carry;
        // Carry bit is set if result is less than operands.
        // The result will be smaller if the number has overflowed.
        carry = (ln_limb_t) ((c[i] <= a[i] && c[i] <= b[i]) ? 1 : 0);
    }

    return carry;
}

void crypto_func(ln_limb_t ln_a[CRYPTO_IN_SIZE_WORDS], ln_limb_t ln_b[CRYPTO_IN_SIZE_WORDS],
                 ln_limb_t ln_c[CRYPTO_IN_SIZE_WORDS]) {
    my_ln_add(ln_a, ln_b, ln_c, CRYPTO_IN_SIZE_WORDS);
}