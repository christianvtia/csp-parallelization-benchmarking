#pragma once

#include "ExperimentRunner.h"
#include <string>
#include <chrono>
#include <filesystem>

Config readConfig(const std::string &filename);

std::string toISO8601(const std::chrono::high_resolution_clock::time_point &tp);

void addToCSV(const std::string &outputFilename,
              const Config &config,
              const ExperimentResult &exp);