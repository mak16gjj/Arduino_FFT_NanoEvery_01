#pragma once
#include <stdint.h>

namespace profiler
{
    const int MAX_POINTS = 8;

    class Profiler
    {
        public:
            Profiler();
            void reset();
            void start();
            void log();

            uint32_t* get_results();

        private:
            uint32_t points[MAX_POINTS];
            uint32_t last_micros;
            int last_index;
    };

    extern Profiler global_Profiler;
}
