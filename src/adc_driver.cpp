#include <Arduino.h>

#include "adc_driver.hpp"
#include "optimization_config.h"

namespace adc_driver
{   
    Adc_Driver::Adc_Driver(void* adc)
    {
        this-> adc = adc;
        buffer_ready = false;
        write_buffer_1 = false;
        overflow = false;
        index = 0;
    }

    void Adc_Driver::start_conversion()
    {

    }

    void Adc_Driver::putData(int16_t data)
    {
        if(overflow)
        {
            return;
        }
        int16_t* buffer;
        if(write_buffer_1)
        {
            buffer = buffer_1;
        }
        else
        {
            buffer = buffer_0;
        }
        buffer[index] = data;
        index++;
        if(index >= BUFFER_SIZE)
        {
            index = 0;
            write_buffer_1 = ! write_buffer_1;
            if(buffer_ready)
            {
                overflow = true;
                return;
            }
            else
            {
                buffer_ready = true;
                return;
            }
        }
    }

    bool Adc_Driver::get_values_if_available(int16_t* out)
    {
        int16_t* buffer;
        if(buffer_ready)
        {
            buffer_ready = false;
            // != is equal to xor
            if(write_buffer_1 != overflow)
            {
                buffer = buffer_0;
            }
            else
            {
                buffer = buffer_1;
            }
            for(machine_int i = 0; i < BUFFER_SIZE; i++)
            {
                out[i] = buffer[i];
            }
        }
        else
        {
            return false;
        }
    }

    bool Adc_Driver::get_overflow_status()
    {
        bool result = overflow;
        overflow = false;
        return result;
    }


    //actual sample frequency in Hz
    uint64_t sampleFrequencyFrequency = 0;
    enum sampleFrequency
    {
        // conversion time = 56.89 us
        sampleF_18kHz = 5,
        // conversion time = 29.34 us
        sampleF_34kHz = 4,
        // conversion time = 15.52 us
        sampleF_64kHz = 3,
        // conversion time = 8.75 us
        sampleF_114kHz = 2
    };

    // Set the sample frequency
    void setSampleFrequency(sampleFrequency _sf)
    {
        ADC0_CTRLC &= 0xF8;
        // ADC0_CTRLC |= 0x2; // DIV8 CLK_PER divided by 8, Conversion time 8.75us, max. sample freq: 114kHz
        ADC0_CTRLC |= int(_sf);

        switch (_sf)
        {
            case sampleFrequency::sampleF_114kHz: sampleFrequencyFrequency = 114000; break;
            case sampleFrequency::sampleF_64kHz: sampleFrequencyFrequency = 64000; break;
            case sampleFrequency::sampleF_34kHz: sampleFrequencyFrequency = 34000; break;  
            case sampleFrequency::sampleF_18kHz: sampleFrequencyFrequency = 17578; break; 
            default: break;
        }

    }

    static Adc_Driver* isr_driver;

    void register_adc_in_isr(Adc_Driver* driver)
    {
        isr_driver = driver;
    }


}