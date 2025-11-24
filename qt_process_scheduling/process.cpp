#include "process.h"

Process::Process(std::string name, int arrival, int burst, int priority)
    : m_name(std::move(name)), m_arrival(arrival), m_burst(burst), m_priority(priority) {}

const std::string& Process::name() const { return m_name; }
int Process::arrival() const { return m_arrival; }
int Process::burst() const { return m_burst; }
int Process::priority() const { return m_priority; }

void Process::setName(const std::string& n) { m_name = n; }
void Process::setArrival(int a) { m_arrival = a; }
void Process::setBurst(int b) { m_burst = b; }
void Process::setPriority(int p) { m_priority = p; }
