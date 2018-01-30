#ifndef __longnum_h
#define __longnum_h

/**

 Simple, self-contained libraries for basic long integer math
 
 This library is to be used mainly for educational purposes. It may
 be inefficient and missing functionality. It also contains non-constant-
 time operations and timing leaks. 
 
 ONLY USE WHEN YOU KNOW WHAT YOU ARE DOING!
 
 If you are looking for secure, efficient, modern crypto for embedded
 devices in real applications, consider using Curve25519:
 
 http://munacl.cryptojedi.org/
 
 By David Oswald, d.f.oswald@cs.bham.ac.uk
 26 November 2015
 
 =======================================================================
 
 This is free and unencumbered software released into the public domain.
 
 Anyone is free to copy, modify, publish, use, compile, sell, or
 distribute this software, either in source code form or as a compiled
 binary, for any purpose, commercial or non-commercial, and by any
 means.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 
 =======================================================================
 
 If this software is useful to you, I'd appreciate an attribution,
 contribution (e.g. bug fixes, improvements, ...), or a beer.

**/

#include <stdint.h>
#include <stdlib.h>

typedef uint16_t ln_limb_t;
typedef uint32_t ln_doublelimb_t;

#define LIMB_MASK 0xFFFF
#define LIMB_MAX (LIMB_MASK-1)

#define BYTES_PER_LIMB 2
#define BITS_PER_LIMB (BYTES_PER_LIMB * 8)

/**
 * Compare two LNs for equality (constant time)
 *
 * @param a First LN
 * @param b Second LN
 * @param n Size of a and b in limbs
 * @return 0 if a == b, else != 0
 */
ln_limb_t ln_equal(const ln_limb_t* a, const ln_limb_t* b, const size_t n);

/**
 * Compare LN for equality to zero (constant time)
 *
 * @param a First LN
 * @param n Size of a in limbs
 * @return 0 if a == 0, else != 0
 */
ln_limb_t ln_equal_zero(const ln_limb_t* a, const size_t n);

/**
 * Compare two LNs (non-constant time, early abort)
 *
 * @param a First LN
 * @param b Second LN
 * @param n Size of a and b in limbs
 * @return 0 if a == b, 1 if a > b, -1 if a < b
 */
int8_t ln_compare(const ln_limb_t* a, const ln_limb_t* b, const size_t n);

/**
 * Assign LN from little-endian byte array (ie first byte is LSByte of output)
 *
 * @param in Input array
 * @param in_cnt Size of in in byte
 * @param a Output LN
 * @param n Size of a in limbs
 * @return Number of limbs written
 */
size_t ln_from_bytes(const uint8_t* in, const size_t in_cnt, ln_limb_t* a, const size_t n);

/**
 * Set a LN to zero
 *
 * @param a LN to zeroize
 * @param n Size of a in limbs
 */
void ln_clear(ln_limb_t* a, const size_t n);

/**
 * Set a <- b
 *
 * @param a Target LN
 * @param b Source LN
 * @param n Size of a, b in limbs
 */
void ln_assign(ln_limb_t* a, const ln_limb_t* b, const size_t n);

/**
 * Add two LNs
 *
 * @param a First LN
 * @param b Second LN
 * @param c Output LN, c = a + b
 * @param n Size of a, b, c in limbs
 * @return Carry of addition of most significant limbs
 */
ln_limb_t ln_add(const ln_limb_t* a, const ln_limb_t* b, ln_limb_t* c, const size_t n);

/**
 * Subtract two LNs (using 2's complement)
 *
 * @param a First LN
 * @param b Second LN
 * @param c Output LN, c = a - b
 * @param n Size of a, b, c in limbs
 * @return Borrow of addition of most significant limbs (Internally uses a + (~b + 1))
 */
ln_limb_t ln_sub(const ln_limb_t* a, const ln_limb_t* b, ln_limb_t* c, const size_t n);

/**
 * Add a single limb to a LN
 *
 * @param a Input LN
 * @param bl Limb to add
 * @param c Output LN, c = a + bl
 * @param n Size of a, c in limbs
 * @return Carry of addition of most significant limbs
 */
ln_limb_t ln_add_single_limb(const ln_limb_t* a, const ln_limb_t bl, ln_limb_t* c, const size_t n);

/**
 * Multiply two LNs (schoolbook algorithm)
 *
 * @param a First LN
 * @param b Second LN
 * @param c Output LN, c = a * b, has to have size 2n
 * @param n Size of a, b in limbs
 */
void ln_multiply(const ln_limb_t* a, const ln_limb_t* b, ln_limb_t* c, const size_t n);

/**
 * Square a LN (schoolbook algorithm)
 *
 * @param a LN
 * @param c Output LN, c = a * a, has to have size 2n
 * @param n Size of a in limbs
 */
void ln_square(const ln_limb_t* a, ln_limb_t* c, const size_t n);

/**
 * Multiply LN with single limb (schoolbook algorithm)
 *
 * @param a First LN
 * @param b Limb to multiply
 * @param c Output LN, c = a * b, has to have size n + 1
 * @param n Size of a in limbs
 */
void ln_multiply_single_limb(const ln_limb_t* a, const ln_limb_t b, ln_limb_t* c, const size_t n);

/**
 * Shift a LN left by a number of words (i.e. * b^s)
 *
 * @param a First LN, size n_a
 * @param s Number of word positions to shift by
 * @param c Output LN, c = a << 16*s, has to at least have size n_a + s
 * @param n_a Size of a in limbs
 * @param n_c Size of c in limbs, has to be at least n_a + s
 * @return 0 on success, otherwise != 0
 */
int8_t ln_shift_word_left(const ln_limb_t* a, const size_t s, ln_limb_t* c, const size_t n_a, const size_t n_c);

/**
 * An LN modulo another LN, using Barrett reduction
 *
 * @param a First LN, size 2*n
 * @param b Second LN, size n
 * @param mu Barret value mu = floor(base^2n/b), size n + 1
 * @param n Size of b in limbs
 * @param r Output LN, r = a mod b, has to at least have size n
 * @param sp Scratchpad LN, has to at least have size n_sp = 2*n + 2*n
 * @param n_sp Size of sp in limbs
 * @return 0 on success, otherwise != 0
 */
int8_t ln_mod_barrett(const ln_limb_t* a, const ln_limb_t* b, const ln_limb_t* mu, const size_t n, ln_limb_t* r, ln_limb_t* sp, const size_t n_sp);

#endif