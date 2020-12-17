#include <Arduino.h>
#include <feeder.h>

#define MOTOR_NUMBER 1
#define MOTOR_POS_SENSOR_A_PIN 14
#define MOTOR_POS_SENSOR_B_PIN 15
#define RFID_RX_PIN 17
#define RFID_TX_PIN 13 // unused
#define IR_RX_PIN 4
#define MOTOR_MANUAL_TOGGLE_PIN 16
#define LIGHT_PIN 2

Feeder feeder(
    MOTOR_NUMBER,
    MOTOR_POS_SENSOR_A_PIN,
    MOTOR_POS_SENSOR_B_PIN,
    RFID_RX_PIN,
    RFID_TX_PIN,
    IR_RX_PIN,
    MOTOR_MANUAL_TOGGLE_PIN,
    LIGHT_PIN
);

void setup() {
    
    feeder.setup();
}

void loop() {
    feeder.loop();
}