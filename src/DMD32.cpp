#include "dmd32.h"

DMD::DMD(byte panelsWide, byte panelsHigh) {
    uint16_t ui;
    DisplaysWide = panelsWide;
    DisplaysHigh = panelsHigh;
    DisplaysTotal = DisplaysWide * DisplaysHigh;
    row1 = DisplaysTotal << 4;
    row2 = DisplaysTotal << 5;
    row3 = ((DisplaysTotal << 2) * 3) << 2;
    bDMDScreenRAM = (byte *)malloc(DisplaysTotal * DMD_RAM_SIZE_BYTES);

    vspi = new SPIClass(VSPI);
    vspi->begin();

    digitalWrite(PIN_DMD_A, LOW);
    digitalWrite(PIN_DMD_B, LOW);
    digitalWrite(PIN_DMD_CLK, LOW);
    digitalWrite(PIN_DMD_SCLK, LOW);
    digitalWrite(PIN_DMD_R_DATA, HIGH);
    digitalWrite(PIN_DMD_nOE, LOW);

    pinMode(PIN_DMD_A, OUTPUT);
    pinMode(PIN_DMD_B, OUTPUT);
    pinMode(PIN_DMD_CLK, OUTPUT);
    pinMode(PIN_DMD_SCLK, OUTPUT);
    pinMode(PIN_DMD_R_DATA, OUTPUT);
    pinMode(PIN_DMD_nOE, OUTPUT);

    clearScreen(true);

    // init the scan line/ram pointer to the required start point
    bDMDByte = 0;
}

void DMD::writePixel(unsigned int bX, unsigned int bY, byte bGraphicsMode, byte bPixel) {
    unsigned int uiDMDRAMPointer;

    if (bX >= (DMD_PIXELS_ACROSS * DisplaysWide) || bY >= (DMD_PIXELS_DOWN * DisplaysHigh))
        return;

    byte panel = (bX / DMD_PIXELS_ACROSS) + (DisplaysWide * (bY / DMD_PIXELS_DOWN));
    bX = (bX % DMD_PIXELS_ACROSS) + (panel << 5);
    bY = bY % DMD_PIXELS_DOWN;
    // set pointer to DMD RAM byte to be modified
    uiDMDRAMPointer = bX / 8 + bY * (DisplaysTotal << 2);

    byte lookup = bPixelLookupTable[bX & 0x07];

    switch (bGraphicsMode) {

    case GRAPHICS_NORMAL:
        if (bPixel == true)
            bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup; // zero bit is pixel on
        else
            bDMDScreenRAM[uiDMDRAMPointer] |= lookup; // one bit is pixel off
        break;

    case GRAPHICS_INVERSE:
        if (bPixel == false)
            bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup; // zero bit is pixel on
        else
            bDMDScreenRAM[uiDMDRAMPointer] |= lookup; // one bit is pixel off
        break;

    case GRAPHICS_TOGGLE:
        if (bPixel == true) {
            if ((bDMDScreenRAM[uiDMDRAMPointer] & lookup) == 0)
                bDMDScreenRAM[uiDMDRAMPointer] |= lookup; // one bit is pixel off
            else
                bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup; // one bit is pixel off
        }
        break;

    case GRAPHICS_OR:
        // only set pixels on
        if (bPixel == true)
            bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup; // zero bit is pixel on
        break;

    case GRAPHICS_NOR:
        // only clear on pixels
        if ((bPixel == true) &&
            ((bDMDScreenRAM[uiDMDRAMPointer] & lookup) == 0))
            bDMDScreenRAM[uiDMDRAMPointer] |= lookup; // one bit is pixel off
        break;
    }
}

void DMD::drawString(int bX, int bY, const char *bChars, byte length, byte bGraphicsMode) {
    if (bX >= (DMD_PIXELS_ACROSS * DisplaysWide) || bY >= DMD_PIXELS_DOWN * DisplaysHigh)
        return;

    uint8_t height = pgm_read_byte(this->Font + FONT_HEIGHT);
    if (bY + height < 0)
        return;

    int strWidth = 0;
    this->drawLine(bX - 1, bY, bX - 1, bY + height, GRAPHICS_INVERSE);

    for (int i = 0; i < length; i++) {
        int charWide = this->drawChar(bX + strWidth, bY, bChars[i], bGraphicsMode);

        if (charWide > 0) {
            strWidth += charWide;
            this->drawLine(bX + strWidth, bY, bX + strWidth, bY + height, GRAPHICS_INVERSE);
            strWidth++;
        } else if (charWide < 0) {
            return;
        }

        if ((bX + strWidth) >= DMD_PIXELS_ACROSS * DisplaysWide || bY >= DMD_PIXELS_DOWN * DisplaysHigh)
            return;
    }
}

void DMD::clearScreen(byte bNormal) {
    if (bNormal)
        memset(bDMDScreenRAM, 0xFF, DMD_RAM_SIZE_BYTES * DisplaysTotal);
    else
        memset(bDMDScreenRAM, 0x00, DMD_RAM_SIZE_BYTES * DisplaysTotal);
}

void DMD::drawLine(int x1, int y1, int x2, int y2, byte bGraphicsMode) {
    int dy = y2 - y1;
    int dx = x2 - x1;
    int stepx, stepy;

    if (dy < 0) {
        dy = -dy;
        stepy = -1;
    } else {
        stepy = 1;
    }

    if (dx < 0) {
        dx = -dx;
        stepx = -1;
    } else {
        stepx = 1;
    }

    dy <<= 1; // dy is now 2 * dy
    dx <<= 1; // dx is now 2 * dx

    writePixel(x1, y1, bGraphicsMode, true);

    if (dx > dy) {
        int fraction = dy - (dx >> 1); // same as 2 * dy - dx
        while (x1 != x2) {
            if (fraction >= 0) {
                y1 += stepy;
                fraction -= dx; // same as fraction -= 2 * dx
            }

            x1 += stepx;
            fraction += dy; // same as fraction -= 2 * dy
            writePixel(x1, y1, bGraphicsMode, true);
        }
    } else {
        int fraction = dx - (dy >> 1);

        while (y1 != y2) {
            if (fraction >= 0) {
                x1 += stepx;
                fraction -= dy;
            }

            y1 += stepy;
            fraction += dx;
            writePixel(x1, y1, bGraphicsMode, true);
        }
    }
}

void DMD::drawBox(int x1, int y1, int x2, int y2, byte bGraphicsMode) {
    drawLine(x1, y1, x2, y1, bGraphicsMode);
    drawLine(x2, y1, x2, y2, bGraphicsMode);
    drawLine(x2, y2, x1, y2, bGraphicsMode);
    drawLine(x1, y2, x1, y1, bGraphicsMode);
}

void DMD::drawFilledBox(int x1, int y1, int x2, int y2, byte bGraphicsMode) {
    for (int b = x1; b <= x2; b++) {
        drawLine(b, y1, b, y2, bGraphicsMode);
    }
}

void DMD::scanDisplayBySPI() {
    // if PIN_OTHER_SPI_nCS is in use during a DMD scan request then scanDisplayBySPI() will exit without conflict! (and skip that scan)
    if (digitalRead(PIN_OTHER_SPI_nCS) == HIGH) {
        // SPI transfer pixels to the display hardware shift registers
        int rowsize = DisplaysTotal << 2;
        int offset = rowsize * bDMDByte;
        
        for (int i = 0; i < rowsize; i++) {
            vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
            vspi->transfer(bDMDScreenRAM[offset + i + row3]);
            vspi->transfer(bDMDScreenRAM[offset + i + row2]);
            vspi->transfer(bDMDScreenRAM[offset + i + row1]);
            vspi->transfer(bDMDScreenRAM[offset + i]);
            vspi->endTransaction();
        }

        OE_DMD_ROWS_OFF();
        LATCH_DMD_SHIFT_REG_TO_OUTPUT();
        
        switch (bDMDByte) {
        case 0: // row 1, 5, 9, 13 were clocked out
            LIGHT_DMD_ROW_01_05_09_13();
            bDMDByte = 1;
            break;

        case 1: // row 2, 6, 10, 14 were clocked out
            LIGHT_DMD_ROW_02_06_10_14();
            bDMDByte = 2;
            break;

        case 2: // row 3, 7, 11, 15 were clocked out
            LIGHT_DMD_ROW_03_07_11_15();
            bDMDByte = 3;
            break;

        case 3: // row 4, 8, 12, 16 were clocked out
            LIGHT_DMD_ROW_04_08_12_16();
            bDMDByte = 0;
            break;
        }

        OE_DMD_ROWS_ON();
    }
}

void DMD::selectFont(const uint8_t *font) {
    this->Font = font;
}

int DMD::drawChar(const int bX, const int bY, const unsigned char letter, byte bGraphicsMode) {
    if (bX > (DMD_PIXELS_ACROSS * DisplaysWide) || bY > (DMD_PIXELS_DOWN * DisplaysHigh))
        return -1;

    unsigned char c = letter;
    uint8_t height = pgm_read_byte(this->Font + FONT_HEIGHT);

    if (c == ' ') {
        int charWide = charWidth(' ');
        this->drawFilledBox(bX, bY, bX + charWide, bY + height, GRAPHICS_INVERSE);
        return charWide;
    }

    uint8_t width = 0;
    uint8_t bytes = (height + 7) / 8;

    uint8_t firstChar = pgm_read_byte(this->Font + FONT_FIRST_CHAR);
    uint8_t charCount = pgm_read_byte(this->Font + FONT_CHAR_COUNT);

    uint16_t index = 0;

    if (c < firstChar || c >= (firstChar + charCount))
        return 0;
    c -= firstChar;

    if (pgm_read_byte(this->Font + FONT_LENGTH) == 0 && pgm_read_byte(this->Font + FONT_LENGTH + 1) == 0) {
        // zero length is flag indicating fixed width font (array does not contain width data entries)
        width = pgm_read_byte(this->Font + FONT_FIXED_WIDTH);
        index = c * bytes * width + FONT_WIDTH_TABLE;
    } else {
        // variable width font, read width data, to get the index
        for (uint8_t i = 0; i < c; i++) {
            index += pgm_read_byte(this->Font + FONT_WIDTH_TABLE + i);
        }

        index = index * bytes + charCount + FONT_WIDTH_TABLE;
        width = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c);
    }

    if (bX < -width || bY < -height)
        return width;

    // last but not least, draw the character
    for (uint8_t j = 0; j < width; j++) { // Width
        for (uint8_t i = bytes - 1; i < 254; i--) { // Vertical Bytes
            uint8_t data = pgm_read_byte(this->Font + index + j + (i * width));
            int offset = (i * 8);

            if ((i == bytes - 1) && bytes > 1) {
                offset = height - 8;
            }

            for (uint8_t k = 0; k < 8; k++) { // Vertical bits
                if ((offset + k >= i * 8) && (offset + k <= height)) {
                    if (data & (1 << k)) {
                        writePixel(bX + j, bY + offset + k, bGraphicsMode, true);
                    } else {
                        writePixel(bX + j, bY + offset + k, bGraphicsMode, false);
                    }
                }
            }
        }
    }
    
    return width;
}

int DMD::charWidth(const unsigned char letter) {
    unsigned char c = letter;
    if (c == ' ')
        c = 'n'; // Space is often not included in font so use width of 'n'
    uint8_t width = 0;

    uint8_t firstChar = pgm_read_byte(this->Font + FONT_FIRST_CHAR);
    uint8_t charCount = pgm_read_byte(this->Font + FONT_CHAR_COUNT);

    uint16_t index = 0;

    if (c < firstChar || c >= (firstChar + charCount))
        return 0;
    c -= firstChar;

    if (pgm_read_byte(this->Font + FONT_LENGTH) == 0 && pgm_read_byte(this->Font + FONT_LENGTH + 1) == 0) {
        // zero length is flag indicating fixed width font (array does not contain width data entries)
        width = pgm_read_byte(this->Font + FONT_FIXED_WIDTH);
    } else {
        // variable width font, read width data
        width = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c);
    }

    return width;
}

void DMD::setBuffer(uint8_t* newBuffer) {
    bDMDScreenRAM = newBuffer;
}
