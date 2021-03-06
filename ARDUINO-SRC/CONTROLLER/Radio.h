#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "settings.h"

#ifndef RADIO_H
#define RADIO_H
bool secondRead = false; // help to  brake recursive in fucntion RadioRead;

RF24 radio(CE_PIN, CSN_PIN); // CE, CSN

void RadioInit()
{
    while (!radio.begin())
    {
        Serial.println("Waiting for start radio");
        delay(1000);
    }
    Serial.println("Radio Started");
    radio.setPALevel(RF24_PA_MAX);
    radio.setDataRate(RF24_250KBPS);
}

String RadioRead(uint8_t channel, const byte *address, uint8_t attempt)
{
    // Serial.print("Reaing Start");

    radio.openReadingPipe(channel, address);
    radio.startListening();

    int startTime = millis();
    int endTime = millis();

    // try to get readings about 2 seconds
    while (endTime - startTime <= READING_WAITING)
    {
        if (radio.available())
        {
            char text[32] = "";
            radio.read(&text, sizeof(text));
            return text;
        }
        endTime = millis();
    }

    // if read error occure try read again from node
    attempt--;
    if (attempt > 0)
    {
        Serial.print("trying to read attempt : ");
        Serial.println(attempt);
        return RadioRead(channel, address, attempt);
    }

    // still canot read ther is some falt in node
    radio.stopListening();
    return "e";
}

bool RadioWrite(String data, const byte *address, uint8_t attempt)
{
    radio.openWritingPipe(address);
    radio.stopListening();

    const char datatoWrite[READ_WRITEBUFF_SIZE];
    data.toCharArray(datatoWrite, READ_WRITEBUFF_SIZE);

    if (radio.write(&datatoWrite, sizeof(datatoWrite)))
    {
        return true;
    }

    // if write error occure try again;
    attempt--;
    if (attempt > 0)
    {
        delay(250); // give some delay before try next attempt
        return RadioWrite(data, address, attempt);
    }

    // stil canot write then ther is erro in node
    return false;
}

#endif