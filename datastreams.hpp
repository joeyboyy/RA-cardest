#pragma once

#include <random>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

std::mt19937 ds_rng(*(int*)"shhh");

/**
 * Generate a synthetic datastream following a Zipfian law.
 * 
 * out_Z     datastream Z, is overwritten
 * stream_length    length of stream N
 * universe_size    size of distinct elements n (universe impicitely is {0,...,n-1})
 * alpha            parameter of Zipfian law
 */
inline void generate_zipfian(std::vector<int>& out_Z, int stream_length, int universe_size, float alpha)
{
    /* prepare weights */
    std::vector<float> weights;
    weights.resize(universe_size);
    for (int i = 0; i < universe_size; i++)
    {
        weights[i] = std::pow((float)i, -alpha); // scaling is implicit in discrete_distribution
    }

    std::discrete_distribution<int> d(weights.begin(), weights.end());

    /* generate random instances and populate Z */
    out_Z.clear();
    out_Z.resize(stream_length);
    for (int j = 0; j < stream_length; j++)
    {
        out_Z[j] = d(ds_rng);
    }
}

/**
 * Read a data stream (of words) from a file.
 * 
 * out_Z        datastream Z, is overwritten
 * filepath     path to file
 */
inline void read_stream(std::vector<std::string>& out_Z, std::string filepath)
{
    std::ifstream datastream(filepath, std::ios_base::in);
    if (!datastream.is_open())
    {
        std::cerr << "Couldn't open data stream input file!\n";
        throw;
    }

    out_Z.clear();
    std::string z;
    while (datastream >> z)
    {
        out_Z.push_back(z);
    }
}