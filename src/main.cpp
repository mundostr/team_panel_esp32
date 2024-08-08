#include "config.h"
#include "main.h"


void setup() {
    #ifdef DEBUG
    Serial.begin(SERIAL_DEBUG_BAUDRATE);
    #endif

    init_pins();
    init_display();
    init_radio();
    init_interrupts();

    delay(1000);

    #ifdef DEBUG
    Serial.println("SISTEMA INICIADO");
    #endif
}


void loop() {
    if (millis() - timer_loop >= 5) {
        loop_display();
        loop_radio();
        loop_laps_button();

        timer_loop = millis();
    }
}
