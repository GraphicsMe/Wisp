#ifndef TIMER_H
#define TIMER_H

#include "common.h"
#include <Windows.h>

WISP_NAMESPACE_BEGIN

class Timer
{
public:
    Timer();
    void start();
    void stop();
    void reset();

    double elapsedTime();
    double currentTime();

private:
    bool running;
    double time0, elapsed;
    LARGE_INTEGER performance_counter, performance_frequency;
    double one_over_frequency;
};

WISP_NAMESPACE_END
#endif // TIMER_H
