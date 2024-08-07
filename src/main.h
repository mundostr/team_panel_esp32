#pragma once


#include "config.h"


void IRAM_ATTR updateDisplay() {
    dmd.scanDisplayBySPI();
}

void IRAM_ATTR updateWarmup() {
    ss--;

    update_display_stopwatch = true;
}

void IRAM_ATTR updateStopwatch() {
    ts++;

    if (ts == 10) {
        ss++;
        ts = 0;
        if (ss > 59) { ss = 0; mm++; }
    }
    
    update_display_stopwatch = true;
}

void update_fault_lights() {
    dmd.drawFilledBox(0, 33, 127, 47, GRAPHICS_INVERSE);

    switch (faults_counter) {
        case 0: {
            break;
        }

        case 1: {
            dmd.drawFilledBox(1, 34, 30, 46, GRAPHICS_NORMAL);
            break;
        }

        case 2: {
            dmd.drawFilledBox(1, 34, 30, 46, GRAPHICS_NORMAL);
            dmd.drawFilledBox(33, 34, 62, 46, GRAPHICS_NORMAL);
            break;
        }

        case 3: {
            dmd.drawFilledBox(1, 34, 30, 46, GRAPHICS_NORMAL);
            dmd.drawFilledBox(33, 34, 62, 46, GRAPHICS_NORMAL);
            dmd.drawFilledBox(65, 34, 94, 46, GRAPHICS_NORMAL);
            break;
        }

        case 4: {
            dmd.drawFilledBox(1, 34, 30, 46, GRAPHICS_NORMAL);
            dmd.drawFilledBox(33, 34, 62, 46, GRAPHICS_NORMAL);
            dmd.drawFilledBox(65, 34, 94, 46, GRAPHICS_NORMAL);
            dmd.drawFilledBox(97, 34, 126, 46, GRAPHICS_NORMAL);
            break;
        }

        default: {}
    }
}


void set_display_static() {
    dmd.clearScreen(true);
    dmd.selectFont(Droid_Sans_24);

    dmd.drawString(LAPS_CHAR_1, 5, "0", 1, GRAPHICS_NORMAL);
    dmd.drawString(LAPS_CHAR_2, 5, "0", 1, GRAPHICS_NORMAL);
    dmd.drawString(LAPS_CHAR_3, 5, "0", 1, GRAPHICS_NORMAL);
    
    dmd.drawString(SWATCH_CHAR_1, 5, "0", 1, GRAPHICS_NORMAL);
    dmd.drawString(SWATCH_CHAR_2, 5, "0", 1, GRAPHICS_NORMAL);
    dmd.drawString(SWATCH_CHAR_3, 5, ":", 1, GRAPHICS_NORMAL);
    dmd.drawString(SWATCH_CHAR_4, 5, "0", 1, GRAPHICS_NORMAL);
    dmd.drawString(SWATCH_CHAR_5, 5, "0", 1, GRAPHICS_NORMAL);
    dmd.drawString(SWATCH_CHAR_6, 5, ".", 1, GRAPHICS_NORMAL);
    dmd.drawString(SWATCH_CHAR_7, 5, "0", 1, GRAPHICS_NORMAL);

    dmd.drawBox(0, 0, 127, 31, GRAPHICS_NORMAL);
    dmd.drawBox(1, 1, 126, 30, GRAPHICS_NORMAL);
    
    dmd.drawLine(POSX_VBAR, 0, POSX_VBAR, 31, GRAPHICS_NORMAL);
    dmd.drawLine(POSX_VBAR + 1, 0, POSX_VBAR + 1, 31, GRAPHICS_NORMAL);
}

void init_pins() {
    pinMode(laps_button.pin, INPUT);
    digitalWrite(laps_button.pin, 0);
}

void init_radio() {
    // hspi = new SPIClass(HSPI);
    // hspi->begin(HSPI_SCLK_PIN, HSPI_MISO_PIN, HSPI_MOSI_PIN, HSPI_CE_PIN); //SCLK = 14, MISO = 12, MOSI = 13, SS = 15
    // pinMode(HSPI_CE_PIN, OUTPUT);

    if (!radio.begin()) {
        #ifdef DEBUG
        Serial.println(F("NRF24: ERROR"));
        #endif
        for(;;);
    }

    radio.setPALevel(RF24_PA_MAX);   //(RF24_PA_MIN|RF24_PA_LOW|RF24_PA_HIGH|RF24_PA_MAX)
    radio.setDataRate(RF24_250KBPS); //(RF24_250KBPS|RF24_1MBPS|RF24_2MBPS)
    radio.setChannel(NRF_CHANNEL);
    radio.setPayloadSize(sizeof(payload));
    radio.openReadingPipe(0, RADIO_ADDRESS);
    radio.startListening(); // Modo RX / RX Mode
    
    #ifdef DEBUG
    Serial.println(F("NRF24: OK modo RX / RX mode OK"));
    #endif
}

void init_interrupts() {
    display_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(display_timer, &updateDisplay, true);
    timerAlarmWrite(display_timer, DISPLAY_REFRESH, true);
    timerAlarmEnable(display_timer);
    
    warmup_timer = timerBegin(1, 80, true);
    timerAttachInterrupt(warmup_timer, &updateWarmup, true);
    timerAlarmWrite(warmup_timer, 1000000, true);

    stopwatch_timer = timerBegin(2, 80, true);
    timerAttachInterrupt(stopwatch_timer, &updateStopwatch, true);
    timerAlarmWrite(stopwatch_timer, 100000, true);
}

void init_display() {
    vspi = new SPIClass(VSPI);
    vspi->begin();
    ledcSetup(0, DISPLAY_REFRESH, 8);
    ledcAttachPin(PIN_DMD_nOE, 0);
    ledcWrite(0, DISPLAY_BRIGHTNESS);
    set_display_static();
}


void start_race() {
    race_started = true;
    warmup_started = false;
    last30_started = false;
    update_display_laps = true;
    update_display_stopwatch = true;
    laps_counter = 0, mm = 0, ss = 0, ts = 0;
    prevLapsUnit = 0, prevLapsTen = 0, prevLapsHundred = 0;
    prevStopMinUnit = 0, prevStopMinTen = 0, prevStopSecUnit = 0, prevStopSecTen = 0, prevStopDec = 0;
    faults_counter = 0;

    dmd.drawFilledBox(47, 5, 125, 29, GRAPHICS_INVERSE);
    dmd.drawFilledBox(0, 33, 127, 47, GRAPHICS_INVERSE);

    int offset = -6;
    dmd.drawString(SWATCH_CHAR_2 + offset, 5, "0", 1, GRAPHICS_NORMAL);
    dmd.drawString(SWATCH_CHAR_3 + offset, 5, ":", 1, GRAPHICS_NORMAL);
    dmd.drawString(SWATCH_CHAR_4 + offset, 5, "0", 1, GRAPHICS_NORMAL);
    dmd.drawString(SWATCH_CHAR_5 + offset, 5, "0", 1, GRAPHICS_NORMAL);
    dmd.drawString(SWATCH_CHAR_6 + offset, 5, ".", 1, GRAPHICS_NORMAL);
    dmd.drawString(SWATCH_CHAR_7 + offset, 5, "0", 1, GRAPHICS_NORMAL);

    if (!timerAlarmEnabled(stopwatch_timer)) timerAlarmEnable(stopwatch_timer);
}

void verify_payload_data(char *data) {
    unsigned int command = 99;

    for (unsigned int c = 0; c < 7; c++) if (strcmp(data, COMMANDS_ARRAY[c]) == 0) { command = c; }

    #ifdef DEBUG
    Serial.println(COMMANDS_ARRAY[command]);
    #endif

    switch (command) {
        // RFP GFP YFP (Fault plus)
        case 0: {
            faults_counter < 4 ? faults_counter++ : faults_counter = 4;
            update_fault_lights();
            break;
        }
        
        // RFM GFM YFM (Fault minus)
        case 1: {
            faults_counter > 0 ? faults_counter-- : faults_counter = 0;
            update_fault_lights();
            break;
        }

        // SRS (Start race signal)
        case 2: {
            if (!warmup_started && ! last30_started && !race_started) {
                start_signal_just_received = true;
                start_delay_timer = millis();
            }
            break;
        }

        // RRS (Reset race signal)
        case 3: {
            warmup_started = false;
            last30_started = false;
            race_started = false;
            update_display_laps = true;
            update_display_stopwatch = true;
            laps_counter = 0, mm = 0, ss = 0, ts = 0;
            prevLapsUnit = 0, prevLapsTen = 0, prevLapsHundred = 0;
            prevStopMinUnit = 0, prevStopMinTen = 0, prevStopSecUnit = 0, prevStopSecTen = 0, prevStopDec = 0;
            faults_counter = 0;
            if (timerAlarmEnabled(warmup_timer)) timerAlarmDisable(warmup_timer);
            if (timerAlarmEnabled(stopwatch_timer)) timerAlarmDisable(stopwatch_timer);
            set_display_static();
            break;
        }

        // SES (Start engines signal)
        case 4: {
            if (!warmup_started && ! last30_started && !race_started) {
                warmup_started = true;
                update_display_laps = true;
                update_display_stopwatch = true;
                
                dmd.drawFilledBox(47, 5, 125, 29, GRAPHICS_INVERSE);

                dmd.drawString(SWATCH_CHAR_2, 5, "1", 1, GRAPHICS_NORMAL);
                dmd.drawString(SWATCH_CHAR_3, 5, ":", 1, GRAPHICS_NORMAL);
                dmd.drawString(SWATCH_CHAR_4, 5, "3", 1, GRAPHICS_NORMAL);
                dmd.drawString(SWATCH_CHAR_5, 5, "0", 1, GRAPHICS_NORMAL);

                laps_counter = 0, mm = 0, ss = 91, ts = 0;
                if (!timerAlarmEnabled(warmup_timer)) timerAlarmEnable(warmup_timer);
            }
            break;
        }

        // 100
        case 5: {
            laps_limit = 100;
            break;
        }

        // 200
        case 6: {
            laps_limit = 200;
            break;
        }
        
        default: {}
    }
}


void draw_laps() {
    static char display_laps_buffer[4];

    snprintf(display_laps_buffer, 4, "%03d", laps_counter);
        
    int hundred = laps_counter / 100;
    int ten = (laps_counter / 10) % 10;
    int unit = laps_counter % 10;

    if (hundred != prevLapsHundred) {
        dmd.drawFilledBox(3, 5, LAPS_CHAR_2 - 1, 29, GRAPHICS_INVERSE);
        dmd.drawString(LAPS_CHAR_1, 5, &display_laps_buffer[0], 1, GRAPHICS_NORMAL);
        prevLapsHundred = hundred;
    }

    if (ten != prevLapsTen) {
        dmd.drawFilledBox(LAPS_CHAR_2, 5, LAPS_CHAR_3 - 1, 29, GRAPHICS_INVERSE);
        dmd.drawString(LAPS_CHAR_2, 5, &display_laps_buffer[1], 1, GRAPHICS_NORMAL);
        prevLapsTen = ten;
    }

    if (unit != prevLapsUnit) {
        dmd.drawFilledBox(LAPS_CHAR_3, 5, POSX_VBAR - 1, 29, GRAPHICS_INVERSE);
        dmd.drawString(LAPS_CHAR_3, 5, &display_laps_buffer[2], 1, GRAPHICS_NORMAL);
        prevLapsUnit = unit;
    }

    dmd.drawBox(1, 1, 126, 30, GRAPHICS_NORMAL);
}

void draw_stopwatch_warmup() {
    static char display_stopwatch_buffer[8];

    int ten = (ss / 10) % 10;
    int unit = ss % 10;

    if (ss >= 60) {
        snprintf(display_stopwatch_buffer, 8, "01:%02d.0", ss - 60);
    } else {
        snprintf(display_stopwatch_buffer, 8, "00:%02d.0", ss);
            
        if (ss == 59) {
            dmd.drawFilledBox(SWATCH_CHAR_2, 5, SWATCH_CHAR_3 - 1, 29, GRAPHICS_INVERSE);
            dmd.drawString(SWATCH_CHAR_2, 5, &display_stopwatch_buffer[1], 1, GRAPHICS_NORMAL);
        }

        if (ss == 0) {
            dmd.drawString(SWATCH_CHAR_4, 5, "3", 1, GRAPHICS_NORMAL);
            dmd.drawString(SWATCH_CHAR_5, 5, "0", 1, GRAPHICS_NORMAL);

            warmup_started = false;
            last30_started = true;
            prevStopSecUnit = 0;
            prevStopSecTen = 0;
            ss = 30;
        }
    }

    if (ten != prevStopSecTen) {
        dmd.drawFilledBox(SWATCH_CHAR_4, 5, SWATCH_CHAR_5 - 1, 29, GRAPHICS_INVERSE);
        dmd.drawString(SWATCH_CHAR_4, 5, &display_stopwatch_buffer[3], 1, GRAPHICS_NORMAL);
        prevStopSecTen = ten;
    }

    if (unit != prevStopSecUnit) {
        dmd.drawFilledBox(SWATCH_CHAR_5, 5, SWATCH_CHAR_6 - 1, 29, GRAPHICS_INVERSE);
        dmd.drawString(SWATCH_CHAR_5, 5, &display_stopwatch_buffer[4], 1, GRAPHICS_NORMAL);
        prevStopSecUnit = unit;
    }
}

void draw_stopwatch_last30() {
    static char display_stopwatch_buffer[8];

    int ten = (ss / 10) % 10;
    int unit = ss % 10;

    snprintf(display_stopwatch_buffer, 8, "00:%02d.0", ss);

    if (ten != prevStopSecTen) {
        dmd.drawFilledBox(SWATCH_CHAR_4, 5, SWATCH_CHAR_5 - 1, 29, GRAPHICS_INVERSE);
        dmd.drawString(SWATCH_CHAR_4, 5, &display_stopwatch_buffer[3], 1, GRAPHICS_NORMAL);
        prevStopSecTen = ten;
    }

    if (unit != prevStopSecUnit) {
        dmd.drawFilledBox(SWATCH_CHAR_5, 5, SWATCH_CHAR_6 - 1, 29, GRAPHICS_INVERSE);
        dmd.drawString(SWATCH_CHAR_5, 5, &display_stopwatch_buffer[4], 1, GRAPHICS_NORMAL);
        prevStopSecUnit = unit;
    }

    if (ss == 0) {
        if (timerAlarmEnabled(warmup_timer)) timerAlarmDisable(warmup_timer);
        if (STOPWATCH_AUTOSTART) start_race();
    }
}

void draw_stopwatch_race() {
    static char display_stopwatch_buffer[8];

    int offset = mm >= 10 ? 0 : -6;
    int minTen = (mm / 10) % 10;
    int minUnit = mm % 10;
    int secTen = (ss / 10) % 10;
    int secUnit = ss % 10;

    snprintf(display_stopwatch_buffer, 8, "%02d:%02d.%d", mm, ss, ts);

    if (mm >= 10 && minTen != prevStopMinTen) {
        if (mm == 10) {
            dmd.drawFilledBox(SWATCH_CHAR_1, 5, 125, 29, GRAPHICS_INVERSE);
            dmd.drawString(SWATCH_CHAR_3 + offset, 5, &display_stopwatch_buffer[2], 1, GRAPHICS_NORMAL);
            dmd.drawString(SWATCH_CHAR_6 + offset, 5, &display_stopwatch_buffer[5], 1, GRAPHICS_NORMAL);
        }

        dmd.drawFilledBox(SWATCH_CHAR_1 + offset, 5, SWATCH_CHAR_2 + offset - 1, 29, GRAPHICS_INVERSE);
        dmd.drawString(SWATCH_CHAR_1, 5, &display_stopwatch_buffer[0], 1, GRAPHICS_NORMAL);
        prevStopMinTen = minTen;
    }

    if (minUnit != prevStopMinUnit) {
        dmd.drawFilledBox(SWATCH_CHAR_2 + offset, 5, SWATCH_CHAR_3 + offset - 1, 29, GRAPHICS_INVERSE);
        dmd.drawString(SWATCH_CHAR_2 + offset, 5, &display_stopwatch_buffer[1], 1, GRAPHICS_NORMAL);
        prevStopMinUnit = minUnit;
    }

    if (secTen != prevStopSecTen) {
        dmd.drawFilledBox(SWATCH_CHAR_4 + offset, 5, SWATCH_CHAR_5 + offset - 1, 29, GRAPHICS_INVERSE);
        dmd.drawString(SWATCH_CHAR_4 + offset, 5, &display_stopwatch_buffer[3], 1, GRAPHICS_NORMAL);
        prevStopSecTen = secTen;
    }
    
    if (secUnit != prevStopSecUnit) {
        dmd.drawFilledBox(SWATCH_CHAR_5 + offset, 5, SWATCH_CHAR_6 + offset - 1, 29, GRAPHICS_INVERSE);
        dmd.drawString(SWATCH_CHAR_5 + offset, 5, &display_stopwatch_buffer[4], 1, GRAPHICS_NORMAL);
        prevStopSecUnit = secUnit;
    }

    if (ts != prevStopDec) {
        dmd.drawFilledBox(SWATCH_CHAR_7 + offset, 5, 125, 29, GRAPHICS_INVERSE);
        dmd.drawString(SWATCH_CHAR_7 + offset, 5, &display_stopwatch_buffer[6], 1, GRAPHICS_NORMAL);
        prevStopDec = ts;
    }
}

void loop_display() {
    if (update_display_laps) {
        draw_laps();
        update_display_laps = false;
    }

    if (update_display_stopwatch) {
        if (race_started) {
            draw_stopwatch_race();
            
            if (mm == STOPWATCH_MINUTES_LIMIT) {
                race_started = false;
                if (timerAlarmEnabled(stopwatch_timer)) timerAlarmDisable(stopwatch_timer);
                set_display_static();
            }
        } else if (last30_started) {
            draw_stopwatch_last30();
        } else if (warmup_started) {
            draw_stopwatch_warmup();
        }

        dmd.drawBox(1, 1, 126, 30, GRAPHICS_NORMAL);

        update_display_stopwatch = false;
    }
}

void increment_laps() {
    if (laps_counter < laps_limit) {
        laps_counter++;
        
        update_display_laps = true;
        
        if (laps_counter == laps_limit) {
            race_started = false;
            if (timerAlarmEnabled(stopwatch_timer)) timerAlarmDisable(stopwatch_timer);
        }
    }
}

void loop_laps_button() {
    if (race_started) {
        laps_button.currentState = digitalRead(laps_button.pin);
        
        if (laps_button.currentState != laps_button.lastFlickerableState) {
            laps_button.debounceTimer = millis();
            laps_button.lastFlickerableState = laps_button.currentState;
        }

        if (millis() - laps_button.debounceTimer >= LAPS_DEBOUNCE_TIME) {
            if (laps_button.lastSteadyState == LOW && laps_button.currentState == HIGH) increment_laps();
            laps_button.lastSteadyState = laps_button.currentState;
        }
    }
}

void loop_radio() {
    if (radio.available() > 0) {
        radio.read(&payload, sizeof(payload));
        verify_payload_data(payload.data);
    }

    if (start_signal_just_received && millis() - start_delay_timer >= START_SIGNAL_DELAY) {
        start_signal_just_received = false;
        start_delay_timer = millis();
        start_race();
    }
        
    // static char outputBuf[50];
    // snprintf(outputBuf, sizeof(outputBuf), "ID: %s, comando: %s", payload.id, payload.data);
    // Serial.println(outputBuf);
}