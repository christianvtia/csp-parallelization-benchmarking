#include "ExperimentRunner.h"
#include "ExperimentIO.h"

int main()
{

    const int n = 5;

    const int boardSizes[] = {16};
    const std::string solverTypes[] = {"AC3", "AC3-DVO"};
    const int threadCounts[] = {10, 8, 6, 4, 2};

    std::string fileName = "ac3_dvo_parallel_results_part2.csv";

    for (const auto& solver : solverTypes) {

        for (const auto& nThreads : threadCounts) {
            Config config;
            config.solverType = solver;
            config.nThreads = nThreads;
            config.boardSize = 16;
            config.printAllSolutions = false;
            config.printResultsToTxt = true;
            config.saveSolutionsToTxt = false;
            config.isParallel = true;
            config.domainGranularity = 2;

            for (int run = 0; run < n; ++run) {
                ExperimentResult result = runExperiment(config);

                if(config.printResultsToTxt)
                {
                    addToCSV(fileName, config, result);
                }
                std::cout << "------------------------------------------------\n";
            }
        }
    }

    
    return 0;
}