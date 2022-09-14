#include <Arduino.h>
#include "Profiler.hpp"


namespace profiler
{
    Profiler::Profiler()
    {
        #ifdef PROFILING_ENABLED
        reset();
        #endif
    }

    void Profiler::reset()
    {
        #ifdef PROFILING_ENABLED
        for(int i = 0; i < MAX_POINTS; i++)
        {
            points[i] = 0;
        }
        last_index = 0;
        #endif
    }

    void Profiler::start()
    {
        #ifdef PROFILING_ENABLED
        last_micros = micros();
        #endif
    }

    void Profiler::log()
    {
        #ifdef PROFILING_ENABLED
        points[last_index] = micros() - last_micros;
        last_index++;
        if(last_index >= MAX_POINTS)
        {
            last_index = 0;
        }
        last_micros = micros();
        #endif
    }

    uint32_t* Profiler::get_results()
    {
        #ifdef PROFILING_ENABLED
        return points;
        #endif
    }

    Profiler global_Profiler;
}