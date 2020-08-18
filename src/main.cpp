#include <Arduino.h>
#include <SoftwareSerial.h>
#include <AFMotor.h>
#include <rdm6300.h>
#include <NewPing.h>

#define MOTOR_NUMBER 1
#define MOTOR_POS_SENSOR_A_PIN 14
#define MOTOR_POS_SENSOR_B_PIN 15
#define MOTOR_MANUAL_TOGGLE_PIN 16
#define RFID_RX_PIN 17
#define SONAR_ECHO_PIN 18
#define SONAR_TRIGGER_PIN 19
#define SONAR_MAX_DISTANCE 13 // Distance threshold

#define MODE_AUTO 0
#define MODE_MANUAL 1

uint32_t AUTHORIZED_TAG_ID = 8603689; // Mily
// uint32_t AUTHORIZED_TAG_ID = 2767327; // Andy
int mode = MODE_AUTO;
long startMillis;
const int PRESENCE_TIMEOUT = 3000; // Time before closing the tray
bool isEating = false;

AF_DCMotor motor(MOTOR_NUMBER);
Rdm6300 rdm6300;
NewPing sonar(SONAR_TRIGGER_PIN, SONAR_ECHO_PIN, SONAR_MAX_DISTANCE);

bool isTrayExtended() {
    return digitalRead(MOTOR_POS_SENSOR_A_PIN) == LOW && digitalRead(MOTOR_POS_SENSOR_B_PIN) == HIGH;
}

bool isTrayRetracted() {
    return digitalRead(MOTOR_POS_SENSOR_A_PIN) == HIGH && digitalRead(MOTOR_POS_SENSOR_B_PIN) == LOW;
}

void extendTray() {
    Serial.print("Extending...");
    while (!isTrayExtended()) {
        motor.run(FORWARD);
    }
    motor.run(RELEASE);
    Serial.println("DONE");
}

void retractTray() {
    Serial.print("Retracting...");
    while (!isTrayRetracted()) {
        motor.run(BACKWARD);
    }
    motor.run(RELEASE);
    Serial.println("DONE");
}

void handleManualToggle() {
    int manual_press = digitalRead(MOTOR_MANUAL_TOGGLE_PIN);
    if (manual_press == LOW) {
        if (isTrayExtended()) {
            retractTray();
            Serial.println("MODE: Manual. Ignoring RFID.");
            mode = MODE_MANUAL;
        } else if (isTrayRetracted()) {
            extendTray();
            Serial.println("MODE: Auto. Ready to read RFID.");
            mode = MODE_AUTO;
        }
    }   
}

bool isAuthorized() {
    if (rdm6300.update()) {
        uint32_t tagId = rdm6300.get_tag_id();
        Serial.println(tagId);
        if (tagId == AUTHORIZED_TAG_ID && rdm6300.is_tag_near()) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool isPresence() {
    return sonar.ping_cm() != 0;
}

void setup() {

    Serial.begin(9600);

    pinMode(MOTOR_POS_SENSOR_A_PIN, INPUT_PULLUP);
    pinMode(MOTOR_POS_SENSOR_B_PIN, INPUT_PULLUP);
    pinMode(MOTOR_MANUAL_TOGGLE_PIN, INPUT_PULLUP);

    rdm6300.begin(RFID_RX_PIN);
    
    motor.setSpeed(255);
    motor.run(RELEASE);

    extendTray();
    
    Serial.println("Init done.");
}

void loop() {
    handleManualToggle();
    if (mode == MODE_MANUAL) {
        // Noop
    } else if (mode == MODE_AUTO) {
        if (isAuthorized() && isTrayExtended()) {
            if (isEating) {
                Serial.println("Alredy eating");
            } else {
                Serial.println("Is authorized, opening.");
                isEating = true;
                retractTray();
            }
        } else {
            if (isTrayRetracted()) {
                
                Serial.println("The tray is open. Attempting to close");
                
                if (!isPresence()) {
                    
                    startMillis = millis();
                    Serial.println("starting timer...");
                    
                    while(millis()-startMillis < PRESENCE_TIMEOUT) {
                        
                        rdm6300.update(); // Force flushing the rfid buffer.

                        if (millis() % 1000 == 0 ) {
                            Serial.print(".");
                        }
                          
                        if (isPresence()) {
                            Serial.print("X");
                            delay(1000);
                            startMillis = millis();
                        }  
                        
                    }

                    if (!isPresence()) {
                        Serial.println("Timer ran out.");
                        extendTray();
                        isEating = false;
                    }      
                }
            }
        }
    }
    delay(10);
}
