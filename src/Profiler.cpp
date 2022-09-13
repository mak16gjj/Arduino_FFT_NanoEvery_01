#include <Arduino.h>
#include "Profiler.hpp"


namespace Profiler
{
    Profiler::Profiler()
    {
        reset();
    }

    void Profiler::reset()
    {
        for(int i = 0; i < MAX_POINTS; i++)
        {
            points[i] = 0;
        }
        last_index = 0;
    }

    void Profiler::start()
    {
        last_micros = micros();
    }

    void Profiler::log()
    {
        points[last_index] = micros() - last_micros;
        last_index++;
        if(last_index >= MAX_POINTS)
        {
            last_index = 0;
        }
        last_micros = micros();
    }

    uint32_t* Profiler::get_results()
    {
        return points;
    }

}