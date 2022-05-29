#include <Wire.h>
#include "nunchuk.h"

void setup() {

    Serial.begin(9600);
    Wire.begin();
    // nunchuk_init_power(); // A1 and A2 is power supply
    nunchuk_init();
}

void loop() {

    if (nunchuk_read()) {
        // Work with nunchuk_data
        nunchuk_print();
    }
    delay(10);
}
