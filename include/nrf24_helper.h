#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

namespace nrf24
{

    RF24 radio(7, 8); // CE, CSN

    const byte address[6] = "23532";

    // Received value
    uint8_t receivedvalue[10];

    void setupSEND()
    {
        radio.begin();
        radio.openWritingPipe(address);
        radio.setPALevel(RF24_PA_HIGH);
        radio.setDataRate( RF24_2MBPS );
        radio.setRetries(0,0);
        radio.disableCRC();
        radio.stopListening();
    }

    void sendSEND(uint8_t _sendvalue[10])
    {
        uint8_t sendvalue[10];

        for (uint8_t i = 0; i < 10; i++)
            sendvalue[i] = _sendvalue[i];
        radio.write(&sendvalue, sizeof(sendvalue));
    }

    void setupRECEIVE()
    {
        radio.begin();
        radio.openReadingPipe(0, address);
        radio.setPALevel(RF24_PA_LOW);
        radio.setDataRate( RF24_2MBPS );
        radio.startListening();
    }

    void tickRECEIVE()
    {
        if (radio.available())
        {

            radio.read(&receivedvalue, sizeof(receivedvalue));
            Serial.println("Array received");
        }
    }
}