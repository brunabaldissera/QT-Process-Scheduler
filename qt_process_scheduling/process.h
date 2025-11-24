#ifndef PROCESS_H
#define PROCESS_H

#include <string>

class Process {
public:
    Process() = default;
    Process(std::string name, int arrival, int burst, int priority = 0);

    // getters
    const std::string& name() const;
    int arrival() const;
    int burst() const;
    int priority() const;

    // setters
    void setName(const std::string& n);
    void setArrival(int a);
    void setBurst(int b);
    void setPriority(int p);

private:
    std::string m_name;
    int m_arrival{0};
    int m_burst{1};
    int m_priority{0};
};

#endif // PROCESS_H
