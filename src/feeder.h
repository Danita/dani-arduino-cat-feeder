#pragma once

#include <Arduino.h>
#include <door.h>
#include <authorization.h>
#include <neotimer.h>

class Feeder {

    Door door;
    Authorization auth;
    uint8_t manualTogglePin;
    Neotimer closeDoorTimer;

    enum states {
        F_IDLE,
        F_OPENING,
        F_OPEN,
        F_CLOSING,
        F_CLOSED
    };

    uint8_t state = F_IDLE;
        
    public:
    
        Feeder(
            uint8_t motorNumber, 
            uint8_t sensorPinA, 
            uint8_t sensorPinB,
            uint8_t rfidRxPin, 
            uint8_t RfidTxPin,
            uint8_t manualTogglePin
            ):
            door(motorNumber, sensorPinA, sensorPinB),
            auth(rfidRxPin, RfidTxPin),
            manualTogglePin(manualTogglePin)
        {}

        bool isManualBtnPressed() {
            int ret = digitalRead(manualTogglePin);
            return ret == LOW;
        }

        void setup() {
            Serial.begin(9600);
            door.setup();
            auth.setup();
            closeDoorTimer = Neotimer(10000);
            Serial.println("[Feeder] is ready.");
            pinMode(manualTogglePin, INPUT_PULLUP);
        }

        void loop() {

            switch(state) {
                
                case F_IDLE:
                    if (auth.isAuthenticated()) {
                        door.close();
                        state = F_CLOSING;
                        Serial.println("Tag is authenticated, F_IDLE to F_CLOSING.");
                    } else {
                        door.open();
                    }
                break;

                case F_CLOSING:
                    auth.flush(); // clear any tags read during door closing
                    if (door.isClosed()) {
                        state = F_CLOSED;
                        Serial.println("F_CLOSING to F_CLOSED.");
                    }
                break;

                case F_CLOSED:
                    if (closeDoorTimer.done()) {
                        Serial.println("Timer done");
                        closeDoorTimer.reset();
                        if (auth.isAuthenticated()) {
                            closeDoorTimer.start();
                            Serial.println("Authenticated after done, restarted open timer...");
                        } else {
                            door.open();
                            state = F_OPENING;
                            Serial.println("F_CLOSED to F_OPENING.");
                        }
                    } else {
                        if (!closeDoorTimer.waiting()) {
                            closeDoorTimer.start();
                            Serial.println("Started open timer...");
                        } else {
                            if (auth.isAuthenticated()) {
                                closeDoorTimer.reset();
                                closeDoorTimer.start();
                                Serial.println("Authenticated during wait, restarted open timer...");
                            }
                        }
                    }
                break;

                case F_OPENING:
                    if (door.isOpen()) {
                        state = F_IDLE;
                        Serial.println("Door is open, F_OPENING to F_IDLE.");
                    } else if (auth.isAuthenticated()) {
                        door.close();
                        state = F_CLOSING;
                        Serial.println("Tag is authenticated, F_OPENING to F_CLOSING.");
                    }
                break;
            }

            door.loop();
            auth.loop();
        }

};
