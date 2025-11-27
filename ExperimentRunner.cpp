#include "ExperimentRunner.h"
#include <atomic>

double getCurrentMemoryUsageMB()
{
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        return pmc.WorkingSetSize / (1024.0 * 1024.0);
    }
    return 0.0;
}

double getCpuTime() {
    FILETIME creation, exit, kernel, user;
    if (GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user)) {
        ULARGE_INTEGER k, u;
        k.LowPart = kernel.dwLowDateTime;
        k.HighPart = kernel.dwHighDateTime;
        u.LowPart = user.dwLowDateTime;
        u.HighPart = user.dwHighDateTime;

        return (double)(k.QuadPart + u.QuadPart) * 1e-7;
    }
    return 0;
}

void printConfig(const Config &config)
{
    std::cout << "N-Queens Solver" << "\n";
    std::cout << "- Solver: " << config.solverType << "\n";
    std::cout << "- Board Size: " << config.boardSize << "\n";
    std::cout << "- Parallel: " << (config.isParallel ? "Yes" : "No") << "\n";
    if (config.isParallel)
    {
        std::cout << "- Threads: " << config.nThreads << "\n";
        std::cout << "- Domain Granularity: " << config.domainGranularity << "\n";
    }
    std::cout << "\n";
}

// spawn solver based on config
// maxDepth is used for filling out the domain at the start
std::unique_ptr<Solver> spawnSolver(const std::string &solverType, int boardSize, const Solution &initialState, int maxDepth = 0,
                                    std::queue<Solution> *workQueue = nullptr, std::mutex *queueMutex = nullptr)
{
    if (solverType == "BT")
    {
        return std::make_unique<BTSolver>(boardSize, initialState, maxDepth, workQueue, queueMutex);
    }
    else if (solverType == "BT-FC")
    {
        return std::make_unique<BTFCSolver>(boardSize, initialState, maxDepth, workQueue, queueMutex);
    }
    else if (solverType == "BT-FC-DVO")
    {
        return std::make_unique<BTFCDVOSolver>(boardSize, initialState, maxDepth, workQueue, queueMutex);
    }
    else if (solverType == "AC3")
    {
        return std::make_unique<AC3Solver>(boardSize, initialState, maxDepth, workQueue, queueMutex);
    }
    else if (solverType == "AC3-DVO")
    {
        return std::make_unique<AC3DVOSolver>(boardSize, initialState, maxDepth, workQueue, queueMutex);
    }

    std::cout << "Error while spawning solver! Are you sure you typed in a valid type?\n";
    return nullptr;
}
// https://stackoverflow.com/questions/12347371/stdput-time-formats
std::string getCurrentTimestamp()
{
    // when to use auto vs time t?
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d-%H-%M-%S");
    return oss.str();
}

void printSolution(const Solution &sol)
{
    for (int row = 0; row < sol.size(); row++)
    {
        for (int col = 0; col < sol.size(); col++)
        {
            std::cout << (sol[row] == col ? "Q " : ". ");
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}


// pop from primary work queue + allocate solver + start solve + loop if work queue not empty
void workerThread(std::queue<Solution> *workQueue, std::mutex *queueMutex, const Config &config, std::vector<std::unique_ptr<Solver>> *solvers, std::mutex *solversMutex)
{
    while (true)
    {
        Solution initialState;

        // pop work from queue
        {
            std::lock_guard<std::mutex> lock(*queueMutex);
            if (workQueue->empty())
            {
                break; // wq empty
            }
            initialState = workQueue->front();
            workQueue->pop();
        }

        auto solver = spawnSolver(config.solverType, config.boardSize, initialState);
        solver->solve();

        // double check if locking is proper
        {
            std::lock_guard<std::mutex> lock(*solversMutex);
            solvers->push_back(std::move(solver));
        }
    }
}



ExperimentResult runExperiment(const Config& config) {

    printConfig(config);


    std::atomic<bool> running = true;
    double peakMemoryMB = 0.0;

    std::thread monitor([&]() {
        double localPeak = 0.0;
        while (running.load()) {
            double mem = getCurrentMemoryUsageMB();
            if (mem > localPeak)
                localPeak = mem;

            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // polling interval
        }
        peakMemoryMB = localPeak;
    });

    auto startTime = std::chrono::high_resolution_clock::now();
    std::vector<Solution> allSolutions;
    std::chrono::high_resolution_clock::time_point firstSolutionTime;
    double startCpuTime = getCpuTime();

    // if threads > 1, make work queue, init a solver with depth = domainGrnularity to populate wq
    // then, init nThreads workThreads
    if (config.isParallel)
    {
        std::queue<Solution> workQueue;
        std::mutex queueMutex;

        Solution baseState(config.boardSize, -1);
        auto seedSolver = spawnSolver(config.solverType, config.boardSize, baseState,
                                      config.domainGranularity, &workQueue, &queueMutex);
        seedSolver->solve();

        std::cout << "Work queue populated with " << workQueue.size() << " initial states\n \n";

        std::vector<std::unique_ptr<Solver>> solvers;
        std::mutex solversMutex;
        std::vector<std::thread> threads;
        for (int i = 0; i < config.nThreads; i++)
        {
            threads.emplace_back(workerThread, &workQueue, &queueMutex, std::ref(config), &solvers, &solversMutex);
        }

        for (auto &thread : threads)
        {
            thread.join();
        }

        // compile solutions from all solvers
        bool foundFirst = false;
        for (auto &solver : solvers)
        {
            // std::vector<Solution> &solutions = solver->getSolutions();
            const std::vector<Solution> &solutions = solver->getSolutions();
            allSolutions.insert(allSolutions.end(), solutions.begin(), solutions.end());

            // yoink the fastest first sol from all solvers

            // you have to check if solutions empty, bc otherwise, it crashes if nStates < initial domains,
            // or the initial domain it gets ends up being a dead end
            // if (!foundFirst)
            if (!foundFirst && !solutions.empty())
            {
                firstSolutionTime = solver->getFirstSolutionTime();
                foundFirst = true;
            }
            else if (!solutions.empty())
            {
                if (solver->getFirstSolutionTime() < firstSolutionTime)
                    firstSolutionTime = solver->getFirstSolutionTime();
            }
        }
    }

    // if NOT PARALLEL, just run solver plainly, with seed domain of empty board
    else
    {
        Solution initialState(config.boardSize, -1);
        auto solver = spawnSolver(config.solverType, config.boardSize, initialState);
        solver->solve();

        allSolutions = solver->getSolutions();
        firstSolutionTime = solver->getFirstSolutionTime();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double endCpuTime = getCpuTime();
    double elapsedCpuTime = endCpuTime - startCpuTime;

    running = false;
    monitor.join();
    

    double timeToFirst = std::chrono::duration<double>(firstSolutionTime - startTime).count();
    double timeToAll = std::chrono::duration<double>(endTime - startTime).count();

    // results
    std::cout << "Time to First Solution: " << timeToFirst << " seconds\n";
    std::cout << "Time to All Solutions: " << timeToAll << " seconds\n";
    std::cout << "CPU Time Used: " << elapsedCpuTime << " seconds\n";
    std::cout << "Peak Memory Usage: " << peakMemoryMB << " MB\n";

    std::cout << "Number of Solutions: " << allSolutions.size() << "\n\n";
    
    if (config.printAllSolutions)
    {
        std::cout << "All Solutions: \n\n";
        for (size_t i = 0; i < allSolutions.size(); i++)
        {
            std::cout << "Solution " << (i + 1) << ":\n";
            printSolution(allSolutions[i]);
        }
    }

    return ExperimentResult{
        startTime,
        endTime,
        firstSolutionTime,
        timeToFirst,
        timeToAll,
        elapsedCpuTime,
        peakMemoryMB,
        static_cast<int>(allSolutions.size())
    };

}