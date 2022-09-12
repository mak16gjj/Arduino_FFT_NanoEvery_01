#pragma once

#include <Arduino.h>
#include <FastLED_helper.h>
#include <equalizer_helper.h>

#define AUDIO_IN_PIN A0

namespace customfft
{
    void custom_fft(int in[], int out[]);
    int omega_power_real(int exponent_nominator);
    int omega_power_imaginary(int exponent_nominator);
    int abs_complex(int real, int imaginary);

    void custom_fft(int16_t in[], int16_t out[]);

    #define FFT_BUTTERFLY_DATA_TYPE int32_t
    #define FFT_GENERAL_DATA_TYPE int16_t

    const int16_t MAX_MAGNITUDE = 1024;
    const byte OMEGA_SHIFT_AMOUNT = 10;
    const byte LB_NUMBER_OF_SAMPLES = 6;
    const uint16_t NUMBER_OF_SAMPLES = 1 << LB_NUMBER_OF_SAMPLES;

    //---------------------------------lookup data------------------------------------//
    const byte bit_reverse_index[NUMBER_OF_SAMPLES] =
    {
        0x00, 0x20, 0x10, 0x30, 0x08, 0x28, 0x18, 0x38,
        0x04, 0x24, 0x14, 0x34, 0x0c, 0x2c, 0x1c, 0x3c,
        0x02, 0x22, 0x12, 0x32, 0x0a, 0x2a, 0x1a, 0x3a,
        0x06, 0x26, 0x16, 0x36, 0x0e, 0x2e, 0x1e, 0x3e,
        0x01, 0x21, 0x11, 0x31, 0x09, 0x29, 0x19, 0x39,
        0x05, 0x25, 0x15, 0x35, 0x0d, 0x2d, 0x1d, 0x3d,
        0x03, 0x23, 0x13, 0x33, 0x0b, 0x2b, 0x1b, 0x3b,
        0x07, 0x27, 0x17, 0x37, 0x0f, 0x2f, 0x1f, 0x3f,
    };

    const int16_t omega_data[NUMBER_OF_SAMPLES / 4 + 1] = // 1024 * sin values in pi / 32 steps  tp pi/2
        {
            0 >> (10 - OMEGA_SHIFT_AMOUNT), 
            100 >> (10 - OMEGA_SHIFT_AMOUNT),  
            200 >> (10 - OMEGA_SHIFT_AMOUNT),  
            297 >> (10 - OMEGA_SHIFT_AMOUNT), 
            392 >> (10 - OMEGA_SHIFT_AMOUNT),  
            483 >> (10 - OMEGA_SHIFT_AMOUNT),  
            569 >> (10 - OMEGA_SHIFT_AMOUNT),  
            650 >> (10 - OMEGA_SHIFT_AMOUNT), 
            724 >> (10 - OMEGA_SHIFT_AMOUNT),  
            792 >> (10 - OMEGA_SHIFT_AMOUNT),  
            851 >> (10 - OMEGA_SHIFT_AMOUNT),  
            903 >> (10 - OMEGA_SHIFT_AMOUNT), 
            946 >> (10 - OMEGA_SHIFT_AMOUNT),  
            980 >> (10 - OMEGA_SHIFT_AMOUNT),  
            1004 >> (10 - OMEGA_SHIFT_AMOUNT), 
            1019 >> (10 - OMEGA_SHIFT_AMOUNT), 
            1024 >> (10 - OMEGA_SHIFT_AMOUNT)
        };
    
    const byte RSSdata[20] = {7, 6, 6, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2};
    //---------------------------------------------------------------------------------//





    //uint16_t highestBandInBin[8] = {1, 2, 4, 6, 12, 21, 37, 67};
    uint8_t highestBandInBin[8] = {1, 2, 3, 4, 7, 12, 20, 33};
    uint16_t bandValues[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t oldBarHeights[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t peak[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // The length of these arrays must be >= NUM_BANDS
    const uint16_t AMPLITUDE = 1;      // Depending on your audio source level, you may need to alter this value. Can be used as a 'sensitivity' control.
    const uint16_t NUM_BANDS = 8;         // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands
    const uint16_t TOP = 7;
    const uint16_t NOISE = 50; // Used as a crude noise filter, values below this are ignored

    uint16_t out[NUMBER_OF_SAMPLES]={};
    uint16_t data[NUMBER_OF_SAMPLES]={};

    void doCustomFFT()
    {
        for(uint16_t sample_nr = 0; sample_nr < NUMBER_OF_SAMPLES;sample_nr++)
        {
            data[sample_nr] = analogRead(AUDIO_IN_PIN);
        }

        custom_fft(data,out);
        // Serial.println(f);
    }
   

    //-----------------------------FFT Function----------------------------------------------//
    /*
    Code to perform Custom FFT on arduino

    1. in[]     : Input data array, gets changed by this function
    2. out[]    : 

    Original function by:
    Contact: abhilashpatel121@gmail.com
    Documentation & details: https://www.instructables.com/member/abhilash_patel/instructables/
    */

    void custom_fft(int16_t in[], int16_t out[])
    {
        FFT_GENERAL_DATA_TYPE temp_data_magnitude;
        FFT_GENERAL_DATA_TYPE  data_max, data_min, data_avg, data_magnitude;
        byte scale = 0;
        bool check = false;

        data_avg = 0;
        data_max = INT_FAST16_MIN;
        data_min = INT_FAST16_MAX;



        for (int i = 0; i < NUMBER_OF_SAMPLES; i++) // getting min max and average for scalling
        {            
            data_avg = data_avg + in[i];
            if (in[i] > data_max)
            {
                data_max = in[i];
            }
            if (in[i] < data_min)
            {
                data_min = in[i];
            }
        }

        // this works with up to 10 bit numbers in in[], otherwise we risk overflow when adding
        data_avg = data_avg >> LB_NUMBER_OF_SAMPLES; // data_avg / NUMBER_OF_SAMPLES, calculates actual average

        data_magnitude = data_max - data_min;
        temp_data_magnitude = data_magnitude;

        

        if (data_magnitude > MAX_MAGNITUDE)
        {
            while (temp_data_magnitude > MAX_MAGNITUDE)
            {
                temp_data_magnitude = temp_data_magnitude >> 1;
                scale ++;
            }
        }

        if (data_magnitude < MAX_MAGNITUDE)
        {
            while (temp_data_magnitude < MAX_MAGNITUDE)
            {
                temp_data_magnitude = temp_data_magnitude << 1;
                scale --;
            }

            // maximum scale should be below 1024
            scale += 1;
        }

        //scale all input values
        if(scale > 0)
        {
            for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
            {
                in[i] = in[i] - data_avg;
                in[i] = in[i] >> scale;
            }
        }

        if(scale < 0)
        {
            for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
            {
                in[i] = in[i] - data_avg;
                //TODO: inspect assembly how (-scale) is expanded
                in[i] = in[i] << (-scale);
            }
        }

        //reorder input by bit reversed addressing, store it in out temporarily

        for(int i = 0; i < NUMBER_OF_SAMPLES; i++)
        {
            out[i] = in[bit_reverse_index[i]];
            in[bit_reverse_index[i]] = 0;
        }

        // out now holds the real part and in holds the imaginary part of our values


        // create runs for the butterfly algorithm
        for(int i = 0; i < LB_NUMBER_OF_SAMPLES; i++)
        {
            //calculate the offset between samples
            int butterfly_index_offset = 1 << i; //This is 1 at first, then 2, 4, 8, 16, 32 (1<<5)
            int twiddle_factor_increase = NUMBER_OF_SAMPLES >> i + 1; // starts with 32, then 16, 8, 4, 2, 1
            int twiddle_factor_exponent = 0;

            for(int j = 0; j < NUMBER_OF_SAMPLES >> 1; j++)
            {
                //do a butterfly calculation:
                /*
                    twiddle factor = 
                    top value = top value + bottom value * twiddle factor
                    bottom value = top value - bottom value * twiddle factor
                */
               
                FFT_BUTTERFLY_DATA_TYPE imaginary_bottom = in[j+butterfly_index_offset];                
                FFT_BUTTERFLY_DATA_TYPE real_bottom = out[j+butterfly_index_offset];

                FFT_BUTTERFLY_DATA_TYPE real_twiddlefactor = omega_power_real(twiddle_factor_exponent);
                FFT_BUTTERFLY_DATA_TYPE imaginary_twiddlefactor = omega_power_imaginary(twiddle_factor_exponent);
                
                // use these variables to calculate bottom * twiddle factor
                FFT_BUTTERFLY_DATA_TYPE real_top = real_bottom * real_twiddlefactor - imaginary_bottom * imaginary_twiddlefactor;
                FFT_BUTTERFLY_DATA_TYPE imaginary_top = real_bottom * imaginary_twiddlefactor + imaginary_bottom * real_twiddlefactor;

                // holds bottom * twiddle factor
                real_bottom = real_top;
                imaginary_bottom = imaginary_top;

                // shift top as much as bottom was shifted due to twiddle factor
                imaginary_top = in[j] << OMEGA_SHIFT_AMOUNT;
                real_top = out[j] << OMEGA_SHIFT_AMOUNT;

                // hold the result for top
                real_twiddlefactor = real_top + real_bottom;
                imaginary_twiddlefactor = imaginary_top + imaginary_bottom;

                //calculate bottom values
                real_bottom = real_top - real_bottom;
                imaginary_bottom = imaginary_top-imaginary_bottom;

                //write back, undo the shift from the twiddle factor

                out[j] = real_top;
                in[j] = imaginary_top;

                out[j+butterfly_index_offset] = real_bottom;
                in[j+butterfly_index_offset] = imaginary_bottom;


                //adjust tiwddle factor for next calculation
                twiddle_factor_exponent += twiddle_factor_increase;
                twiddle_factor_exponent %= NUMBER_OF_SAMPLES / 2;
            }
        }

        //TODO: see if we need to reintroduce the scaling check

        //reverse input scaling

        if(scale > 0)
        {
            for(int i = 0; i < NUMBER_OF_SAMPLES; i++)
            {
                in[i] <<= scale;
                out[i] <<= scale;
            }
        }
        if(scale < 0)
        {
            for(int i = 0; i < NUMBER_OF_SAMPLES; i++)
            {
                in[i] <<= (-scale);
                out[i] <<= (-scale);
            }
        }

        //calculate Magnitudes

        for(int i = 0; i < NUMBER_OF_SAMPLES; i++)
        {
            out[i] = abs_complex(out[i], in[i]);
        }
        
      

        // Reset bandValues[]
        for (int i = 0; i < 8; i++)
        {
            bandValues[i] = 0;
        }

        for (int i = 1; i < (NUMBER_OF_SAMPLES / 2); i++)
        { // Don't use sample 0 and only first SAMPLES/2 are usable. Each array element represents a frequency bin and its value the amplitude.
            if (out[i] > NOISE)
            { // Add a crude noise filter

                // 8 bands, 12kHz top band
                if (i <= highestBandInBin[0])
                    bandValues[0] += (int)out[i];
                if (i > highestBandInBin[0] && i <= highestBandInBin[1])
                    bandValues[1] += (int)out[i];
                if (i > highestBandInBin[1] && i <= highestBandInBin[2])
                    bandValues[2] += (int)out[i];
                if (i > highestBandInBin[2] && i <= highestBandInBin[3])
                    bandValues[3] += (int)out[i];
                if (i > highestBandInBin[3] && i <= highestBandInBin[4])
                    bandValues[4] += (int)out[i];
                if (i > highestBandInBin[4] && i <= highestBandInBin[5])
                    bandValues[5] += (int)out[i];
                if (i > highestBandInBin[5] && i <= highestBandInBin[6])
                    bandValues[6] += (int)out[i];
                if (i > highestBandInBin[6] && i <= highestBandInBin[7])
                    bandValues[7] += (int)out[i];
            }
        }
        /////////////
        ///////////////
        //////////////
        equalizer::buildbars(bandValues, equalizer::BarNumber::Bars8, 7);
    }

    //---------------------------------omega function---------------------------------------//

    int omega_power_real(int exponent_nominator)
    {
        // check if we are in the 2nd quadrant
        if(exponent_nominator > NUMBER_OF_SAMPLES / 4)
        {
            return -omega_data[exponent_nominator - NUMBER_OF_SAMPLES / 4];
        }
        else
        {
            return omega_data[NUMBER_OF_SAMPLES / 4 - exponent_nominator];
        }
    }

    int omega_power_imaginary(int exponent_nominator)
    {
        // check if we are in the 2nd quadrant
        if(exponent_nominator > NUMBER_OF_SAMPLES / 4)
        {
            exponent_nominator = NUMBER_OF_SAMPLES / 2 - exponent_nominator;
        }
        return omega_data[exponent_nominator];
    }
    

    //--------------------------------------------------------------------------------//

    //--------------------------------Fast RSS----------------------------------------//

    int abs_complex(int real, int imaginary)
    {
        uint32_t target = (uint32_t)real * (uint32_t)real + (uint32_t)imaginary * (uint32_t)imaginary;
        uint32_t scale = 0x8000;
        uint32_t guess = 0;
        uint32_t guess_alt;

        for(int i = 0; i < 16; i++)
        {
            guess_alt = guess + scale;
            if(guess_alt*guess_alt <= target)
            {
                guess = guess_alt;
            }
            scale >> 1;
        }
        return guess;

    }

    int fastRSS(int a, int b)
    {
        if (a == 0 && b == 0)
        {
            return (0);
        }
        int min, max, temp1, temp2;
        byte clevel;
        if (a < 0)
        {
            a = -a;
        }
        if (b < 0)
        {
            b = -b;
        }
        clevel = 0;
        if (a > b)
        {
            max = a;
            min = b;
        }
        else
        {
            max = b;
            min = a;
        }

        if (max > (min + min + min))
        {
            return max;
        }
        else
        {
            temp1 = min >> 3; // min /8 , min 1
            if (temp1 == 0) // <8
            {
                temp1 = 1;
            }
            temp2 = min;
            while (temp2 < max)
            {
                temp2 = temp2 + temp1;
                clevel = clevel + 1;
            }
            temp2 = RSSdata[clevel];
            temp1 = temp1 >> 1;

            //just use a multiply command, this wastes so much time
            for (int i = 0; i < temp2; i++)
            {
                max = max + temp1;
            }
            return (max);
        }
    }
    //--------------------------------------------------------------------------------//
}