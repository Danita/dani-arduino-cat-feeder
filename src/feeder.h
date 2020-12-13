#pragma once

#include <Arduino.h>
#include <door.h>
#include <authorization.h>

class Feeder {

    Door door;
    Authorization auth;
    uint8_t IRPinRx;
        
    public:
    
        Feeder(
            uint8_t motorNumber, 
            uint8_t sensorPinA, 
            uint8_t sensorPinB,
            uint8_t rfidRxPin, 
            uint8_t RfidTxPin,
            uint8_t IRPinRx
            ):
            door(motorNumber, sensorPinA, sensorPinB),
            auth(rfidRxPin, RfidTxPin),
            IRPinRx(IRPinRx)
        {}

        void setup() {
            Serial.begin(9600);
            pinMode(IRPinRx,INPUT);
            door.setup();
            auth.setup();
            door.close();
            Serial.println("Feeder is ready.");
        }

        void loop() {
            door.loop();
            auth.loop();
            if (auth.isAuthorized()) {
                door.open();
            }
        }

        bool isCatPresent() {
            int val = analogRead(IRPinRx);
            return (val < 1000);
        }

};
