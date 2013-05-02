#include "timer.h"

WISP_NAMESPACE_BEGIN

Timer::Timer() :
    time0(0),
    elapsed(0),
    running(false)
{
    QueryPerformanceFrequency(&performance_frequency);
    one_over_frequency = 1.0 / ((double)performance_frequency.QuadPart);
}

void Timer::start()
{
    assert (!running);
    running = true;
    time0 = this->currentTime();
}

void Timer::stop()
{
    assert (running);
    running = false;
    elapsed += this->currentTime() - time0;
}

void Timer::reset()
{
    running = false;
    elapsed = 0;
}

double Timer::elapsedTime()
{
    if (running)
    {
        this->stop();
        this->start();
    }
    return elapsed;
}

double Timer::currentTime()
{
    QueryPerformanceCounter(&performance_counter);
    return (double)performance_counter.QuadPart * one_over_frequency;
}


WISP_NAMESPACE_END
