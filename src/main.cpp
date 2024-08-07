#include "config.h"
#include "main.h"

void setup() {
    #ifdef DEBUG
    Serial.begin(SERIAL_DEBUG_BAUDRATE);
    #endif

    init_interrupts();
    init_display();
    init_radio();

    delay(500);
}

void loop() {
    loop_display();
    
    if (millis() - loop_timer >= 10) {
        loop_radio();
        loop_laps_button();

        loop_timer = millis();
    }
}
