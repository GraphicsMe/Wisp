/*
 * =======================================================================
 *   WARNING    WARNING    WARNING    WARNING    WARNING    WARNING
 * =======================================================================
 *   Remember to put on SAFETY GOGGLES before looking at this file. You
 *   are most certainly not expected to read or understand any of it.
 * =======================================================================
 */

/*
   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

#include "random.h"

WISP_NAMESPACE_BEGIN

Random::Random() {
    m_mti = MT_N+1;
}

/* initializes mt[MT_N] with a seed */
void Random::seed(uint32_t s) {
    m_mt[0] = s & 0xffffffffUL;
    for (m_mti = 1; m_mti < MT_N; m_mti++) {
        m_mt[m_mti] =
            (1812433253UL * (m_mt[m_mti-1] ^ (m_mt[m_mti-1] >> 30)) + m_mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].						*/
        /* 2002/01/09 modified by Makoto Matsumoto			 */
        m_mt[m_mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void Random::seed(uint32_t *values, int length) {
    int i, j, k;
    seed(19650218UL);
    i=1; j=0;
    k = (MT_N>length ? MT_N : length);
    for (; k; k--) {
        m_mt[i] = (m_mt[i] ^ ((m_mt[i-1] ^ (m_mt[i-1] >> 30)) * 1664525UL))
          + values[j] + j; /* non linear */
        m_mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=MT_N) { m_mt[0] = m_mt[MT_N-1]; i=1; }
        if (j>=length) j=0;
    }
    for (k=MT_N-1; k; k--) {
        m_mt[i] = (m_mt[i] ^ ((m_mt[i-1] ^ (m_mt[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        m_mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=MT_N) { m_mt[0] = m_mt[MT_N-1]; i=1; }
    }

    m_mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
}

void Random::seed(Random *random) {
    uint32_t buf[MT_N];
    for (int i=0; i<MT_N; ++i)
        buf[i] = random->nextUInt();
    seed(buf, MT_N);
}

/* generates a random number on [0,0xffffffff]-interval */
uint32_t Random::nextUInt() {
    uint32_t y;
    static uint32_t mag01[2]={0x0UL, MT_MATRIX_A};
    /* mag01[x] = x * MT_MATRIX_A  for x=0,1 */

    if (m_mti >= MT_N) { /* generate MT_N words at one time */
        int kk;

        if (m_mti == MT_N+1)   /* if seed() has not been called, */
            seed(5489UL);   /* a default initial seed is used */

        for (kk=0;kk<MT_N-MT_M;kk++) {
            y = (m_mt[kk] & MT_UPPER_MASK)|(m_mt[kk+1] & MT_LOWER_MASK);
            m_mt[kk] = m_mt[kk+MT_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<MT_N-1;kk++) {
            y = (m_mt[kk] & MT_UPPER_MASK)|(m_mt[kk+1] & MT_LOWER_MASK);
            m_mt[kk] = m_mt[kk+(MT_M-MT_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (m_mt[MT_N-1] & MT_UPPER_MASK)|(m_mt[0] & MT_LOWER_MASK);
        m_mt[MT_N-1] = m_mt[MT_M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        m_mti = 0;
    }

    y = m_mt[m_mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}

float Random::nextFloat() {
    /* Trick from MTGP: generate an uniformly distributed
       single precision number in [1,2) and subtract 1. */
    union {
        uint32_t u;
        float f;
    } x;
    x.u = (nextUInt() >> 9) | 0x3f800000UL;
    return x.f - 1.0f;
}

WISP_NAMESPACE_END

