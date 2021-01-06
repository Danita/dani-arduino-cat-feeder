#pragma once

#include <Arduino.h>
#include <door.h>
#include <authorization.h>
#include <neotimer.h>

class Feeder {

    Door door;
    Authorization auth;
    uint8_t IRPinRx;
    uint8_t manualTogglePin;
    Neotimer closeDoorTimer;

    enum states {
        F_IDLE,
        F_OPENING,
        F_OPEN,
        F_CLOSING,
        F_MANUALLY_OPENING,
        F_MANUALLY_OPEN
    };

    uint8_t state = F_IDLE;
        
    public:
    
        Feeder(
            uint8_t motorNumber, 
            uint8_t sensorPinA, 
            uint8_t sensorPinB,
            uint8_t rfidRxPin, 
            uint8_t RfidTxPin,
            uint8_t IRPinRx,
            uint8_t manualTogglePin
            ):
            door(motorNumber, sensorPinA, sensorPinB),
            auth(rfidRxPin, RfidTxPin),
            IRPinRx(IRPinRx),
            manualTogglePin(manualTogglePin)
        {}

        bool isCatPresent() {
            int val = analogRead(IRPinRx);
            return (val < 1000);
        }

        bool isManualBtnPressed() {
            int ret = digitalRead(manualTogglePin);
            return ret == LOW;
        }

        void setup() {
            Serial.begin(9600);
            door.setup();
            auth.setup();
            closeDoorTimer = Neotimer(5000);
            Serial.println("[Feeder] is ready.");
            pinMode(manualTogglePin, INPUT_PULLUP);
        }

        void loop() {

            switch(state) {
                
                case F_IDLE:
                    if (auth.isAuthorized()) {
                        door.open();
                        state = F_OPENING;
                        Serial.println("Tag is authorized, F_IDLE to F_OPENING.");
                    } else {
                        door.close();
                    }

                    if (isManualBtnPressed()) {
                        door.open();
                        state = F_MANUALLY_OPENING;
                        Serial.println("Manual toggle button is pressed, F_IDLE to F_MANUALLY_OPENING.");
                    }
                break;

                case F_OPENING:
                    if (door.isOpen()) {
                        state = F_OPEN;
                        Serial.println("F_OPENING to F_OPEN.");
                    }
                break;

                case F_OPEN:
                    if (closeDoorTimer.done()) {
                        Serial.println("Timer done");
                        if (!isCatPresent()) {
                            closeDoorTimer.reset();
                            door.close();
                            state = F_CLOSING;
                            Serial.println("Cat not present, F_OPEN to F_CLOSING.");
                        } else {
                            Serial.println("Cat present, restarting close timer");
                            closeDoorTimer.start();
                        }
                    } else {
                        if (!closeDoorTimer.waiting()) {
                            closeDoorTimer.start();
                            Serial.println("Started close timer...");
                        }
                    }
                break;

                case F_CLOSING:
                    if (door.isClosed()) {
                        state = F_IDLE;
                        Serial.println("Door is closed, F_CLOSING to F_IDLE.");
                    }
                break;

                case F_MANUALLY_OPENING:
                    if (door.isOpen()) {
                        state = F_MANUALLY_OPEN;
                        Serial.println("F_MANUALLY_OPENING to F_MANUALLY_OPEN.");
                    }
                break;

                case F_MANUALLY_OPEN:
                    if (isManualBtnPressed()) {
                        Serial.println("Manual button pressed, closing door");
                        door.close();
                    }
                    if (door.isClosed()) {
                        auth.flush(); // clear any tags read during manual mode.
                        state = F_IDLE;
                        Serial.println("Door is closed, F_MANUALLY_OPEN to F_IDLE.");
                    }
                break;
            }

            door.loop();
            auth.loop();
        }

};
