/*! \file timer.h
    \brief timer for realtime
*/
#ifndef SMALLBOARD_TIMER_H
#define SMALLBOARD_TIMER_H

#include <chrono>

class Timer
{
public:
    Timer();             // constructor
    void Reset();        // reset timer
    int64_t sec() const; // second
    int64_t ms() const;  // millisecond
    int64_t us() const;  // microsecond
    float fsec() const;  // float-type second
    float fms() const;   // float-type millisecond
    float fus() const;   // float-type microsecond
private:
    using CLOCK = std::chrono::system_clock clock;
    clock::time_point timestamp;
}

#endif