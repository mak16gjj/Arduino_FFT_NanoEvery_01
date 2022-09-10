#pragma once

#include <Arduino.h>

namespace adc
{   
    
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
}