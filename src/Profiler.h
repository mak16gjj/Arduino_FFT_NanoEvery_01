#pragma once


namespace Profiler
{
    const int MAX_POINTS = 8;

    void reset();
    void start();
    void stop();
    uint32_t* get_results();
}
