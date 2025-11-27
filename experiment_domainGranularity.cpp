#include "ExperimentRunner.h"
#include "ExperimentIO.h"

int main()
{

    std::string fileName = "granularities_results.csv";

    const int granularities[] = {1, 2, 3};

    const int numRuns = 5;

    for (const auto& granularity : granularities) {

        for (int run = 0; run < numRuns; ++run) {
            Config config;
            config.solverType = "AC3";
            config.nThreads = 6;
            config.boardSize = 16;
            config.printAllSolutions = false;
            config.printResultsToTxt = true;
            config.saveSolutionsToTxt = false;
            config.isParallel = true;
            config.domainGranularity = granularity;

            ExperimentResult result = runExperiment(config);

            if(config.printResultsToTxt)
            {
                addToCSV(fileName, config, result);
            }
            std::cout << "------------------------------------------------\n";
        }
    }
    return 0;
}