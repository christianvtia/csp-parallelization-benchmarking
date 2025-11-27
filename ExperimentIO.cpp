#include "ExperimentIO.h"

Config readConfig(const std::string &filename)
{
    Config config;
    config.domainGranularity = 1; // by default, only populate first variable

    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string key, value;

        if (std::getline(iss, key, ':'))
        {
            std::getline(iss, value);

            // clean
            value.erase(0, value.find_first_not_of(" \t"));

            if (key == "solverType")
                config.solverType = value;
            else if (key == "nThreads")
                config.nThreads = std::stoi(value);
            else if (key == "boardSize")
                config.boardSize = std::stoi(value);
            else if (key == "printAllSolutions")
                config.printAllSolutions = (value == "true");
            else if (key == "printResultsToTxt")
                config.printResultsToTxt = (value == "true");
            else if (key == "saveSolutionsToTxt")
                config.saveSolutionsToTxt = (value == "true");
            else if (key == "domainGranularity")
                config.domainGranularity = std::stoi(value);
        }
    }

    config.isParallel = (config.nThreads > 1);
    return config;
}

std::string toISO8601(const std::chrono::high_resolution_clock::time_point &tp)
{
    using namespace std::chrono;

    auto ms = time_point_cast<milliseconds>(tp);
    auto epoch_ms = ms.time_since_epoch().count();

    std::time_t t = epoch_ms / 1000;
    int remainder_ms = epoch_ms % 1000;
    std::tm tm{};
    gmtime_s(&tm, &t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S")
        << "." << std::setw(3) << std::setfill('0') << remainder_ms
        << "Z";
    return oss.str();
}


void addToCSV(const std::string &outputFilename, const Config &config, const ExperimentResult &exp)
{
    bool fileExists = std::filesystem::exists(outputFilename);
    std::ofstream file(outputFilename, std::ios::app);

    if (!fileExists)
    {
        file << "solverType,threads,isParallel,boardSize,domainGranularity,"
                "startTime,endTime,firstSolutionTime,"
                "timeToFirst,timeToAll,cpuTime,peakMemoryMB,numberOfSolutions\n";
    }

    file << config.solverType << ","
         << config.nThreads << ","
         << (config.isParallel ? 1 : 0) << ","
         << config.boardSize << ","
         << config.domainGranularity << ","
         << toISO8601(exp.startTime) << ","
         << toISO8601(exp.endTime) << ","
         << toISO8601(exp.firstSolutionTime) << ","
         << exp.timeToFirst << ","
         << exp.timeToAll << ","
         << exp.cpuTime << ","
         << exp.peakMemoryMB << ","
         << exp.numberOfSolutions << "\n";

    file.close();

    std::cout << "Results appended to " << outputFilename << "\n";
}