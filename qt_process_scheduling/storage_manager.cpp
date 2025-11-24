#include "storage_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool StorageManager::saveProject(const std::vector<Process>& processes,
                                 const AlgorithmParameters& params,
                                 const std::vector<ProcessResult>& results,
                                 const std::string& filename) const
{
    std::ofstream out(filename);
    if (!out.is_open()) return false;

    // Parâmetros
    out << "#PARAMS\n";
    out << params.algorithmName << "," << params.quantum << "\n";

    // Processos
    out << "#PROCESSES\n";
    out << "Name,Arrival,Burst,Priority\n";
    for (const auto& p : processes)
        out << p.name() << "," << p.arrival() << "," << p.burst() << "," << p.priority() << "\n";

    // Resultados
    out << "#RESULTS\n";
    out << "PID,Start,Finish,Waiting,Turnaround\n";
    for (const auto& r : results)
        out << r.name << "," << r.startTime << "," << r.finishTime << "," << r.waitingTime << "," << r.turnaroundTime << "\n";

    return true;
}

bool StorageManager::loadProject(std::vector<Process>& processes,
    AlgorithmParameters& params,
    std::vector<ProcessResult>& results,
    const std::string& filename) const
    {
    std::ifstream in(filename);
    if (!in.is_open()) return false;

    std::string line;
    enum Section { NONE, PARAMS, PROCESSES, RESULTS } section = NONE;

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        if (line[0] == '#') {
            if (line == "#PARAMS") section = PARAMS;
            else if (line == "#PROCESSES") section = PROCESSES;
            else if (line == "#RESULTS") section = RESULTS;
            else section = NONE;
            continue;
        }

        std::stringstream ss(line);
        std::string token;

        switch(section) {
            case PARAMS: {
                std::getline(ss, token, ','); params.algorithmName = token;
                std::getline(ss, token, ','); params.quantum = std::stoi(token);
                break;
            }
            case PROCESSES: {
                std::string name; int arrival, burst, priority;
                std::getline(ss, token, ','); name = token;
                std::getline(ss, token, ','); arrival = std::stoi(token);
                std::getline(ss, token, ','); burst = std::stoi(token);
                std::getline(ss, token, ','); priority = std::stoi(token);
                processes.emplace_back(name, arrival, burst, priority);
                break;
            }
            case RESULTS: {
                ProcessResult r;
                std::getline(ss, token, ','); r.name = token;
                std::getline(ss, token, ','); r.startTime = std::stoi(token);
                std::getline(ss, token, ','); r.finishTime = std::stoi(token);
                std::getline(ss, token, ','); r.waitingTime = std::stoi(token);
                std::getline(ss, token, ','); r.turnaroundTime = std::stoi(token);
                results.push_back(r);
                break;
            }
            default: break;
        }
    }

    return true;
}

