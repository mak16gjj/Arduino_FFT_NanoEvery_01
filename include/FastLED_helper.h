#pragma once

#include <Arduino.h>
#include <FastLED.h>

namespace fastled
{

    // LEDs:
    const uint16_t NUM_LEDS_X = 10;
    const uint16_t NUM_LEDS_Y = 7;
    const uint16_t NUM_LEDS = NUM_LEDS_X * NUM_LEDS_Y;
    const uint16_t DATA_PIN = 2;

    CRGB leds[NUM_LEDS]; // eindimensional, zum an den stripe schicken

    //Effects
    void flashSides1(uint64_t dur);
    uint64_t sidemillis1 = 0;
    void flashSides2(uint64_t dur);
    uint64_t sidemillis2 = 0;

    void setup()
    {
        pinMode(DATA_PIN, OUTPUT);
        FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
        FastLED.setCorrection(TypicalLEDStrip);
        FastLED.setTemperature(Tungsten40W);
        FastLED.setBrightness(25);
        FastLED.setDither(0);

        for (uint8_t i = 0; i < NUM_LEDS; i++)
            leds[i] = CRGB::Green;
        FastLED.show();
    }

    void fastled_tick()
    {
        if(millis() > sidemillis1)
            flashSides1(0);
        if(millis() > sidemillis2)
            flashSides2(0);

    }

    void drawBar(int _band, int _height)
    {


        for (int y = 0; y <= _height; y++)
        {
            leds[((6 - y) * 10) + _band + 1] = CRGB::Red; // normal palette access
            // leds[((6-y)*10)+_band + 1] = CRGB::Red;
            //Serial.println(map(y, 0, 6, 0, 255));
        }
        for(int y = _height + 1; y < NUM_LEDS_Y;y++)
        {
            leds[((6 - y) * 10) + _band + 1] = CRGB::Black; // normal palette access
        }
        FastLED.show();
    }

    void flashSides1(uint64_t dur)
    {
        uint8_t sideleds[] = {0,10,20,30,40,50,60};

        if(dur > 0)
        {
            for (uint8_t led : sideleds)
            {
                leds[led] = CRGB::White;
            }
            FastLED.show();
            sidemillis1 = millis() + dur;
        }
        else //turn off
        {
            for (uint8_t led : sideleds)
            {
                leds[led] = CRGB::Black;
            }
            FastLED.show();
        }
    }
    void flashSides2(uint64_t dur)
    {
        uint8_t sideleds[] = {9,19,29,39,49,59,69};

        if(dur > 0)
        {
            for (uint8_t led : sideleds)
            {
                leds[led] = CRGB::Blue;
            }
            FastLED.show();
            sidemillis2 = millis() + dur;
        }
        else //turn off
        {
            for (uint8_t led : sideleds)
            {
                leds[led] = CRGB::Black;
            }
            FastLED.show();
        }
    }

}