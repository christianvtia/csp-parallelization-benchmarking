#pragma once

#include <iostream>
#include <iomanip>
#include <thread>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>

#include <memory>
#include <mutex>
#include <queue>

#include <windows.h>
#include <psapi.h>


#include "BTSolver.h"
#include "BTFCSolver.h"
#include "BTFCDVOSolver.h"

#include "AC3Solver.h"
#include "AC3DVOSolver.h"

struct Config
{
    std::string solverType;
    int nThreads;
    int boardSize;
    bool printAllSolutions;
    bool printResultsToTxt;
    bool saveSolutionsToTxt;
    bool isParallel;
    int domainGranularity;
};

struct ExperimentResult {
    std::chrono::high_resolution_clock::time_point startTime;
    std::chrono::high_resolution_clock::time_point endTime;
    std::chrono::high_resolution_clock::time_point firstSolutionTime;
    

    double timeToFirst;
    double timeToAll;
    double cpuTime;
    double peakMemoryMB;
    int numberOfSolutions;
};

ExperimentResult runExperiment(const Config& config);