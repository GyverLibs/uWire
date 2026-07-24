#include <Arduino.h>
#include <uWire.h>

void setup() {
    Serial.begin(115200);
    uWire.begin();
}

void loop() {
    Serial.println("Begin scan");

    for (uint8_t addr = 1; addr <= 127; addr++) {
        uWire.beginTransmission(addr);
        if (!uWire.endTransmission()) {
            Serial.print("0x");
            Serial.println(addr, HEX);
        }
    }

    Serial.println("End scan");
    Serial.println();
    delay(1000);
}