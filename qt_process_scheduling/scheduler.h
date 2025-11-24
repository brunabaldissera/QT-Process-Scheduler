#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <string>
#include "process.h"

struct ExecutionStep {
    int time;
    std::string running;
};

struct ProcessResult {
    std::string name;
    int startTime;
    int finishTime;
    int waitingTime;
    int turnaroundTime;
};

struct ScheduleOutcome {
    std::vector<ProcessResult> results;
    std::vector<ExecutionStep> timeline;
};

class Scheduler {
public:
    Scheduler(int maxProcs = 100) : m_maxProcesses(maxProcs) {}

    ScheduleOutcome fcfs(const std::vector<Process>& procs) const;
    ScheduleOutcome sjf(const std::vector<Process>& procs) const;
    ScheduleOutcome srtf(const std::vector<Process>& procs) const;
    ScheduleOutcome rr(const std::vector<Process>& procs, int quantum) const;
    ScheduleOutcome priority(const std::vector<Process>& procs) const;

private:
    int m_maxProcesses;

    std::vector<Process> limitProcesses(const std::vector<Process>& procs) const {
        if ((int)procs.size() <= m_maxProcesses)
            return procs;
        return std::vector<Process>(procs.begin(), procs.begin() + m_maxProcesses);
    }
};

#endif // SCHEDULER_H
