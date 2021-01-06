#pragma once

#include <Arduino.h>
#include <AFMotor.h>

class Door {

    AF_DCMotor motor;
    uint8_t sensorAPin;
    uint8_t sensorBPin;
    uint8_t lightPin;
    bool isOpening = false;
    bool isClosing = false;

    public:
    
        Door(uint8_t motorNumber, uint8_t pinA, uint8_t pinB):
            motor(motorNumber),
            sensorAPin(pinA),
            sensorBPin(pinB)
        {}

        void setup() {
            pinMode(sensorAPin, INPUT_PULLUP);
            pinMode(sensorBPin, INPUT_PULLUP);
            motor.setSpeed(150);
            motor.run(RELEASE);
            Serial.println("[Door] is ready, speed 150.");
        }

        void loop() {

            if (isOpening) {
                if (isOpen()) {
                    motor.run(RELEASE);
                    Serial.println("[Door] is now open.");
                    isOpening = false;
                } else {
                    motor.run(BACKWARD);
                }
            }

            if (isClosing) {
                if (isClosed()) {
                    motor.run(RELEASE);
                    Serial.println("[Door] is now closed.");
                    isClosing = false;
                } else {
                    motor.run(FORWARD);
                }
            }
        }

        bool isOpen() {
            return digitalRead(sensorAPin) == HIGH && digitalRead(sensorBPin) == LOW;
        }

        bool isClosed() {
            return digitalRead(sensorAPin) == LOW && digitalRead(sensorBPin) == HIGH;
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
