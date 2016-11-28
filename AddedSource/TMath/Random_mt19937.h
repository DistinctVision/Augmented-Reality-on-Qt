#ifndef TMATH_RANDOM_MT19937_H
#define TMATH_RANDOM_MT19937_H

namespace TMath {

class Random_mt19937
{
public:
    Random_mt19937() { seed(5489U); }
    Random_mt19937(unsigned s) { seed(s); }
    void seed(unsigned s)
    {
        state[0]= s;
        for (mti = 1; mti < N; mti++) {
            /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
            state[mti] = (1812433253U * (state[mti - 1] ^ (state[mti - 1] >> 30)) + mti);
        }
    }

    unsigned next()
    {
        /* mag01[x] = x * MATRIX_A  for x=0,1 */
        static unsigned mag01[2] = { 0x0U, /*MATRIX_A*/ 0x9908b0dfU};

        const unsigned UPPER_MASK = 0x80000000U;
        const unsigned LOWER_MASK = 0x7fffffffU;

        /* generate N words at one time */
        if (mti >= N) {
            int kk = 0;

            for (; kk < N - M; ++kk) {
                unsigned y = (state[kk] & UPPER_MASK) | (state[kk + 1] & LOWER_MASK);
                state[kk] = state[kk + M] ^ (y >> 1) ^ mag01[y & 0x1U];
            }

            for (; kk < N - 1; ++kk) {
                unsigned y = (state[kk] & UPPER_MASK) | (state[kk + 1] & LOWER_MASK);
                state[kk] = state[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1U];
            }

            unsigned y = (state[N - 1] & UPPER_MASK) | (state[0] & LOWER_MASK);
            state[N - 1] = state[M - 1] ^ (y >> 1) ^ mag01[y & 0x1U];

            mti = 0;
        }

        unsigned y = state[mti++];

        /* Tempering */
        y ^= (y >> 11);
        y ^= (y <<  7) & 0x9d2c5680U;
        y ^= (y << 15) & 0xefc60000U;
        y ^= (y >> 18);

        return y;
    }

    operator int() { return static_cast<int>(next()); }
    operator unsigned() { return next(); }
    operator float() { return next() * (1.0f / 4294967296.f); }
    operator double()
    {
        unsigned a = next() >> 5;
        unsigned b = next() >> 6;
        return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0);
    }

    unsigned operator ()(unsigned b) { return next() % b; }
    unsigned operator ()() { return next(); }

    int uniform(int a, int b) { return static_cast<int>((next() % (b - a) + a)); }

    float uniform(float a, float b) { return ((float)*this)*(b - a) + a; }

    double uniform(double a, double b) { return ((double)*this)*(b - a) + a; }

private:
    enum PeriodParameters { N = 624, M = 397 };
    unsigned state[N];
    int mti;
};

}

#endif //TMATH_RANDOM_MT19937_H
