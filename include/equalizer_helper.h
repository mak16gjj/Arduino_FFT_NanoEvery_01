#pragma once

#include <Arduino.h>
#include <FastLED.h>

namespace equalizer
{

    
    

    enum BarNumber
    {
        Bars4 = 4,
        Bars6 = 6,
        Bars8 = 8,
    };
    //Bar Heights
    uint32_t barHeights[] =     {0,0,0,0,0,0,0,0};
    uint8_t oldBarHeights[] =   {0,0,0,0,0,0,0,0};
    uint8_t peaks[] =           {0,0,0,0,0,0,0,0};
    //Fixed scaling factors, divided by 100
    uint8_t barScalePercent[] = {100,100,100,100,100,100,100,100}; 
    //Dynamic scaling factors, divided by 100
    uint8_t barScaleDynamic[] = {100,100,100,100,100,100,100,100};
    uint32_t barGlobalScale = 100;
    const uint32_t barGlobalScale_max = 50;
    int8_t maxbar = 0;

    //Bass Detection 1
    void detectBass1(uint16_t bassValue);
    const uint16_t bass_nrs1 = 100;
    static uint8_t bassValues1[bass_nrs1];
    static uint16_t bassI1 = 0;

    //Bass Detection 2
    void detectBass2(uint16_t bassValue);
    const uint16_t bass_nrs2 = 100;
    static uint8_t bassValues2[bass_nrs2];
    static uint16_t bassI2 = 0;

    //Volume in Dynamic Percentage
    void calcVolPercent(float volumeValue);

    //Detect Bass peaks > x*Mean
    void detectBass_3(float bassValue);

    //Returns uint8_t: &0x80 => BassPeak? (0/1), &0x7F => VolumePercentage (0-100)
    uint8_t getMusicValue();
    uint8_t volPercentage = 0;
    uint8_t bassPeak = 0;

    //Returns 8 EQ-Barheights (0-8)
    uint8_t barValues[8]; 



    void setup()
    {
        
    }


    //Takes 8 bin values, calculates 4, 6, oder 8 bar heights with max. value
    void buildbars(uint16_t bandValues[], BarNumber bars, uint8_t maxheight)
    {
        for (int8_t bar = 0; bar < 8; bar++)
        {
            // Scale the bars for the display
            barHeights[bar] = int(sqrt(long((bandValues[bar] * (barScalePercent[bar] / 100.0)))) / 1);

            if(barHeights[bar] > barHeights[maxbar])
                maxbar = bar;
        }
        barGlobalScale = min(100 * maxheight / barHeights[maxbar], barGlobalScale_max);

        //barGlobalScale Mean
        static const uint8_t bgsSamples = 10;
        static uint16_t bgsValues[bgsSamples];
        static uint16_t bgsI = 0;
        static uint32_t bgsMean = 0;
        bgsValues[bgsI] = barGlobalScale;
        bgsI++;
        if(bgsI >= bgsSamples) bgsI = 0;

        bgsMean = 0;
        for(uint8_t i = 0; i < bgsSamples;i++)
        {
            bgsMean += bgsValues[i];
        }
        bgsMean = max(round(float(bgsMean)/float(bgsSamples)),1);

        barGlobalScale = bgsMean;
        //barGlobalScale Mean

        /*
        Serial.print(">GlobalScale:");
        Serial.println(barGlobalScale)
        */;
        

        for (int8_t bar = 0; bar < 8; bar++)
        {
            float _scale = barGlobalScale / 100.0;
            barHeights[bar] = int(barHeights[bar] * _scale);
        }
        for (uint8_t bar = 0; bar < 8; bar++)
        {

            //Serial.print(barHeights[bar]);
            //Serial.print(" | \t");


            if (barHeights[bar] > maxheight)
                barHeights[bar] = maxheight;

            // Small amount of averaging between frames
            barHeights[bar] = ((oldBarHeights[bar] * 1) + barHeights[bar]) / 2;

            // Move peak up
            if (barHeights[bar] > peaks[bar])
            {
                peaks[bar] = min(maxheight, barHeights[bar]);
            }

            barValues[bar] = barHeights[bar];
            // Draw bars
            fastled::drawBar(bar, barHeights[bar]);

            // Save oldBarHeights for averaging later
            oldBarHeights[bar] = barHeights[bar];
        }

        detectBass1(4*bandValues[0]+bandValues[1]);
        //detectBass2(bandValues[1]+bandValues[2]+bandValues[3]+bandValues[4]+bandValues[5]+bandValues[6]+bandValues[7]);

        float bandFloats[8];
        for (uint8_t i = 0; i < 8;i++)
        {
            bandFloats[i] = min(sqrt(bandValues[i]), 100.0);
        }
        double _val = (100*((4*bandFloats[0]+2*bandFloats[1])/5))/max((bandFloats[1]+bandFloats[2]+bandFloats[3]+bandFloats[4]+bandFloats[5]+bandFloats[6]+bandFloats[7]), 1);
        _val= min(_val, 150);
        //detectBass_lib1(_val); //ok
        detectBass_3(_val);
        calcVolPercent(bandFloats[0]+bandFloats[1]+bandFloats[2]+bandFloats[3]+bandFloats[4]+bandFloats[5]+bandFloats[6]+bandFloats[7]);
    }

    //Not used
    void detectBass1(uint16_t bassValue)
    { 
        static uint64_t lastdecrement = millis();
        //Serial.print("|| S: ");
        if(bassValue > 0)
        {
            bassValues1[bassI1] = bassValue;
            bassI1++;
            if(bassI1 >= bass_nrs1)
                bassI1 = 0;
        }
        uint32_t bassAvg = 0;
        uint16_t nonezero = 0;
        for(uint16_t i = 0; i < bass_nrs1; i++)
        {
            if(bassValues1[i] > 0)
            {
                bassAvg += bassValues1[i];
                nonezero++;
            }
        }
        bassAvg /= nonezero;

        
        if(millis() - lastdecrement > 500 && bassAvg > 1)
        {
            bassValues1[bassI1 > 0 ? bassI1-1 : bass_nrs1 - 1] = 1;
            bassI1++;
            lastdecrement = millis();
        }
        

        //Serial.print(bassAvg);
        //Serial.print("\t >");
        //Serial.print(bassValue);
        //Serial.print("\t");
        uint32_t bassFactor = bassValue > 0 ? (sqrt((bassValue*100) / bassAvg)) : 0;
        
        //Serial.print(bassFactor);
        //Serial.print("\t");

        if(bassFactor > 10)
        {
            fastled::flashSides1(250);
        }
    }

    //Not used
    void detectBass2(uint16_t bassValue)
    { 
        static uint64_t lastdecrement = millis();
        Serial.print("|| S: ");
        if(bassValue > 0)
        {
            bassValues2[bassI2] = bassValue;
            bassI2++;
            if(bassI2 >= bass_nrs2)
                bassI2 = 0;
        }
        uint32_t bassAvg = 0;
        uint16_t nonezero = 0;
        for(uint16_t i = 0; i < bass_nrs2; i++)
        {
            if(bassValues2[i] > 0)
            {
                bassAvg += bassValues2[i];
                nonezero++;
            }
        }
        bassAvg /= nonezero;

        
        if(millis() - lastdecrement > 500 && bassAvg > 1)
        {
            bassValues2[bassI2 > 0 ? bassI2-1 : bass_nrs2 - 1] = 1;
            bassI2++;
            lastdecrement = millis();
        }
        uint32_t bassFactor = bassValue > 0 ? (sqrt((bassValue*100) / bassAvg)) : 0;

        /*
        Serial.print(bassAvg);
        Serial.print("\t >");
        Serial.print(bassValue);
        Serial.print("\t");
        Serial.print(bassFactor);
        Serial.print("\t");
        */

        if(bassFactor > 10)
        {
            fastled::flashSides2(250);
        }
    }

    
    void detectBass_3(float bassValue)
    {
        static const uint16_t bassSamples = 50;
        static uint16_t bassValues[bassSamples];
        static uint16_t bassI = 0;
        static uint32_t bassMean = 0;

        bassValues[bassI] = bassValue > 0 ? min(round(bassValue), bassMean*5) : round(bassMean*0.5);
        bassI++;
        if(bassI >= bassSamples) bassI = 0;

        bassMean = 0;
        for(uint8_t i = 0; i < bassSamples;i++)
        {
            bassMean += bassValues[i];
        }
        bassMean = max(round(float(bassMean)/float(bassSamples)),1);

        uint8_t peak = 0;
        bassPeak = 0;
        if(round(bassValue) > bassMean * 2)
        {
            peak = 1;
            bassPeak = 1;
            fastled::flashSides2(250);
        }

        /*
        Serial.print(">Bassdata:");
        Serial.println(round(bassValue)); // print data
        Serial.print(">Basspeak:");
        Serial.println(peak*10); // print peak status
        Serial.print(">BassMean:");
        Serial.println(bassMean); // print moving average
        */
    }
    
    //Dynamic Volume in Percent -  Variables


    //Dynamic Volume in Percent
    void calcVolPercent(float volumeValue)
    {
        //Dynamic Volume in Percent -  Variables
        static float ek_0 = 0;  //recent input
        static float ek_1 = 0;  //last input
        static float uk_0 = 0;  //recent output
        static float uk_1 = 0;  //last output
        static float uk_2 = 0;  //2nd last output
        static const uint8_t volSamples = 50;
        static uint16_t volValues[volSamples];
        static uint16_t volI = 0;
        static uint32_t volMean = 0;


        ek_0 = volumeValue > 0 ? volumeValue : ek_1*0.5;

        uk_0 = 4*(0.1*ek_0 + 0.15*ek_1 - 0.05*uk_1 - 0.05*uk_2);
        uk_2 = uk_1;
        uk_1 = uk_0;
        ek_1 = ek_0;


        //volValues[volI] = volumeValue > 0 ? round(volumeValue) : round(volMean*0.5);
        volValues[volI] = uk_0 > 0 ? round(uk_0) : round(volMean*0.5);
        volI++;
        if(volI >= volSamples) volI = 0;

        volMean = 0;
        for(uint8_t i = 0; i < volSamples;i++)
        {
            volMean += volValues[i];
        }
        volMean = max(round(float(volMean)/float(volSamples)),1);

        float volPercent = min(((uk_0*66) / float(volMean)), 100);

        volPercentage = constrain(round(volPercent), 0, 100);

        /*
        Serial.print(">Vdata:");
        Serial.println(ek_0); // print data

        Serial.print(">Vfiltered:");
        Serial.println(uk_0); // print moving average

        Serial.print(">VMean:");
        Serial.println(volMean); // print moving average

        Serial.print(">VPercent:");
        Serial.println(volPercent); 
        */
    }

    uint8_t getMusicValue()
    {
        return (volPercentage | (bassPeak << 7));
    }





}
