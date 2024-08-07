#ifndef DMD_H_
#define DMD_H_

#include "Arduino.h"
#include <SPI.h>

#define PIN_DMD_nOE         22 // D22 active low Output Enable, setting this low lights all the LEDs in the selected rows. Can pwm it at very high frequency for brightness control.
#define PIN_DMD_A           0 // D19
#define PIN_DMD_B		    21 // D21
#define PIN_DMD_CLK		    18 // D18_SCK  is SPI Clock if SPI is used
#define PIN_DMD_SCLK		2 // D02 Latch
#define PIN_DMD_R_DATA      23 // D23_MOSI is SPI Master Out if SPI is used
//Define this chip select pin that the Ethernet W5100 IC or other SPI device uses
//if it is in use during a DMD scan request then scanDisplayBySPI() will exit without conflict! (and skip that scan)
#define PIN_OTHER_SPI_nCS SS

#define LIGHT_DMD_ROW_01_05_09_13() { digitalWrite(PIN_DMD_B, LOW); digitalWrite(PIN_DMD_A, LOW); }
#define LIGHT_DMD_ROW_02_06_10_14() { digitalWrite(PIN_DMD_B, LOW); digitalWrite(PIN_DMD_A, HIGH); }
#define LIGHT_DMD_ROW_03_07_11_15() { digitalWrite(PIN_DMD_B, HIGH); digitalWrite(PIN_DMD_A, LOW); }
#define LIGHT_DMD_ROW_04_08_12_16() { digitalWrite(PIN_DMD_B, HIGH); digitalWrite(PIN_DMD_A, HIGH); }
#define LATCH_DMD_SHIFT_REG_TO_OUTPUT()	{ digitalWrite(PIN_DMD_SCLK, HIGH); digitalWrite(PIN_DMD_SCLK,LOW); }
#define OE_DMD_ROWS_OFF() { digitalWrite(PIN_DMD_nOE, LOW); }
#define OE_DMD_ROWS_ON() { digitalWrite(PIN_DMD_nOE, HIGH); }

#define GRAPHICS_NORMAL	    0
#define GRAPHICS_INVERSE	1
#define GRAPHICS_TOGGLE	    2
#define GRAPHICS_OR		    3
#define GRAPHICS_NOR	    4

#define PATTERN_ALT_0	    0
#define PATTERN_ALT_1	    1
#define PATTERN_STRIPE_0	2
#define PATTERN_STRIPE_1	3

#define DMD_PIXELS_ACROSS	32 // pixels across x axis (base 2 size expected)
#define DMD_PIXELS_DOWN	    16 // pixels down y axis
#define DMD_BITSPERPIXEL	1 // 1 bit per pixel, use more bits to allow for pwm screen brightness control
#define DMD_RAM_SIZE_BYTES ((DMD_PIXELS_ACROSS * DMD_BITSPERPIXEL / 8) * DMD_PIXELS_DOWN)

//lookup table for DMD::writePixel to make the pixel indexing routine faster
static byte bPixelLookupTable[8] = {
    0x80,   //0, bit 7
    0x40,   //1, bit 6
    0x20,   //2. bit 5
    0x10,   //3, bit 4
    0x08,   //4, bit 3
    0x04,   //5, bit 2
    0x02,   //6, bit 1
    0x01    //7, bit 0
};

#define FONT_LENGTH			0
#define FONT_FIXED_WIDTH	2
#define FONT_HEIGHT			3
#define FONT_FIRST_CHAR		4
#define FONT_CHAR_COUNT 	5
#define FONT_WIDTH_TABLE	6

typedef uint8_t (*FontCallback)(const uint8_t*);


class DMD {
    public:
    DMD(byte panelsWide, byte panelsHigh);
    
    // Set or clear a pixel at the x and y location (0, 0 is the TOP LEFT corner)
    void writePixel( unsigned int bX, unsigned int bY, byte bGraphicsMode, byte bPixel );
    
    // Draw a string
    void drawString( int bX, int bY, const char* bChars, byte length, byte bGraphicsMode);
    
    // Select a text font
    void selectFont(const uint8_t* font);
    
    // Draw a single character
    int drawChar(const int bX, const int bY, const unsigned char letter, byte bGraphicsMode);
    
    // Find the width of a character
    int charWidth(const unsigned char letter);
    
    // Clear the screen in DMD RAM
    void clearScreen( byte bNormal );
    
    // Draw or clear a line from x1,y1 to x2,y2
    void drawLine( int x1, int y1, int x2, int y2, byte bGraphicsMode );
    
    // Draw or clear a box(rectangle) with a single pixel border
    void drawBox( int x1, int y1, int x2, int y2, byte bGraphicsMode );
    
    // Draw or clear a filled box(rectangle) with a single pixel border
    void drawFilledBox( int x1, int y1, int x2, int y2, byte bGraphicsMode );
    
    // Scan the dot matrix LED panel display, from the RAM mirror out to the display hardware.
    // Call 4 times to scan the whole display which is made up of 4 interleaved rows within the 16 total rows.
    // Insert the calls to this function into the main loop for the highest call rate, or from a timer interrupt
    void scanDisplayBySPI();

    void setBuffer(uint8_t* newBuffer);
    
    private:
    // Mirror of DMD pixels in RAM, ready to be clocked out by the main loop or high speed timer calls
    byte *bDMDScreenRAM;
    char marqueeText[256];
    byte marqueeLength;
    int marqueeWidth;
    int marqueeHeight;
    int marqueeOffsetX;
    int marqueeOffsetY;

    // Pointer to current font
    const uint8_t* Font;
    
    byte DisplaysWide;
    byte DisplaysHigh;
    byte DisplaysTotal;
    int row1, row2, row3;

    // scanning pointer into bDMDScreenRAM, setup init @48 for the first valid scan
    volatile byte bDMDByte;
    
	SPIClass * vspi = NULL;
    static const int spiClk = 12 * 1000000; // MHz SPI clock
};

#endif
