#pragma once

#define ARDUINO_OPTIMIZATIONS

#ifdef ARDUINO_OPTIMIZATIONS
    #define ARDUINO_SHIFT_10_OPTIMIZATION
    #define machine_int unsigned char
#else
    #define machine_int int
#endif