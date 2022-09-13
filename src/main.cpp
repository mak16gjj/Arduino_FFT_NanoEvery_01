#include "Arduino.h"
#include <Streaming.h>
#include <FastLED_helper.h>
#include <FFT_helper.h>
#include <adc_helper.h>
#include <approxfft_helper.h>
#include <msgeq7_helper.h>
#include <equalizer_helper.h>
#include <nrf24_helper.h>
#include <customfft_helper.h>

#include "Profiler.hpp"

#define Pin_Button 4
uint8_t last_Button_State = 0;

int analogValue;
unsigned long newTime2;
unsigned long newTime3;

//intervalls
unsigned long intervall_main = 50;
unsigned long prevmillis_main = 0;

//Function 0: Test
uint8_t f0_i = 11;

//Array for sending out
void sendArray();
uint8_t sendoutarray[10];
uint8_t mode = 1;
uint8_t submode = 1;
void Mode1_changeSubmode();
uint32_t subModeChangeIntervall = 10000;  //Submodechange alle 10s



void setup() {
  Serial.begin(115200);
  pinMode(Pin_Button, INPUT);

  adc::setSampleFrequency(adc::sampleFrequency::sampleF_18kHz);
  fastled::setup();
  msgeq7::begin();
  equalizer::setup();
  nrf24::setupSEND();
  delay(1000); 
  
  Serial.println("Starting nano Every!");
  
}

void loop() {

  //msgeq7::tick();
  fastled::fastled_tick();
  
  if (millis() > prevmillis_main + intervall_main)
  {
    prevmillis_main = millis();

    newTime2 = millis();
    if(digitalRead(Pin_Button) && last_Button_State == 0)
    {
      mode++;
      if(mode > 7)
        mode = 1;
      Serial.print("Mode changed to ");
      Serial.println(mode);
      sendArray();
    }
    last_Button_State = digitalRead(Pin_Button);


    
    if(mode == 2)
    {
      //Profiler::reset();
      //approxfft::doApproxFFT();
      customfft::doCustomFFT();
      Mode1_changeSubmode(); 
      //Profiler::start();
      //submode = 2;  //nur zum Testen des spezifischen Submodes
      sendArray();
      //Profiler::log();
      //uint32_t* results = Profiler::get_results();
      /*
      Serial << "ADC Capture Time: " << results[0] <<" us" << endl;
      Serial << "FFT Calculation Time: " << results[1] <<" us" << endl;
      Serial << "Bar Building Time: " << results[2] <<" us" << endl;
      Serial << "Send Array Time: " << results[3] <<" us" << endl;
      */
    }

    
    

    Serial.println(millis() - newTime2);    
    Serial << "Mode: " << mode << endl;

    


  }

}

void sendArray()
{

  if(mode == 1)  //Aus-Mode
  {
    submode = 1;
    sendoutarray[0] = ((mode & 0xF) << 4) | (submode & 0xF);
    sendoutarray[1] = 1;
    sendoutarray[2] = 0;
    sendoutarray[3] = 0;
    sendoutarray[4] = 0;
    sendoutarray[5] = 0;
    sendoutarray[6] = 0;
    sendoutarray[7] = 0;
    sendoutarray[8] = 0;
    sendoutarray[9] = 0;
  }
  if(mode == 2)  //Musik-Mode
  {
    sendoutarray[0] = ((mode & 0xF) << 4) | (submode & 0xF);
    sendoutarray[1] = equalizer::getMusicValue();
    sendoutarray[2] = equalizer::barValues[0];
    sendoutarray[3] = equalizer::barValues[1];
    sendoutarray[4] = equalizer::barValues[2];
    sendoutarray[5] = equalizer::barValues[3];
    sendoutarray[6] = equalizer::barValues[4];
    sendoutarray[7] = equalizer::barValues[5];
    sendoutarray[8] = equalizer::barValues[6];
    sendoutarray[9] = equalizer::barValues[7];
  }
  if(mode == 3)  //Rot-Mode
  {
    submode = 1;
    sendoutarray[0] = ((mode & 0xF) << 4) | (submode & 0xF);
    sendoutarray[1] = 1;
    sendoutarray[2] = 0;
    sendoutarray[3] = 0;
    sendoutarray[4] = 0;
    sendoutarray[5] = 0;
    sendoutarray[6] = 0;
    sendoutarray[7] = 0;
    sendoutarray[8] = 0;
    sendoutarray[9] = 0;
  }
  if(mode == 4)  //Grün-Mode
  {
    submode = 1;
    sendoutarray[0] = ((mode & 0xF) << 4) | (submode & 0xF);
    sendoutarray[1] = 1;
    sendoutarray[2] = 0;
    sendoutarray[3] = 0;
    sendoutarray[4] = 0;
    sendoutarray[5] = 0;
    sendoutarray[6] = 0;
    sendoutarray[7] = 0;
    sendoutarray[8] = 0;
    sendoutarray[9] = 0;
  }
  if(mode == 5)  //Blau-Mode
  {
    submode = 1;
    sendoutarray[0] = ((mode & 0xF) << 4) | (submode & 0xF);
    sendoutarray[1] = 1;
    sendoutarray[2] = 0;
    sendoutarray[3] = 0;
    sendoutarray[4] = 0;
    sendoutarray[5] = 0;
    sendoutarray[6] = 0;
    sendoutarray[7] = 0;
    sendoutarray[8] = 0;
    sendoutarray[9] = 0;
  }
  if(mode == 6)  //Weiß-Mode
  {
    submode = 1;
    sendoutarray[0] = ((mode & 0xF) << 4) | (submode & 0xF);
    sendoutarray[1] = 1;
    sendoutarray[2] = 0;
    sendoutarray[3] = 0;
    sendoutarray[4] = 0;
    sendoutarray[5] = 0;
    sendoutarray[6] = 0;
    sendoutarray[7] = 0;
    sendoutarray[8] = 0;
    sendoutarray[9] = 0;
  }
  if(mode == 7)  //Farbwechsel-Mode
  {
    submode = 1;
    sendoutarray[0] = ((mode & 0xF) << 4) | (submode & 0xF);
    sendoutarray[1] = 1;
    sendoutarray[2] = 0;
    sendoutarray[3] = 0;
    sendoutarray[4] = 0;
    sendoutarray[5] = 0;
    sendoutarray[6] = 0;
    sendoutarray[7] = 0;
    sendoutarray[8] = 0;
    sendoutarray[9] = 0;
  }

  nrf24::sendSEND(sendoutarray);
  Serial << "Send out: " << sendoutarray[0] << endl;

}

void Mode1_changeSubmode()
{
  static uint32_t prevModeChangeMillis = 0;
  if(millis() < prevModeChangeMillis) prevModeChangeMillis = 0;  //49-Tage-Überlauf-Schutz

  if(millis() - prevModeChangeMillis > subModeChangeIntervall)
  {
    submode = random(1,4);  //1-4
    prevModeChangeMillis = millis();
  }
}



