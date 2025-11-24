#ifndef SCHEDULING_CONTROLLER_H
#define SCHEDULING_CONTROLLER_H

#include <vector>
#include <string>
#include "process.h"
#include "scheduler.h"

class SchedulingController {
public:
    SchedulingController();

    void setProcesses(const std::vector<Process>& procs);
    const std::vector<Process>& processes() const;

    ScheduleOutcome runAlgorithm(const std::string& algName, int quantum = 1) const;

private:
    std::vector<Process> m_processes;
    Scheduler m_scheduler;
};

#endif // SCHEDULING_CONTROLLER_H
