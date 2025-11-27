#include "ExperimentRunner.h"
#include "ExperimentIO.h"

int main()
{

    std::string fileName = "parallel_results.csv";
    
    const int threadCounts[] = {10, 8, 6, 4, 2};
    const std::string solverTypes[] = {"BT", "BT-FC", "BT-FC-DVO", "AC3", "AC3-DVO"};

    const int numRuns = 5;

    for (const auto& solver : solverTypes) {

        for (const auto& nThreads : threadCounts) {
            for (int run = 0; run < numRuns; ++run) {
                Config config;
                config.solverType = solver;
                config.nThreads = nThreads;
                config.boardSize = 16;
                config.printAllSolutions = false;
                config.printResultsToTxt = true;
                config.saveSolutionsToTxt = false;
                config.isParallel = true;
                config.domainGranularity = 2;

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