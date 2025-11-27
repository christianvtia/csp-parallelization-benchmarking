#include "ExperimentRunner.h"
#include "ExperimentIO.h"

#include <filesystem>
#include <sstream>
#include <fstream>

int main()
{
    Config config = readConfig("config.txt");
    ExperimentResult result = runExperiment(config);

    if(config.printResultsToTxt)
    {
        addToCSV("test.csv", config, result);
    }

    return 0;
}