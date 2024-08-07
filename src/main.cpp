#include "config.h"
#include "main.h"


void setup() {
    #ifdef DEBUG
    Serial.begin(SERIAL_DEBUG_BAUDRATE);
    #endif

    delay(3000);

    init_pins();
    init_interrupts();
    init_display();
    init_radio();

    #ifdef DEBUG
    Serial.println("SISTEMA INICIADO");
    #endif
}


void loop() {
    loop_radio();
    loop_display();
    loop_laps_button();
}
