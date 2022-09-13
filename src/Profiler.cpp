#include <Arduino.h>
#include "Profiler.h"


namespace Profiler
{
    static uint32_t points[MAX_POINTS] = {};
    static uint32_t last_micros;
    static int last_index;


    void reset()
    {
        for(int i = 0; i < MAX_POINTS; i++)
        {
            points[i] = 0;
        }
        last_index = 0;
    }

    void start()
    {
        last_micros = micros();
    }

    void stop()
    {
        points[last_index] = micros() - last_micros;
        last_index++;
        if(last_index >= MAX_POINTS)
        {
            last_index = 0;
        }
        last_micros = micros();
    }

    uint32_t* get_results()
    {
        return points;
    }

}