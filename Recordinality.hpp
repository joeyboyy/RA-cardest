#pragma once

#include <vector>
#include <cmath>
#include "clhash/clhash.h"

/**
 * Check if key y is distinct from S[0], ..., S[k_part-1].
 * (k_part non-negative)
 * 
 * Return value:
 * not distinct     <  0
 * yes distinct     >= 0
 */
template <typename hash_type>
inline int is_distinct(const hash_type *S, int k_part, hash_type y)
{
    for (int i = 0; i < k_part; i++)
    {
        if (S[i] == y)  return -1;
    }
    return k_part;
}

/**
 * Deprecated (performance reasons)
 * Check if key y is distinct from and greater than smallest element in S.
 * (S of positive size k > 0)
 * 
 * Return value:
 * not distinct     < 0
 * yes distinct     index of smallest element in S
 */
template <typename hash_type>
inline int depr_is_distinct_k_record(const hash_type *S, int k, hash_type y)
{
    hash_type min = S[0];
    int min_idx = 0;
    for (int i = 1; i < k; i++)
    {
        if (S[i] < min) 
        {
            min = S[i];
            min_idx = i;
        }
        if (S[i] == y)  return -1;
    }
    // WTF IS GOING ON?? to do
    // the string "ingenioso" is greater than "el", and accordingly,
    // std::string("ingenioso") <= std::string("el") evaluates to false,
    // but when y is "ingenioso" and min is "el",
    // y <= min evaluates to true ????????
    // \357 \273 \277 byte ordering is problem
    //The sequence \357 \273 \277 indicates UTF-8 is following. Which does not need to take byte-ordering into account, as the byte is the unit for such files.
    if (!(y > min))
        return -1;
    return min_idx;
}


/* static variable needed in is_distinct_k_record */
static uint64_t minS; /* invariant: minimum in S */
static int  minS_idx; /* invariant: index of minimum */

/**
 * Check if key y is distinct from and greater than smallest element in S.
 * (S of positive size k > 0)
 * 
 * WEIRD CALLING CONDITIONS(!):
 * - before first call, initialize_minS needs to be called
 * - on every call returning yes, the element at the index returned needs to be consequently 
 * - overwritten with y (remains because of function interface backwards compatibility)
 * 
 * Return value:
 * not distinct k-record    < 0
 * yes distinct k-record    index of smallest element in S
 */
int is_distinct_k_record(const uint64_t *S, int k, uint64_t y)
{
    /* special case when k == 1 */
    if (k == 1)
        return (y > S[0] ? 0 : -1);
    
    /* if y is not greater than minimum in S, it has no chance of being a k-record */
    if (y > minS)
    {
        /* check if y is present in S, alongside find second smallest element in S */
        uint64_t min2 = 0xffffffff'ffffffff;
        int  min2_idx = -1;
        for (int i = 0; i < k; i++)
        {
            uint64_t Si = S[i];
            if (Si < min2 && Si != minS) 
            {
                min2 = Si;
                min2_idx = i;
            }
            if (Si == y)  return -1;
        }
        /* y is distinct k-record: return index of minimum and update static vars minS, minS_idx */
        int ret = minS_idx;
        if (y < min2) {
            minS = y;
            //minS_idx = minS_idx;
        }
        else {
            minS = min2;
            minS_idx = min2_idx;
        }
        return ret;
    }
    else
        return -1;
}

/**
 * Set the static variables minS and minS_idx to value resp index of minimum element in S.
 * (S of positive size k > 0)
 * 
 * see is_distinct_k_record()
 */
void initialize_minS(const uint64_t *S, int k)
{
    uint64_t min = 0xffffffff'ffffffff;
    int  min_idx = -1;
    for (int i = 0; i < k; i++)
    {
        if (S[i] < min)
        {
            min = S[i];
            min_idx = i;
        }
    }
    minS = min;
    minS_idx = min_idx;
}


/**
 * Recordinality cardinality estimation through k-records.
 * 
 * hash     hash function (instance of clhasher struct)
 * Z        data stream / multiset
 * k        k
 * 
 * Memory: k hash values (2k*logn bits) + 1 counter (loglogn bits)
 *         - 2logn bits per hash value bc to avoid collisios, we need hash universe size > n^2 ==> log(n^2) = 2log(n) bits
 */
template <typename z_type>
inline double rec(clhasher &hash, const std::vector<z_type> &Z, int k)
{
    int R = 0, j = 0;
    uint64_t *S = new uint64_t[k];

    /* fill S with the first k distinct elements (hash values) */
    for (int i = 0; i < k && j < (int)Z.size(); j++)
    {
        const uint64_t y = hash(Z[j]);
        if (is_distinct(S, i, y) >= 0)
        {
            R++;
            S[i] = y;
            i++;
        }
    }
    if (j == (int)Z.size()) // if already seen whole datastream
        return R;

    /* count (further) k-records */
    initialize_minS(S, k);
    for (; j < (int)Z.size(); j++)
    {
        const uint64_t y = hash(Z[j]);

        const int min_idx = is_distinct_k_record(S, k, y);
        if (min_idx >= 0)
        {
            R++;
            S[min_idx] = y; /* S = S + y - minS */
        }
    }

    /* by lecture: return Z := k(1+1/k)^(R-k+1) - 1 */
    double E = k*std::pow(1 + 1./k, R-k+1) - 1;

    delete[] S;
    return E;
}


/**
 * Recordinality cardinality estimation through k-records.
 * 
 * hash     hash function (instance of clhasher struct)
 * Z        data stream / multiset
 * k        k
 * 
 * Memory: k hash values (2k*logn bits) + 1 counter (loglogn bits)
 *         - 2logn bits per hash value bc to avoid collisios, we need hash universe size > n^2 ==> log(n^2) = 2log(n) bits
 */
template <typename z_type>
inline double rec_nohash(const std::vector<z_type> &Z, int k)
{
    int R = 0, j = 0;
    z_type *S = new z_type[k];

    /* fill S with the first k distinct elements (hash values) */
    for (int i = 0; i < k && j < (int)Z.size(); j++)
    {
        const z_type y = Z[j];
        if (is_distinct(S, i, y) >= 0)
        {
            R++;
            S[i] = y;
            i++;
        }
    }
    if (j == (int)Z.size()) // if already seen whole datastream
        return R;

    /* count (further) k-records */
    //initialize_minS(S, k);
    for (; j < (int)Z.size(); j++)
    {
        const z_type y = Z[j];

        const int min_idx = depr_is_distinct_k_record(S, k, y);
        if (min_idx >= 0)
        {
            R++;
            S[min_idx] = y; /* S = S + y - minS */
        }
    }

    /* by lecture: return Z := k(1+1/k)^(R-k+1) - 1 */
    double E = k*std::pow(1 + 1./k, R-k+1) - 1;

    delete[] S;
    return E;
}
