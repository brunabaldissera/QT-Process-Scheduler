#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <vector>
#include <string>
#include "scheduler.h"

struct AlgorithmParameters {
    std::string algorithmName;
    int quantum;
};

class StorageManager {
public:
    StorageManager() = default;

    bool saveProject(const std::vector<Process>& processes,
                     const AlgorithmParameters& params,
                     const std::vector<ProcessResult>& results,
                     const std::string& filename) const;

    bool loadProject(std::vector<Process>& processes,
                     AlgorithmParameters& params,
                     std::vector<ProcessResult>& results,
                     const std::string& filename) const;
};

#endif // STORAGE_MANAGER_H
