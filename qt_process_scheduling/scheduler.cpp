#include "scheduler.h"
#include <algorithm>
#include <limits>
#include <queue>
#include <unordered_map>

namespace {
    void ensure_timeline(std::vector<ExecutionStep>& t, int len) {
        for (int i = (int)t.size(); i < len; ++i) {
            t.push_back({i, "Idle"});
        }
    }
}

ScheduleOutcome Scheduler::fcfs(const std::vector<Process>& procs) const {
    ScheduleOutcome out;
    auto queue = limitProcesses(procs);
    if (queue.empty()) return out;

    std::sort(queue.begin(), queue.end(), [](const Process& a, const Process& b){
        if (a.arrival() != b.arrival()) return a.arrival() < b.arrival();
        return a.name() < b.name();
    });

    int time = 0;
    for (const auto& p : queue) {
        int start = std::max(time, p.arrival());
        int finish = start + p.burst();
        ensure_timeline(out.timeline, finish);
        for (int t = start; t < finish; ++t) out.timeline[t].running = p.name();

        ProcessResult r{p.name(), start, finish, start - p.arrival(), finish - p.arrival()};
        out.results.push_back(r);
        time = finish;
    }
    return out;
}

ScheduleOutcome Scheduler::sjf(const std::vector<Process>& procs) const {
    ScheduleOutcome out;
    auto queue = limitProcesses(procs);
    if (queue.empty()) return out;

    int n = (int)queue.size();
    std::vector<bool> done(n, false);
    int completed = 0;
    int time = 0;

    while (completed < n) {
        int idx = -1;
        int minBurst = std::numeric_limits<int>::max();
        for (int i = 0; i < n; ++i) {
            if (!done[i] && queue[i].arrival() <= time) {
                if (queue[i].burst() < minBurst) {
                    minBurst = queue[i].burst();
                    idx = i;
                }
            }
        }
        if (idx == -1) {
            ensure_timeline(out.timeline, time+1);
            time++;
            continue;
        }
        int start = time;
        int finish = start + queue[idx].burst();
        ensure_timeline(out.timeline, finish);
        for (int t = start; t < finish; ++t) out.timeline[t].running = queue[idx].name();

        ProcessResult r{queue[idx].name(), start, finish, start - queue[idx].arrival(), finish - queue[idx].arrival()};
        out.results.push_back(r);
        done[idx] = true;
        completed++;
        time = finish;
    }
    return out;
}

ScheduleOutcome Scheduler::srtf(const std::vector<Process>& procs) const {
    ScheduleOutcome out;
    auto queue = limitProcesses(procs);
    int n = (int)queue.size();
    if (n == 0) return out;

    struct Node { Process p; int rem; int idx; };
    std::vector<Node> nodes;
    nodes.reserve(n);
    for (int i=0;i<n;++i) nodes.push_back({queue[i], queue[i].burst(), i});

    std::vector<int> startTime(n, -1);
    std::vector<int> finishTime(n, -1);
    int completed = 0;
    int time = 0;

    while (completed < n) {
        int sel = -1;
        int minRem = std::numeric_limits<int>::max();
        for (int i=0;i<n;++i) {
            if (nodes[i].p.arrival() <= time && nodes[i].rem > 0) {
                if (nodes[i].rem < minRem) {
                    minRem = nodes[i].rem;
                    sel = i;
                }
            }
        }
        if (sel == -1) {
            ensure_timeline(out.timeline, time+1);
            ++time;
            continue;
        }

        if (startTime[sel] == -1) startTime[sel] = time;
        ensure_timeline(out.timeline, time+1);
        out.timeline[time].running = nodes[sel].p.name();
        nodes[sel].rem -= 1;
        ++time;

        if (nodes[sel].rem == 0) {
            finishTime[sel] = time;
            ++completed;
            int turnaround = finishTime[sel] - nodes[sel].p.arrival();
            int waiting = turnaround - nodes[sel].p.burst();
            out.results.push_back({nodes[sel].p.name(), startTime[sel], finishTime[sel], waiting, turnaround});
        }
    }
    return out;
}

ScheduleOutcome Scheduler::rr(const std::vector<Process>& procs, int quantum) const {
    ScheduleOutcome out;
    auto queue = limitProcesses(procs);
    if (queue.empty()) return out;
    if (quantum <= 0) quantum = 1;

    int n = (int)queue.size();
    std::vector<int> rem(n);
    std::vector<int> start(n, -1);
    std::vector<int> arrival_time(n);
    std::vector<bool> inReadyQueue(n, false);
    for (int i = 0; i < n; ++i) {
        rem[i] = queue[i].burst();
        arrival_time[i] = queue[i].arrival();
    }

    std::vector<std::pair<int, int>> sorted_arrivals(n);
    for (int i = 0; i < n; ++i) sorted_arrivals[i] = {arrival_time[i], i};
    std::sort(sorted_arrivals.begin(), sorted_arrivals.end());
    int arrival_ptr = 0;

    std::queue<int> readyQ;
    int time = 0;
    int completed_count = 0;

    while (completed_count < n) {
        while (arrival_ptr < n && sorted_arrivals[arrival_ptr].first <= time) {
            int i = sorted_arrivals[arrival_ptr].second;
            if (!inReadyQueue[i] && rem[i] > 0) {
                readyQ.push(i);
                inReadyQueue[i] = true;
            }
            ++arrival_ptr;
        }

        if (readyQ.empty()) {
            int nextArrival = std::numeric_limits<int>::max();
            if (arrival_ptr < n) {
                nextArrival = sorted_arrivals[arrival_ptr].first;
            } else {
                break;
            }

            ensure_timeline(out.timeline, nextArrival);
            time = nextArrival;
            continue;
        }

        int i = readyQ.front(); readyQ.pop(); inReadyQueue[i] = false;

        if (start[i] == -1) start[i] = time;

        int execution_slice = std::min(quantum, rem[i]);
        int finish_time_slice = time + execution_slice;

        for (int t = time; t < finish_time_slice; ++t) {
            ensure_timeline(out.timeline, t + 1);
            out.timeline[t].running = queue[i].name();
            --rem[i];

            while (arrival_ptr < n && sorted_arrivals[arrival_ptr].first <= t + 1) {
                int j = sorted_arrivals[arrival_ptr].second;
                if (!inReadyQueue[j] && rem[j] > 0) {
                    readyQ.push(j);
                    inReadyQueue[j] = true;
                }
                ++arrival_ptr;
            }
        }

        time = finish_time_slice;

        if (rem[i] > 0) {
            readyQ.push(i);
            inReadyQueue[i] = true;
        } else {
            int turnaround = time - queue[i].arrival();
            int waiting = turnaround - queue[i].burst();

            out.results.push_back({queue[i].name(), start[i], time, waiting, turnaround});
            ++completed_count;
        }
    }

    return out;
}

ScheduleOutcome Scheduler::priority(const std::vector<Process>& procs) const {
    ScheduleOutcome out;
    auto queue = limitProcesses(procs);
    if (queue.empty()) return out;

    int n = (int)queue.size();
    std::vector<bool> done(n, false);
    int completed = 0;
    int time = 0;

    while (completed < n) {
        int sel = -1;
        int bestPri = std::numeric_limits<int>::max();
        for (int i=0;i<n;++i) {
            if (!done[i] && queue[i].arrival() <= time) {
                if (queue[i].priority() < bestPri) {
                    bestPri = queue[i].priority();
                    sel = i;
                } else if (queue[i].priority() == bestPri) {
                    if (sel == -1 || queue[i].arrival() < queue[sel].arrival()) sel = i;
                }
            }
        }
        if (sel == -1) {
            ensure_timeline(out.timeline, time+1);
            ++time;
            continue;
        }
        int start = time;
        int finish = start + queue[sel].burst();
        ensure_timeline(out.timeline, finish);
        for (int t = start; t < finish; ++t) out.timeline[t].running = queue[sel].name();

        out.results.push_back({queue[sel].name(), start, finish, start - queue[sel].arrival(), finish - queue[sel].arrival()});
        done[sel] = true;
        ++completed;
        time = finish;
    }
    return out;
}
