#include "fft.hpp"

#include "Profiler.hpp"
#include "debug_config.h"
#include "optimization_config.h"

#include <Streaming.h>

namespace fft
{
    int omega_power_real(int exponent_nominator);
    int omega_power_imaginary(int exponent_nominator);
    int abs_complex(FFT_DATA_TYPE real, FFT_DATA_TYPE imaginary);
    int fastRSS(FFT_DATA_TYPE a, FFT_DATA_TYPE b);
    

    const FFT_DATA_TYPE MAX_MAGNITUDE = 1024;
    const int OMEGA_SHIFT_AMOUNT = 8;
    

    //---------------------------------lookup data------------------------------------//
    const unsigned char bit_reverse_index[NUMBER_OF_SAMPLES] =
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
    #ifdef ARDUINO_USE_LARGE_SIN_LOOKUP
    const FFT_DATA_TYPE omega_data[3*NUMBER_OF_SAMPLES / 4 + 1] = // 1024 * sin values in pi / 32 steps  tp pi/2
    #else
    const FFT_DATA_TYPE omega_data[NUMBER_OF_SAMPLES / 4 + 1] = // 1024 * sin values in pi / 32 steps  tp pi/2
    #endif
        {
            0 >> (10 - OMEGA_SHIFT_AMOUNT),  //0
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
            1024 >> (10 - OMEGA_SHIFT_AMOUNT), // 16
            #ifdef ARDUINO_USE_LARGE_SIN_LOOKUP
            1019 >> (10 - OMEGA_SHIFT_AMOUNT), 
            1004 >> (10 - OMEGA_SHIFT_AMOUNT), 
            980 >> (10 - OMEGA_SHIFT_AMOUNT),  
            946 >> (10 - OMEGA_SHIFT_AMOUNT),  
            903 >> (10 - OMEGA_SHIFT_AMOUNT), 
            851 >> (10 - OMEGA_SHIFT_AMOUNT),  
            792 >> (10 - OMEGA_SHIFT_AMOUNT),  
            724 >> (10 - OMEGA_SHIFT_AMOUNT),  
            650 >> (10 - OMEGA_SHIFT_AMOUNT), 
            569 >> (10 - OMEGA_SHIFT_AMOUNT),  
            483 >> (10 - OMEGA_SHIFT_AMOUNT),  
            392 >> (10 - OMEGA_SHIFT_AMOUNT),  
            297 >> (10 - OMEGA_SHIFT_AMOUNT), 
            200 >> (10 - OMEGA_SHIFT_AMOUNT),  
            100 >> (10 - OMEGA_SHIFT_AMOUNT),  
            0 >> (10 - OMEGA_SHIFT_AMOUNT),   //32
            -100 >> (10 - OMEGA_SHIFT_AMOUNT),  
            -200 >> (10 - OMEGA_SHIFT_AMOUNT),  
            -297 >> (10 - OMEGA_SHIFT_AMOUNT), 
            -392 >> (10 - OMEGA_SHIFT_AMOUNT),  
            -483 >> (10 - OMEGA_SHIFT_AMOUNT),  
            -569 >> (10 - OMEGA_SHIFT_AMOUNT),  
            -650 >> (10 - OMEGA_SHIFT_AMOUNT), 
            -724 >> (10 - OMEGA_SHIFT_AMOUNT),  
            -792 >> (10 - OMEGA_SHIFT_AMOUNT),  
            -851 >> (10 - OMEGA_SHIFT_AMOUNT),  
            -903 >> (10 - OMEGA_SHIFT_AMOUNT), 
            -946 >> (10 - OMEGA_SHIFT_AMOUNT),  
            -980 >> (10 - OMEGA_SHIFT_AMOUNT),  
            -1004 >> (10 - OMEGA_SHIFT_AMOUNT), 
            -1019 >> (10 - OMEGA_SHIFT_AMOUNT), 
            -1024 >> (10 - OMEGA_SHIFT_AMOUNT), //48
            #endif
        };
    
    const unsigned char RSSdata[20] = {7, 6, 6, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2};
    //---------------------------------------------------------------------------------//

   

    void FFT::do_fft(FFT_DATA_TYPE in[], FFT_DATA_TYPE out[])
    {
        profiler::global_Profiler.reset();
        profiler::global_Profiler.start();
        char scale = scale_data(in);
        profiler::global_Profiler.log();

        //reorder input by bit reversed addressing, store it in out temporarily

        apply_bit_reverse_ordering(in, out);
         profiler::global_Profiler.log();

        // out now holds the real part and in holds the imaginary part of our values

        // in and out are switched in this call
        apply_butterfly(out, in);
         profiler::global_Profiler.log();

        //reverse input scaling

        rescale_data(in, out, scale);     
         profiler::global_Profiler.log();  

        #ifdef FFT_PROFILING_OUTPUT

        uint32_t* profiling_data = profiler::global_Profiler.get_results();


        Serial << "FFT Performance:\n";
        Serial << "Prescaleing:" << profiling_data[0] << " us\n";
        Serial << "Reordering:" << profiling_data[1] << " us\n";
        Serial << "Butterfly:" << profiling_data[2] << " us\n";
        Serial << "Rescaling:" << profiling_data[3] << " us\n";
        #endif

    }

    signed char FFT::scale_data(FFT_DATA_TYPE data[])
    {
        signed char scale = 0;
        FFT_DATA_TYPE  data_max, data_min, data_avg, data_magnitude;
        FFT_DATA_TYPE temp_data_magnitude;
        data_avg = 0;
        data_max = FFT_DATA_TYPE_MIN;
        data_min = FFT_DATA_TYPE_MAX;



        for (int i = 0; i < NUMBER_OF_SAMPLES; i++) // getting min max and average for scalling
        {            
            data_avg = data_avg + data[i];
            if (data[i] > data_max)
            {
                data_max = data[i];
            }
            if (data[i] < data_min)
            {
                data_min = data[i];
            }
        }

        // this works with up to 10 bit numbers in in[], otherwise we risk overflow when adding
        data_avg = data_avg >> LB_NUMBER_OF_SAMPLES; // data_avg / NUMBER_OF_SAMPLES, calculates actual average

        data_magnitude = data_max - data_min;
        temp_data_magnitude = data_magnitude;

        if(data_magnitude == 0)
        {
            return 0;
        }

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
                data[i] = data[i] - data_avg;
                data[i] = data[i] >> scale;
            }
        }

        if(scale < 0)
        {
            for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
            {
                data[i] = data[i] - data_avg;
                //TODO: inspect assembly how (-scale) is expanded
                data[i] = data[i] << (-scale);
            }
        }
        return scale;
    }

    void FFT::apply_bit_reverse_ordering(FFT_DATA_TYPE in[], FFT_DATA_TYPE out[])
    {
        for(int i = 0; i < NUMBER_OF_SAMPLES; i++)
        {
            out[i] = in[bit_reverse_index[i]];
            in[bit_reverse_index[i]] = 0;
        }
    }

#ifdef ARDUINO_SHIFT_10_OPTIMIZATION
    typedef struct
    {
        char byte0;
        char byte1;
        char byte2;
        char byte3;
    }FOUR_BYTES;

    typedef union
    {
        FFT_BUTTERFLY_DATA_TYPE butterfly;;
        FOUR_BYTES bytewise;
    }ARDUINO_SHIFT_10_OPTIMIZATION_UNION;
#endif

    void FFT::apply_butterfly(FFT_DATA_TYPE in[], FFT_DATA_TYPE out[])
    {
        int test = 0;
        for(machine_int i = 0; i < LB_NUMBER_OF_SAMPLES; i++)
        {
            //calculate the offset between samples
            machine_int butterfly_index_offset = 1 << i; //This is 1 at first, then 2, 4, 8, 16, 32 (1<<5)
            machine_int twiddle_factor_increase = NUMBER_OF_SAMPLES >> i + 1; // starts with 32, then 16, 8, 4, 2, 1
            machine_int twiddle_factor_exponent = 0;

            //create loop for batches
            for (machine_int j = 0; j < NUMBER_OF_SAMPLES >> (1+i); j++)
            {
                machine_int start_index = j << (i + 1);
                // create loop for each batch 
                for (machine_int k = 0; k < 1 << (i); k++)
                {
                    //do a butterfly calculation:
                    /*
                        twiddle factor =
                        top value = top value + bottom value * twiddle factor
                        bottom value = top value - bottom value * twiddle factor
                    */
                    FFT_BUTTERFLY_DATA_TYPE old_imaginary_bottom = out[start_index + butterfly_index_offset];
                    FFT_BUTTERFLY_DATA_TYPE old_real_bottom = in[start_index + butterfly_index_offset];

                    FFT_BUTTERFLY_DATA_TYPE real_twiddlefactor = omega_power_real(twiddle_factor_exponent);
                    FFT_BUTTERFLY_DATA_TYPE imaginary_twiddlefactor = omega_power_imaginary(twiddle_factor_exponent);

                    // use these variables to calculate bottom * twiddle factor
                    FFT_BUTTERFLY_DATA_TYPE real_product_long = old_real_bottom * real_twiddlefactor - old_imaginary_bottom * imaginary_twiddlefactor;
                    FFT_BUTTERFLY_DATA_TYPE imaginary_product_long = old_real_bottom * imaginary_twiddlefactor + old_imaginary_bottom * real_twiddlefactor;

                    // holds bottom * twiddle factor
                    #ifdef ARDUINO_SHIFT_10_OPTIMIZATION
                    ARDUINO_SHIFT_10_OPTIMIZATION_UNION real_product_union; 
                    ARDUINO_SHIFT_10_OPTIMIZATION_UNION imaginary_product_union;

                    real_product_union.butterfly = real_product_long;
                    imaginary_product_union.butterfly = imaginary_product_long;

                    real_product_union.bytewise.byte3=real_product_union.bytewise.byte2;
                    real_product_union.bytewise.byte2=real_product_union.bytewise.byte1;
                    real_product_union.bytewise.byte1=real_product_union.bytewise.byte0;

                    imaginary_product_union.bytewise.byte3=imaginary_product_union.bytewise.byte2;
                    imaginary_product_union.bytewise.byte2=imaginary_product_union.bytewise.byte1;
                    imaginary_product_union.bytewise.byte1=imaginary_product_union.bytewise.byte0;


                    FFT_DATA_TYPE real_product = real_product_union.butterfly;
                    FFT_DATA_TYPE imaginary_product = imaginary_product_union.butterfly;

                    real_product <<= OMEGA_SHIFT_AMOUNT-8;
                    imaginary_product <<= OMEGA_SHIFT_AMOUNT-8;

                    #else
                    FFT_DATA_TYPE real_product = real_product_long >> OMEGA_SHIFT_AMOUNT;
                    FFT_DATA_TYPE imaginary_product = imaginary_product_long >> OMEGA_SHIFT_AMOUNT;
                    #endif
                    // shift top as much as bottom was shifted due to twiddle factor
                    FFT_DATA_TYPE old_imaginary_top = out[start_index];
                    FFT_DATA_TYPE old_real_top = in[start_index];

                    // hold the result for top
                    FFT_DATA_TYPE real_top = old_real_top + real_product;
                    FFT_DATA_TYPE imaginary_top = old_imaginary_top + imaginary_product;

                    //calculate bottom values
                    FFT_DATA_TYPE real_bottom = old_real_top - real_product;
                    FFT_DATA_TYPE imaginary_bottom = old_imaginary_top - imaginary_product;

                    //write back, undo the shift from the twiddle factor

                    in[start_index] = real_top;
                    out[start_index] = imaginary_top;

                    in[start_index + butterfly_index_offset] = real_bottom;
                    out[start_index + butterfly_index_offset] = imaginary_bottom;


                    //adjust tiwddle factor for next calculation
                    twiddle_factor_exponent += twiddle_factor_increase;
                    twiddle_factor_exponent %= NUMBER_OF_SAMPLES / 2;
                    start_index++;
                }
            }
        }

        for(int i = 0; i < NUMBER_OF_SAMPLES; i++)
        {
            out[i] = abs_complex(out[i], in[i]);
        }
    }

    void FFT::rescale_data(FFT_DATA_TYPE in[], FFT_DATA_TYPE out[],signed char scaling)
    {
        if(scaling >= 0)
        {
            for(int i = 0; i < NUMBER_OF_SAMPLES; i++)
            {
                out[i] = in[i] << scaling;
            }
        }
        if(scaling < 0)
        {
            for(int i = 0; i < NUMBER_OF_SAMPLES; i++)
            {
                out[i] = in[i] >> (-scaling);
            }
        }
    }


    //---------------------------------omega function---------------------------------------//

    #ifdef ARDUINO_USE_LARGE_SIN_LOOKUP
    inline int omega_power_real(int exponent_nominator)
    #else
    int omega_power_real(int exponent_nominator)
    #endif
    {
        #ifndef ARDUINO_USE_LARGE_SIN_LOOKUP
        // check if we are in the 2nd quadrant
        if(exponent_nominator > NUMBER_OF_SAMPLES / 4)
        {
            return -omega_data[exponent_nominator - NUMBER_OF_SAMPLES / 4];
        }
        else
        {
            return omega_data[NUMBER_OF_SAMPLES / 4 - exponent_nominator];
        }
        #else
        return omega_data[exponent_nominator + NUMBER_OF_SAMPLES / 4];
        #endif
    }

    #ifdef ARDUINO_USE_LARGE_SIN_LOOKUP
    inline int omega_power_imaginary(int exponent_nominator)
    #else
    int omega_power_imaginary(int exponent_nominator)
    #endif
    {
        #ifndef ARDUINO_USE_LARGE_SIN_LOOKUP
        // check if we are in the 2nd quadrant
        if(exponent_nominator > NUMBER_OF_SAMPLES / 4)
        {
            exponent_nominator = NUMBER_OF_SAMPLES / 2 - exponent_nominator;
        }
        return omega_data[exponent_nominator];
        #else
        return omega_data[exponent_nominator];
        #endif;
    }
    

    //--------------------------------------------------------------------------------//

    //--------------------------------Fast RSS----------------------------------------//

    int abs_complex(FFT_DATA_TYPE real, FFT_DATA_TYPE imaginary)
    {
        return fastRSS(real, imaginary);
        
        

        uint32_t target = (uint32_t)real * (uint32_t)real + (uint32_t)imaginary * (uint32_t)imaginary;        
        uint32_t scale = 0x8000;
        uint32_t guess = 0;

        for(int i = 0; i < 16; i++)
        {
            uint32_t guess_alt;
            guess_alt = guess + scale;
            if(guess_alt*guess_alt <= target)
            {
                guess = guess_alt;
            }
            scale >> 1;
        }
        return guess;
        

    }

    int fastRSS(FFT_DATA_TYPE a, FFT_DATA_TYPE b)
    {
        if (a == 0 && b == 0)
        {
            return (0);
        }
        FFT_DATA_TYPE min, max;
        signed char clevel;
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
            FFT_DATA_TYPE temp1, temp2;
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
