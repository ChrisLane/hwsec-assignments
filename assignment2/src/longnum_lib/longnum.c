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

#include "longnum.h"
#include <stdbool.h>
#include <sys/param.h>


// ln_equal and ln_equal_zero could both be computed recursively which would
// be more readable however it would be slower unless tail-recursion is optimised
// out into a loop, as used below.

ln_limb_t ln_equal(const ln_limb_t* a, const ln_limb_t* b, const size_t n) {
    bool unequal = false;
    for (ln_limb_t i = 0; i < n; i++)
        unequal |= (a[i] != b[i]);

    // false is 0, true is 1
    // cast to shut up the compiler
    return (uint8_t) unequal;
}

ln_limb_t ln_equal_zero(const ln_limb_t* a, const size_t n) {
    bool nonzero = false;
    for (ln_limb_t i = 0; i < n; i++)
        nonzero |= (a[i] != 0);

    // false is 0
    // cast to shut up the compiler
    return (uint8_t) nonzero;
}

int8_t ln_compare(const ln_limb_t* a, const ln_limb_t* b, const size_t n) {
    for (ln_limb_t i = 0; i < n; i++)
        if (a[i] != b[i])
            return (a[i] > b[i]) ? 1U : -1;
    return 0;
}

size_t ln_from_bytes(const uint8_t* in, const size_t in_cnt, ln_limb_t* a,
                     const size_t n) {

    // Don't assume the bytes fit into given limbs
    size_t maxlimbs = MIN(in_cnt / BYTES_PER_LIMB, n);

    // for n limbs
    for (size_t i = 0; i < maxlimbs * BYTES_PER_LIMB; i++) {
        ln_limb_t *limb = &a[i / BYTES_PER_LIMB];
        ((uint8_t *)limb)[i % BYTES_PER_LIMB] = in[i];
    }
    return maxlimbs;
}

void ln_clear(ln_limb_t* a, const size_t n) {
    for (ln_limb_t i = 0; i < n; i++)
        a[i] = 0;
}

void ln_assign(ln_limb_t* a, const ln_limb_t* b, const size_t n) {
    for (ln_limb_t i = 0; i < n; i++)
        a[i] = b[i];
}

ln_limb_t ln_add(const ln_limb_t* a, const ln_limb_t* b, ln_limb_t* c, const size_t n) {
    uint8_t carry = 0;
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i] + carry;
        // Carry bit is set if result is less than operands.
        // The result will be smaller if the number has overflowed.
        carry = (uint8_t) ((c[i] <= a[i] && c[i] <= b[i]) ? 1 : 0);
    }
    return carry;
}

ln_limb_t ln_sub(const ln_limb_t* a, const ln_limb_t* b, ln_limb_t* c, const size_t n) {
    return 0;
}

ln_limb_t ln_add_single_limb(const ln_limb_t* a, const ln_limb_t bl, ln_limb_t* c, const size_t n) {
    return 0;
}

void ln_multiply(const ln_limb_t* a, const ln_limb_t* b, ln_limb_t* c,
                 const size_t n) {
}

void ln_square(const ln_limb_t* a, ln_limb_t* c, const size_t n) {
}

void ln_multiply_single_limb(const ln_limb_t* a, const ln_limb_t b, ln_limb_t* c,
                             const size_t n) {
}

int8_t ln_shift_word_left(const ln_limb_t* a, const size_t s, ln_limb_t* c,
                          const size_t n_a, const size_t n_c) {
    return 0;
}

int8_t ln_mod_barrett(const ln_limb_t* a, const ln_limb_t* b, const ln_limb_t* mu,
                      const size_t n, ln_limb_t* r, ln_limb_t* sp, const size_t n_sp) {
    return 0;
}
