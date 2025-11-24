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
    std::vector<int> finish(n, -1);
    for (int i=0;i<n;++i) rem[i] = queue[i].burst();

    int time = 0;
    std::queue<int> q;
    std::vector<bool> inQueue(n, false);
    int completed = 0;

    auto enqueueArrived = [&](int currentTime){
        for (int i=0;i<n;++i) {
            if (!inQueue[i] && rem[i] > 0 && queue[i].arrival() <= currentTime) {
                q.push(i);
                inQueue[i] = true;
            }
        }
    };

    enqueueArrived(time);

    while (completed < n) {
        if (q.empty()) {
            int nextArrival = std::numeric_limits<int>::max();
            for (int i=0;i<n;++i) if (rem[i] > 0) nextArrival = std::min(nextArrival, queue[i].arrival());
            if (nextArrival == std::numeric_limits<int>::max()) break;
            ensure_timeline(out.timeline, nextArrival+1);
            time = nextArrival;
            enqueueArrived(time);
            continue;
        }

        int i = q.front(); q.pop(); inQueue[i] = false;
        if (start[i] == -1) start[i] = time;

        int slice = std::min(quantum, rem[i]);
        for (int s = 0; s < slice; ++s) {
            ensure_timeline(out.timeline, time+1);
            out.timeline[time].running = queue[i].name();
            ++time;
        }
        rem[i] -= slice;

        enqueueArrived(time);

        if (rem[i] == 0) {
            finish[i] = time;
            ++completed;
            int turnaround = finish[i] - queue[i].arrival();
            int waiting = turnaround - queue[i].burst();
            out.results.push_back({queue[i].name(), start[i], finish[i], waiting, turnaround});
        } else {
            q.push(i);
            inQueue[i] = true;
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
