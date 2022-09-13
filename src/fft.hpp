#pragma once

#include <stdint.h>



namespace fft
{
    typedef int64_t FFT_DATA_TYPE;
    typedef int64_t FFT_BUTTERFLY_DATA_TYPE;

    const FFT_DATA_TYPE FFT_DATA_TYPE_MAX = INT_FAST16_MAX;
    const FFT_DATA_TYPE FFT_DATA_TYPE_MIN = INT_FAST16_MIN;

    const int LB_NUMBER_OF_SAMPLES = 6;
    const int NUMBER_OF_SAMPLES = 1 << LB_NUMBER_OF_SAMPLES;

    class FFT
    {
        public:
            static void do_fft(FFT_DATA_TYPE in[], FFT_DATA_TYPE out[]);
        private:
            static signed char scale_data(FFT_DATA_TYPE data[]);
            static void apply_bit_reverse_ordering(FFT_DATA_TYPE in[], FFT_DATA_TYPE out[]);
            static void apply_butterfly(FFT_DATA_TYPE in[], FFT_DATA_TYPE out[]);
            static void rescale_data(FFT_DATA_TYPE in[], FFT_DATA_TYPE out[],signed char scaling);
    };
}