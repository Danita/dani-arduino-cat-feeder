#pragma once

#include <Arduino.h>
#include <door.h>
#include <authorization.h>

class Feeder {

    Door door;
    Authorization auth;
    uint8_t IRPinRx;

    enum states {
        F_IDLE,
        F_OPENING,
        F_OPEN,
        F_CLOSING,
    };

    uint8_t state = F_IDLE;
        
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

        bool isCatPresent() {
            int val = analogRead(IRPinRx);
            return (val < 1000);
        }

        void setup() {
            Serial.begin(9600);
            door.setup();
            auth.setup();
            Serial.println("Feeder is ready.");
        }

        void loop() {

            switch(state) {
                
                case F_IDLE:
                    if (auth.isAuthorized()) {
                        door.open();
                        state = F_OPENING;
                        Serial.println("F_IDLE to F_OPENING state.");
                    } else {
                        door.close();
                    }
                break;

                case F_OPENING:
                    if (door.isOpen()) {
                        state = F_OPEN;
                        Serial.println("F_OPENING to F_OPEN state.");
                    }
                break;

                case F_OPEN:
                    if (!isCatPresent()) {
                        door.close();
                        state = F_CLOSING;
                        Serial.println("F_OPEN to F_CLOSING state.");
                    }
                break;

                case F_CLOSING:
                    if (isCatPresent()) {
                        door.open();
                        state = F_OPENING;
                        Serial.println("F_CLOSING to F_OPENING state because of cat present in door!");
                    }
                    if (door.isClosed()) {
                        state = F_IDLE;
                        Serial.println("F_CLOSING to F_IDLE state.");
                    }
                break;
            }

            door.loop();
            auth.loop();
        }

};
