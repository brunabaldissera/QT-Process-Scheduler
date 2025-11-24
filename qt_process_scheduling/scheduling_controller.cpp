#include "scheduling_controller.h"
#include <algorithm>

SchedulingController::SchedulingController() = default;

void SchedulingController::setProcesses(const std::vector<Process>& procs) {
    m_processes = procs;
    std::stable_sort(m_processes.begin(), m_processes.end(), [](const Process& a, const Process& b){
        if (a.arrival() != b.arrival()) return a.arrival() < b.arrival();
        return a.name() < b.name();
    });
}

const std::vector<Process>& SchedulingController::processes() const {
    return m_processes;
}

ScheduleOutcome SchedulingController::runAlgorithm(const std::string& algName, int quantum) const {
    if (algName == "FCFS" || algName == "First Come, First Served") {
        return m_scheduler.fcfs(m_processes);
    } else if (algName == "SJF" || algName == "Shortest Job First") {
        return m_scheduler.sjf(m_processes);
    } else if (algName == "SRTF" || algName == "Shortest Remaining Time First") {
        return m_scheduler.srtf(m_processes);
    } else if (algName == "RR" || algName == "Round Robin") {
        return m_scheduler.rr(m_processes, quantum);
    } else if (algName == "Priority") {
        return m_scheduler.priority(m_processes);
    } else {
        return ScheduleOutcome{};
    }
}
