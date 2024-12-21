#pragma once

#include <vector>
#include <unordered_set>
#include <string>

/**
 * Cardinality of data stream / multiset Z (is whole number).
 */
template <typename z_type>
inline double cardinality(const std::vector<z_type> &Z)
{
    int cardinality = 0;
    std::unordered_set<z_type> Zprime;
    
    for (int j = 0; j < (int)Z.size(); j++)
    {
        auto ins = Zprime.insert(Z[j]);
        if (ins.second == true) cardinality++;
    }
    return cardinality;
}
