/*
https://github.com/Qudor-Engineer/DMD32
https://github.com/3tawi/GPxMatrix
https://github.com/pixelmatix/SmartMatrix
https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-DMA
https://github.com/2dom/PxMatrix
https://github.com/busel7/DMDESP

https://lastminuteengineers.com/nrf24l01-arduino-wireless-communication/
https://randomnerdtutorials.com/esp32-spi-communication-arduino/

https://github.com/greiman/DigitalIO
*/


#pragma once


#include <Arduino.h>
#include <esp32-hal-ledc.h>
#include <DigitalIO.h>
#include <RF24.h>
#include "DMD32.h"
#include "Droid_Sans_24.h"

#define NRF_CSN_PIN 27
#define HSPI_MISO_PIN 12
#define HSPI_MOSI_PIN 13
#define HSPI_SCLK_PIN 14
#define HSPI_CE_PIN 26
#define LAP_COUNTER_INTERRUPT_PIN 4

#define DEBUG
#define SERIAL_DEBUG_BAUDRATE 115200
#define RADIO_COMMAND_LENGTH 3
#define NRF_CHANNEL 108
#define LED_MATRIX_ROWS 3
#define LED_MATRIX_COLS 4
#define POSX_LAPS 6
#define POSX_VBAR 45
#define DISPLAY_BRIGHTNESS 180 // 0 a 255
#define DISPLAY_REFRESH 2500
#define LAPS_DEBOUNCE_TIME 50
#define STOPWATCH_MINUTES_LIMIT 15
#define STOPWATCH_AUTOSTART true
#define START_SIGNAL_DELAY 200

#define CHAR_WIDTH 13
#define POINT_WIDTH 5

#define LAPS_CHAR_1 5
#define LAPS_CHAR_2 LAPS_CHAR_1 + CHAR_WIDTH
#define LAPS_CHAR_3 LAPS_CHAR_2 + CHAR_WIDTH

#define SWATCH_CHAR_1 50
#define SWATCH_CHAR_2 SWATCH_CHAR_1 + CHAR_WIDTH
#define SWATCH_CHAR_3 SWATCH_CHAR_2 + CHAR_WIDTH
#define SWATCH_CHAR_4 SWATCH_CHAR_3 + POINT_WIDTH
#define SWATCH_CHAR_5 SWATCH_CHAR_4 + CHAR_WIDTH
#define SWATCH_CHAR_6 SWATCH_CHAR_5 + CHAR_WIDTH
#define SWATCH_CHAR_7 SWATCH_CHAR_6 + POINT_WIDTH

DMD dmd(LED_MATRIX_COLS, LED_MATRIX_ROWS);
RF24 radio(HSPI_CE_PIN, NRF_CSN_PIN);
hw_timer_t *display_timer = NULL;
hw_timer_t *warmup_timer = NULL;
hw_timer_t *stopwatch_timer = NULL;
SPIClass *vspi = NULL;
SPIClass *hspi = NULL;

volatile bool update_display_stopwatch = false;
volatile uint8_t mm = 0, ss = 0, ts = 0;

const byte RADIO_ADDRESS[6] = "00003";
const char COMMANDS_ARRAY[7][4] = {"YFP", "YFM", "SRS", "RRS", "SES", "100", "200"};
struct Payload { char id[6]; char data[RADIO_COMMAND_LENGTH + 1] = "000"; };
Payload payload;

struct Button { const uint8_t pin; uint32_t debounceTimer; bool lastSteadyState; bool lastFlickerableState; bool currentState; };
Button laps_button = { LAP_COUNTER_INTERRUPT_PIN, 0, LOW, LOW, LOW };

int laps_limit = 100, laps_counter = 0, faults_counter = 0;
int prevLapsUnit = 0, prevLapsTen = 0, prevLapsHundred = 0;
int prevStopMinUnit = 0, prevStopMinTen = 0, prevStopSecUnit = 0, prevStopSecTen = 0, prevStopDec = 0;

bool race_started = false;
bool warmup_started = false;
bool last30_started = false;
bool update_display_laps = false;
bool start_signal_just_received = false;

uint32_t start_delay_timer = 0;
