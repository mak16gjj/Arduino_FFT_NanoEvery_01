#pragma once

#include <Arduino.h>
#include <arduinoFFT.h>
#include <Streaming.h>
#include <adc_helper.h>
#define AUDIO_IN_PIN A0

namespace fft
{
    // arduinoFFT:
    const uint16_t SAMPLES = 64; // Must be a power of 2
    // Arduino Nano Every has a ADC convertion time of 15.52 us => 64.45 kHz.
    const uint16_t SAMPLING_FREQ = adc::sampleFrequencyFrequency; // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
    const uint16_t AMPLITUDE = 1;      // Depending on your audio source level, you may need to alter this value. Can be used as a 'sensitivity' control.
    const uint16_t NUM_BANDS = 8;         // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands
    const uint16_t TOP = 7;
    const uint16_t NOISE = 100; // Used as a crude noise filter, values below this are ignored

    // Sampling and FFT stuff
    uint16_t sampling_period_us;
    uint8_t peak[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // The length of these arrays must be >= NUM_BANDS
    uint8_t oldBarHeights[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t bandValues[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    double vReal[SAMPLES];
    double vImag[SAMPLES];
    unsigned long newTime;
    arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);
    //uint8_t highestBandInBin[8] = {1, 2, 4, 6, 12, 21, 37, 67}; 
    uint8_t highestBandInBin[8] = {1, 2, 3, 4, 7, 12, 20, 33}; 
    uint8_t bandPercentage[8] = {200, 150, 150, 100, 100, 100, 75, 75};




    void doFFT()
    {
        // Sample the audio pin
        for (int i = 0; i < SAMPLES; i++)
        {
            vReal[i] = analogRead(AUDIO_IN_PIN); // A conversion takes about 9.7uS on an ESP32
            vImag[i] = 0;
        }

        // Compute FFT
        FFT.DCRemoval();
        FFT.Windowing(FFT_WIN_TYP_RECTANGLE, FFT_FORWARD);
        FFT.Compute(FFT_FORWARD);
        FFT.ComplexToMagnitude();

        // Reset bandValues[]
        for (int i = 0; i < NUM_BANDS; i++)
        {
            bandValues[i] = 0;
        }

        // Analyse FFT results
        for (int i = 1; i < (SAMPLES / 2); i++)
        { // Don't use sample 0 and only first SAMPLES/2 are usable. Each array element represents a frequency bin and its value the amplitude.
            if (vReal[i] > NOISE)
            { // Add a crude noise filter

                // 8 bands, 12kHz top band
                if (i <= highestBandInBin[0])
                    bandValues[0] += (int)vReal[i];
                if (i > highestBandInBin[0] && i <= highestBandInBin[1])
                    bandValues[1] += (int)vReal[i];
                if (i > highestBandInBin[1] && i <= highestBandInBin[2])
                    bandValues[2] += (int)vReal[i];
                if (i > highestBandInBin[2] && i <= highestBandInBin[3])
                    bandValues[3] += (int)vReal[i];
                if (i > highestBandInBin[3] && i <= highestBandInBin[4])
                    bandValues[4] += (int)vReal[i];
                if (i > highestBandInBin[4] && i <= highestBandInBin[5])
                    bandValues[5] += (int)vReal[i];
                if (i > highestBandInBin[5] && i <= highestBandInBin[6])
                    bandValues[6] += (int)vReal[i];
                if (i > highestBandInBin[6] && i <= highestBandInBin[7])
                    bandValues[7] += (int)vReal[i];
            }
        }

        // Process the FFT data into bar heights
        for (uint8_t band = 0; band < NUM_BANDS; band++)
        {

            // Scale the bars for the display
            int barHeight = int(sqrt(long((bandValues[band] * (100 / 100)))) / AMPLITUDE);

            Serial.print(barHeight);
            Serial.print(" | \t");

            if (barHeight > TOP)
                barHeight = TOP;

            // Small amount of averaging between frames
            barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;

            // Move peak up
            if (barHeight > peak[band])
            {
                peak[band] = min(TOP, barHeight);
            }

            // Draw bars
            fastled::drawBar(band, barHeight);

            // Save oldBarHeights for averaging later
            oldBarHeights[band] = barHeight;
        }
    }

    void sampleFreqTest()
    {
        uint64_t newTime = micros();

        // Do 1 million reads and record time taken
        for (int i = 0; i < 10000; i++)
        {
            uint16_t analogValue = analogRead(AUDIO_IN_PIN);
        }

        float conversionTime = (micros() - newTime) / 10000.0;

        Serial.print("Conversion time: ");
        Serial.print(conversionTime);
        Serial.println(" us");

        Serial.print("Max sampling frequency: ");
        Serial.print((1.0 / conversionTime) * 1000000);
        Serial.println(" Hz");
    }

}
