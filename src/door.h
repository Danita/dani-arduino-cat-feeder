#pragma once

#include <Arduino.h>
#include <AFMotor.h>

class Door {
        AF_DCMotor motor;
        const byte sensorAPin;
        const byte sensorBPin; 
        bool isOpening = false;
        bool isClosing = false;

    public:
    
        Door(uint8_t motorNumber, byte pinA, byte pinB):
            motor(motorNumber),
            sensorAPin(pinA),
            sensorBPin(pinB)
        {}

        void setup() {
            pinMode(sensorAPin, INPUT_PULLUP);
            pinMode(sensorBPin, INPUT_PULLUP);
            motor.setSpeed(255);
            motor.run(RELEASE);
            Serial.println("[Door] is ready.");
        }

        void loop() {
            
            if (isOpening) {
                if (isOpen()) {
                    motor.run(RELEASE);
                    Serial.println("[Door] is now open.");
                    isOpening = false;
                } else {
                    motor.run(FORWARD);
                }
            }

            if (isClosing) {
                if (isClosed()) {
                    motor.run(RELEASE);
                    Serial.println("[Door] is now closed.");
                    isClosing = false;
                } else {
                    motor.run(BACKWARD);
                }
            }
        }

        bool isOpen() {
            return digitalRead(sensorAPin) == LOW && digitalRead(sensorBPin) == HIGH;
        }

        bool isClosed() {
            return digitalRead(sensorAPin) == HIGH && digitalRead(sensorBPin) == LOW;
        }

        void open() {
            if (isOpen() || isOpening) {
                return;
            }
            Serial.println("[Door] opening... ");
            isOpening = true;
            isClosing = false;
        }

        void close() {
            if (isClosed() || isClosing) {
                return;
            }
            Serial.println("[Door] closing... ");
            isOpening = false;
            isClosing = true;
        }

};
