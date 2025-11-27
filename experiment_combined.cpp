#include "ExperimentRunner.h"
#include "ExperimentIO.h"

int main()
{

    const int n = 5;

    std::string fileName = "sequential_results.csv";

    const int boardSizes[] = {8, 10, 12, 14};
    const std::string solverTypes[] = {"BT", "BT-FC", "BT-FC-DVO", "AC3", "AC3-DVO"};
    const int threadCounts[] = {10, 8, 6, 4, 2};

    for (const auto& solver : solverTypes) {
        for (const auto& size : boardSizes) {
            Config config;
            config.solverType = solver;
            config.nThreads = 1;
            config.boardSize = size;
            config.printAllSolutions = false;
            config.printResultsToTxt = true;
            config.saveSolutionsToTxt = false;
            config.isParallel = false;
            config.domainGranularity = 1;

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

    fileName = "parallel_results.csv";

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