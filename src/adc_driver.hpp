#pragma once

#include <stdint.h>

namespace adc_driver
{
    const unsigned char BUFFER_SIZE = 64;
    class Adc_Driver
    {
        public: 
            Adc_Driver(void* adc);

            void start_conversion();
            bool get_values_if_available(int16_t* out);
            bool get_overflow_status();
            void putData(int16_t data);
        private:
            void* adc;
            unsigned char index;
            int16_t buffer_0[BUFFER_SIZE];
            int16_t buffer_1[BUFFER_SIZE];
            bool write_buffer_1;
            volatile bool buffer_ready;
            volatile bool overflow;

            static void register_adc_in_isr(Adc_Driver* driver);
    };


}