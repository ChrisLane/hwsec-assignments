#include "crypto.h"

void crypto_func(ln_limb_t ln_s[CRYPTO_IN_SIZE_WORDS], ln_limb_t ln_n[CRYPTO_IN_SIZE_WORDS], ln_limb_t ln_mu[CRYPTO_IN_SIZE_WORDS + 1], const ln_limb_t exp, ln_limb_t ln_r[CRYPTO_IN_SIZE_WORDS])
{
	// Helper variables
	ln_limb_t tmp[2*CRYPTO_IN_SIZE_WORDS], sp[2*CRYPTO_IN_SIZE_WORDS + 2];

	// Length of exponent in bit
	int8_t e_l = 15;
	
	// Determine MSBit position
	while(e_l > 0 && (exp & (1 << e_l)) == 0)
	{
		e_l--;
	}
		
	// Example code for modular reduction with Barrett method:
     	// ln_mod_barrett(tmp, ln_n, ln_mu, CRYPTO_IN_SIZE_WORDS, ln_r, sp, 2*CRYPTO_IN_SIZE_WORDS + 2);
	// Reduces the double-length long number in tmp modulo ln_n, uses the helper variable mu
	// Stores the result in ln_r
	// Requires the scratchpad variable defined above
		
	// Insert your S-a-M code here
		
}