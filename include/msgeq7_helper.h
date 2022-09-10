#pragma once

#include <Arduino.h>
#include <MSGEQ7.h>

namespace msgeq7
{

#define pinAnalog A1
#define pinReset 5
#define pinStrobe 6
#define MSGEQ7_INTERVAL ReadsPerSecond(10)
#define MSGEQ7_SMOOTH 0
#define MSGEQ7_10BIT

    CMSGEQ7<MSGEQ7_SMOOTH, pinReset, pinStrobe, pinAnalog> MSGEQ7;
    uint64_t reset_intervall = 1000;
    uint64_t last_rest_millis = 0;
    uint16_t binresults[7];



    // This will set the IC ready for reading
    void begin()
    {
        // This will set the IC ready for reading
        MSGEQ7.begin();
        MSGEQ7.reset();
    }

    //To be called in loop without delay
    void tick()
    {

        // Analyze without delay
        bool newReading = MSGEQ7.read(MSGEQ7_INTERVAL);

        // Led strip output
        if (newReading)
        {
            for(uint8_t bin = 0; bin < 7;bin++)
            {
                // visualize the average bass of both channels
                binresults[bin] = MSGEQ7.get(bin);
                // Reduce noise
                //binresults[bin] = mapNoise(binresults[bin]);

                
            }
        }

        if(millis() - last_rest_millis > reset_intervall)
        {
            MSGEQ7.reset();
            last_rest_millis = millis();
        }
    }
    //Serial print out all 7 bins
    void print()
    {
        for(uint8_t bin = 0; bin < 7;bin++)
        {
            Serial.print(binresults[bin]);
            Serial.print(" | \t");
        }
    }

}
