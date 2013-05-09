#ifndef RANDOM_H
#define RANDOM_H

#include "common.h"

WISP_NAMESPACE_BEGIN

/* Period parameters for the Mersenne Twister RNG */
#define MT_N 624
#define MT_M 397
#define MT_MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define MT_UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define MT_LOWER_MASK 0x7fffffffUL /* least significant r bits */

class Random
{
public:
    Random();
    Random(Random* random);
    void seed(uint32_t value);
    void seed(uint32_t* values, int length);
    void seed(Random* random);
    uint32_t nextUInt();
    float nextFloat();

    template <typename T>
    void shuffle(T* samp, size_t count, size_t dims = 1)
    {
        for (size_t i = 0; i < count; ++i)
        {
            size_t other = i + nextUInt() % (count - i);
            for (size_t j = 0; j < dims; ++j)
                std::swap(samp[dims*i+j], samp[dims*other+j]);
        }
    }

private:
    uint32_t m_mt[MT_N];
    int m_mti;
};

WISP_NAMESPACE_END
#endif // RANDOM_H
