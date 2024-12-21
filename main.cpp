#include "datastreams.hpp"
#include "PerfectCounting.hpp"
#include "HyperLogLog.hpp"
#include "Recordinality.hpp"

#include "clhash/clhash.h"
#include <iostream>
#include <iomanip>
#include <numeric>
#include <string>

std::mt19937_64 rng(*(int*)"clha");

void real_experimets()
{
    constexpr int num_trials = 1; /* number of trials per experiment */

    std::vector<int> logm({1,5,8,13}); /* array of all values log(m) for which to run hll */
    std::vector<int> k({1,32,256});    /* array of all values k for which to run rec */

    /* "real" datasets */
    constexpr int num_datasets = 8;
    const char *datasets[num_datasets] = {"crusoe", "dracula", "iliad", "mare-balena", "midsummer-nights-dream", "quijote", "valley-fear", "war-peace"};
    double datasets_card[num_datasets];
    /* read in "real" datasets */
    std::cout << "Read in real datasets" << std::endl;
    std::vector<std::vector<std::string>> Z(num_datasets);
    for (int d = 0; d < num_datasets; d++)
    {
        std::string filename = datasets[d];
        read_stream(Z[d], "../datasets/" + filename + ".txt");
    }
    /* calculate their cardinalities */
    for (int d = 0; d < num_datasets; d++)
    {
        std::cout << "Calculate dataset cardinality " << d << std::endl;
        datasets_card[d] = cardinality(Z[d]);
    }

    /* Experiments on all "real" datasets */
    std::vector<std::vector<double>> hll_avg_rel_acc(num_datasets); /* averages per dataset per m */
    std::vector<std::vector<double>> rec_avg_rel_acc(num_datasets); /* averages per dataset per k */
    std::vector<std::vector<double>> re2_avg_rel_acc(num_datasets); /* averages per dataset per k */
    std::vector<std::vector<double>> hll_average(num_datasets); /* averages per dataset per m */
    std::vector<std::vector<double>> rec_average(num_datasets); /* averages per dataset per k */
    std::vector<std::vector<double>> re2_average(num_datasets); /* averages per dataset per k */

    for (int d = 0; d < num_datasets; d++)
    {
        std::vector<std::vector<double>> hll_estimates(logm.size()); /* results per m per trial */
        std::vector<std::vector<double>> rec_estimates(k.size());    /* results per m per trial */
        std::vector<std::vector<double>> re2_estimates(k.size());    /* results per m per trial */

        /* Run (several trials of) all estimiation algorithms on a data set */
        for (int trial = 0; trial < num_trials; trial++)
        {
            /* Instantiate a random hash function (common for all algorithm variations throughout trial) */
            clhasher h(rng(), rng()); // NOTE: clhasher is a really shitty class that doesn't properly handle memory resources 
                                     // --> do not copy, move, etc..!!!
            /* HyperLogLog */
            for (int i = 0; i < (int)logm.size(); i++)
            {
                std::cout << "Dataset " << d << " - HLL" << uiexp2(logm[i]) << std::endl;
                double estimate = hll(h, Z[d], logm[i]);
                hll_estimates[i].push_back(estimate);
            }
            /* Recordinality */
            for (int i = 0; i < (int)k.size(); i++)
            {
                std::cout << "Dataset " << d << " - REC" << k[i] << std::endl;
                double estimate = rec(h, Z[d], k[i]);
                rec_estimates[i].push_back(estimate);
            }
            /* Recordinality without hash function */
            if (trial == 0)
            {
                for (int i = 0; i < (int)k.size(); i++)
                {
                    std::cout << "Dataset " << d << " - RECnh" << k[i] << std::endl;
                    double estimate = rec_nohash(Z[d], k[i]);
                    re2_estimates[i].push_back(estimate);
                }
            }
        }

        /* Aggregate results */
        /* HyperLogLog */
        for (auto hll_m : hll_estimates)
        {
            double relerr = 0.0;
            for (auto est : hll_m)
            {
                relerr += std::abs(est - datasets_card[d]);
            }
            hll_avg_rel_acc[d].push_back(relerr/hll_m.size() / datasets_card[d]);
            hll_average[d].push_back(std::accumulate(hll_m.begin(), hll_m.end(), 0.0) / hll_m.size());
        }
        /* Recordinality */
        for (auto rec_k : rec_estimates)
        {
            double relerr = 0.0;
            for (auto est : rec_k)
            {
                relerr += std::abs(est - datasets_card[d]) / datasets_card[d];
            }
            rec_avg_rel_acc[d].push_back(relerr/rec_k.size());
            rec_average[d].push_back(std::accumulate(rec_k.begin(), rec_k.end(), 0.0) / rec_k.size());

        }
        /* Recordinality without hash function */
        for (auto re2_k : re2_estimates)
        {
            double relerr = 0.0;
            for (auto est : re2_k)
            {
                relerr += std::abs(est - datasets_card[d]) / datasets_card[d];
            }
            re2_avg_rel_acc[d].push_back(relerr/re2_k.size());
            re2_average[d].push_back(std::accumulate(re2_k.begin(), re2_k.end(), 0.0) / re2_k.size());
        }
    }

    /* Output latex table format to file */
    std::ofstream latexfile("../out/latex_table", std::ios_base::out);
    if (!latexfile.is_open())
    {
        std::cerr << "Couldn't open file for output!\n";
        throw;
    }
    latexfile << "    & ";
    for (int i = 0; i < (int)logm.size(); i++)
    {
        latexfile << " & " << uiexp2(logm[i]);
    }
    for (int i = 0; i < (int)k.size(); i++)
    {
        latexfile << " & " << k[i];
    }
    for (int i = 0; i < (int)k.size(); i++)
    {
        latexfile << " & " << k[i];
    }
    latexfile << " \\\\ \n";
    latexfile << std::fixed << std::setprecision(2);
    for (int d = 0; d < num_datasets; d++)
    {
        latexfile << datasets[d];
        latexfile << " & " << (int)datasets_card[d];
        for (int i = 0; i < (int)logm.size(); i++)
        {
            latexfile << " & " << hll_avg_rel_acc[d][i];
        }
        for (int i = 0; i < (int)k.size(); i++)
        {
            latexfile << " & " << rec_avg_rel_acc[d][i];
        }
        for (int i = 0; i < (int)k.size(); i++)
        {
            latexfile << " & " << re2_avg_rel_acc[d][i];
        }
        latexfile << " \\\\ \n";
    }
    latexfile.close();

    std::ofstream latexfile1("../out/latex_table1", std::ios_base::out);
    if (!latexfile1.is_open())
    {
        std::cerr << "Couldn't open file for output!\n";
        throw;
    }
    latexfile1 << "    & ";
    for (int i = 0; i < (int)logm.size(); i++)
    {
        latexfile1 << " & " << uiexp2(logm[i]);
    }
    for (int i = 0; i < (int)k.size(); i++)
    {
        latexfile1 << " & " << k[i];
    }
    for (int i = 0; i < (int)k.size(); i++)
    {
        latexfile1 << " & " << k[i];
    }
    latexfile1 << " \\\\ \n";
    latexfile1 << std::fixed << std::setprecision(0);
    for (int d = 0; d < num_datasets; d++)
    {
        latexfile1 << datasets[d];
        latexfile1 << " & " << (int)datasets_card[d];
        for (int i = 0; i < (int)logm.size(); i++)
        {
            latexfile1 << " & " << hll_average[d][i];
        }
        for (int i = 0; i < (int)k.size(); i++)
        {
            latexfile1 << " & " << rec_average[d][i];
        }
        for (int i = 0; i < (int)k.size(); i++)
        {
            latexfile1 << " & " << re2_average[d][i];
        }
        latexfile1 << " \\\\ \n";
    }
    latexfile1.close();

}


void synthetic_experimets()
{
    constexpr int num_trials = 100; /* number of trials per experiment */

    std::vector<int> logm({4,5,6,7,8,9,10,12,16}); /* array of all values log(m) for which to run hll */
    std::vector<int> k({1,4,16,64,256,1024}); /* array of all values k for which to run rec */

    /* synthetic datasets */
    constexpr int num_datasets = 15;
    double datasets_card[num_datasets];
    /* generate synthetic datasets and calculate their cardinalities */
    std::cout << "Generate synthetic dataset and calculate cardinalities " << std::endl;
    std::vector<std::vector<int>> Z(num_datasets);
    int stream_length = 1000;
    for (int d = 0; d < num_datasets; d++, stream_length*=2)
    {
        std::cout << "Dataset " << d << " (" << stream_length << ")" << std::endl;
        generate_zipfian(Z[d], stream_length, stream_length, 0.0);
        datasets_card[d] = cardinality(Z[d]);
    }

    /* Experiments on all generated datasets */
    std::vector<std::vector<double>> hll_avg_rel_acc(num_datasets); /* averages per dataset per m */
    std::vector<std::vector<double>> rec_avg_rel_acc(num_datasets); /* averages per dataset per k */
    for (int d = 0; d < num_datasets; d++)
    {
        std::vector<std::vector<double>> hll_estimates(logm.size()); /* results per m per trial */
        std::vector<std::vector<double>> rec_estimates(k.size());    /* results per m per trial */

        /* Run (several trials of) all estimiation algorithms on a data set */
        for (int trial = 0; trial < num_trials; trial++)
        {
            /* Instantiate a random hash function (common for all algorithm variations throughout trial) */
            clhasher h(rng(), rng()); // NOTE: clhasher is a really shitty class that doesn't properly handle memory resources 
                                     // --> do not copy, move, etc..!!!
            /* HyperLogLog */
            for (int i = 0; i < (int)logm.size(); i++)
            {
                std::cout << "Dataset " << d << " - HLL" << uiexp2(logm[i]) << std::endl;
                double estimate = hll(h, Z[d], logm[i]);
                hll_estimates[i].push_back(estimate);
            }
            /* Recordinality */
            for (int i = 0; i < (int)k.size(); i++)
            {
                std::cout << "Dataset " << d << " - REC" << k[i] << std::endl;
                double estimate = rec(h, Z[d], k[i]);
                rec_estimates[i].push_back(estimate);
            }
        }

        /* Aggregate results */
        /* HyperLogLog */
        for (auto hll_m : hll_estimates)
        {
            double relerr = 0.0;
            for (auto est : hll_m)
            {
                relerr += std::abs(est - datasets_card[d]) / datasets_card[d];
            }
            hll_avg_rel_acc[d].push_back(relerr/hll_m.size());
        }
        /* Recordinality */
        for (auto rec_k : rec_estimates)
        {
            double relerr = 0.0;
            for (auto est : rec_k)
            {
                relerr += std::abs(est - datasets_card[d]) / datasets_card[d];
            }
            rec_avg_rel_acc[d].push_back(relerr/rec_k.size());
        }
    }

    /* Output to file */
    for (int i = 0; i < (int)logm.size(); i++)
    {
        std::ofstream ofile("../out/hll_" + std::to_string(uiexp2(logm[i])), std::ios_base::out);
        if (!ofile.is_open())
        {
            std::cerr << "Couldn't open file for output!\n";
            throw;
        }
        for (int d = 0; d < num_datasets; d++)
            ofile << datasets_card[d] << " " << hll_avg_rel_acc[d][i] << "\n";
    }
    for (int i = 0; i < (int)k.size(); i++)
    {
        std::ofstream ofile("../out/rec_" + std::to_string(k[i]), std::ios_base::out);
        if (!ofile.is_open())
        {
            std::cerr << "Couldn't open file for output!\n";
            throw;
        }
        for (int d = 0; d < num_datasets; d++)
            ofile << datasets_card[d] << " " << rec_avg_rel_acc[d][i] << "\n";
    }
}


int main()
{
    real_experimets();
    synthetic_experimets();
}