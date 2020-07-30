#include "arduino.hpp"
#include "board.h"
#include "arduino.hpp"
#include "board.h"

int ledPin = 13;
int motionPin = 2;
int motion;
int motionPrev;

void setup(void) 
{
    // Initialise pins as OUTPUT/INPUT
    pinMode(ledPin, OUTPUT);
    pinMode(motionPin, INPUT);

    // Initialise serial
    Serial.begin(9600);
}

void loop(void)
{
    motion = digitalRead(motionPin);
    if (motion != motionPrev && motion == HIGH)
    {
        digitalWrite(ledPin, HIGH);
        Serial.println("Motion detected.");
      motionPrev = motion;
    }
    else if (motion == LOW)
    {
        digitalWrite(ledPin, LOW);
      motionPrev = motion;
    }
    delay(100);


}
int main(void)
{
    /* run the Arduino setup */
    setup();
    /* and the event loop */
    while (1) {
        loop();
    }
    /* never reached */
    return 0;
}
