#include "crypto.h"

/**
 * Add two LNs
 *
 * @param a First LN
 * @param b Second LN
 * @param c Output LN, c = a + b
 * @param n Size of a, b, c in limbs
 * @return Carry of addition of most significant limbs
 */
static ln_limb_t my_ln_add(const ln_limb_t* a, const ln_limb_t* b, ln_limb_t* c, const size_t n)
{
	// Insert your code here

	return 0;
}

void crypto_func(ln_limb_t ln_a[CRYPTO_IN_SIZE_WORDS], ln_limb_t ln_b[CRYPTO_IN_SIZE_WORDS], ln_limb_t ln_c[CRYPTO_IN_SIZE_WORDS])
{
	my_ln_add(ln_a, ln_b, ln_c, CRYPTO_IN_SIZE_WORDS);
}