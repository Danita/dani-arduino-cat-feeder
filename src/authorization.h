#pragma once

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Rfid134.h>

uint32_t detectedTag = 0;

class RfidNotify {
    public:

        static void OnError(Rfid134_Error errorCode) {
            Serial.println();
            Serial.print("Com Error ");
            Serial.println(errorCode);
        }

        static void OnPacketRead(const Rfid134Reading& reading) {
            char temp[8];

            Serial.print("TAG: ");
            
            // since print doesn't support leading zero's, use sprintf
            // since sprintf with AVR doesn't support uint64_t (llu/lli), use /% trick to
            // break it up into equal sized leading zero pieces
            sprintf(temp, "%06lu", static_cast<uint32_t>(reading.id / 1000000));
            Serial.print(temp);
            Serial.print('-'); 
            sprintf(temp, "%06lu", static_cast<uint32_t>(reading.id % 1000000));
            Serial.print(temp); 
            Serial.println();
            detectedTag = static_cast<uint32_t>(reading.id % 1000000);
        }

};

class Authorization {
        SoftwareSerial secondarySerial;
        Rfid134<SoftwareSerial, RfidNotify> rfid;
        
    public:
        
        Authorization(uint8_t rxPin, uint8_t txPin):
            secondarySerial(rxPin, txPin),
            rfid(secondarySerial)
        {}

        bool isAuthorized() {
            if (detectedTag != 0) {
                detectedTag = 0;
                return true;
            } else {
                return false;
            }
        }

        void flush() {
            detectedTag = 0;
        }
        
        void setup() {
            secondarySerial.begin(9600);
            rfid.begin();
            Serial.println("[Auth] is ready.");
        }

        void loop() {
            rfid.loop();
        }

};