#pragma once

#include "clhash/clhash.h"
#include <vector>
#include <cstring>
#include <cstdint>
#include <cassert>

/**
 * 2^x for non-negative integers
 */
template <typename integer_type>
constexpr integer_type uiexp2(integer_type x)
{
    return ((integer_type)1 << x);
}

/**
 * Count number of leading 0's in y, (result undefined if y == 0).
 */
constexpr uint64_t lzcnt(uint64_t y)
{
    return __builtin_clzll(y);
}

/**
 * Correction factor alpha_m from HLL paper, for m > 1 a power of two
 */
constexpr double alpha(int m)
{
    assert ((m & (m - 1)) == 0); assert (m > 1);
    switch (m)
    {
    case  2: return 0.351; // WolframAlpha
    case  4: return 0.532; // WolframAlpha
    case  8: return 0.626; // WolframAlpha
    case 16: return 0.673; // FlFuGaMe07 == WolframAlpha
    case 32: return 0.697; // FlFuGaMe07 == WolframAlpha
    case 64: return 0.709; // FlFuGaMe07
    default: return 0.7213/(1.+1.079/m); // FlFuGaMe07
    }
}


/**
 * HyperLogLog cardinality estimation, using stochastic averaging with m = 2^(logm) substreams.
 * 
 * hash     hash function (instance of clhasher struct)
 * Z        data stream / multiset
 * logm     log(m), non-negative
 * 
 * Memory: Expected m*loglogm bits
 */
template <typename z_type>
inline double hll(clhasher &hash, const std::vector<z_type> &Z, int logm)
{
    const int m = uiexp2(logm);
    const uint64_t mask = m - 1;

    /* 8 bits for R --> supports up to 255 leading zeros in hash values */
    uint8_t *R = new uint8_t[m];
    std::memset(R, 0, m*sizeof(uint8_t));

    /* assert that P(exists z : h(z) == 0) <= Z.size() * (1/2)^{effective bits of hash} < 1 in a billion */
    assert(Z.size() * 1000000000 / 2 < uiexp2<size_t>(64 - 1 - logm) && "Don't like my chances of not having enough bits in hash.");

    for (int j = 0; j < (int)Z.size(); j++)
    {
        const uint64_t y = hash(Z[j]);
        const uint64_t y_up  = (y & mask);
        const uint64_t y_low = (y & ~mask);
        if (y_low == 0) {std::cerr<<"HLL FAILURE: Not enough bits in hash!\n"; throw;}

        const uint64_t p = lzcnt(y) + 1;
        if (p > R[y_up])
        {
            R[y_up] = (uint8_t)p;
        }
    }

    /* by FlFuGaMe07: compute Z := ( sum_ 2^(-R[k]) )^-1 */
    double E = 0.0;
    for (int k = 1; k < m; k++)
    {
        const uint64_t tmp = uiexp2<uint64_t>(R[k]);
        E += 1./tmp;
    }
    E = 1./E;
    /* by FlFuGaMe07: "raw" HLL estimate: E := alpha_m * m^2 * Z */
    E = alpha(m) * m*m * E;
    /* note that we're not doing small/large range corrections */
    
    delete[] R;
    return E;
}