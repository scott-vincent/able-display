#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "sevensegment.h"

///
/// This class allows you to drive a daisy-chained
/// set of 8 digit 7-segment displays using SPI.
/// 
/// Wiring
/// ------
/// VCC - 3.3v = Pin 1 (does not work correctly if 5v used!)
/// GND - Ground = Pin 6
/// DIN - GPIO 10 (MOSI) = Pin 19
/// CS - GPIO 8 (CE0) = Pin 24
/// CLK - GPIO 11 (SCLK) = Pin 23
/// DOUT - Not connected unless daisy chaining (see below)
/// 
/// To daisy-chain connect DOUT from one module to DIN on
/// the next one. Connect all other pins in parallel.
///
/// Note: SPI must be enabled on the Pi and we only want a
/// single channel with no MISO (to save pins) so you must
/// edit /boot/config.txt and add "dtoverlay=spi0-1cs,no_miso".
/// Don't use "dtparm=spi=on" as this uses up 2 extra pins.
///

/// <summary>
/// Specify Channel 0 if using CE0 or channel 1 if using CE1.
/// </summary>
sevensegment::sevensegment(bool initWiringPi, int spiChannel)
{
    char hostname[256];
    char hex[256];

    gethostname(hostname, 256);

    channel = spiChannel;

    // Caller may want to initialise wiringPi themselves
    if (initWiringPi) {
        // Init wiring pi, needed for delayMicroseconds
        wiringPiSetupGpio();
    }

    // Init wiring pi SPI, get corruption at 10 MHz so use 1 MHz
    wiringPiSPISetup(channel, 1000000);

    // Intialise all displays. Displays hyphens to show
    // displays have been initialised successfully.

    if (strcmp(hostname, "able-display-alx") == 0) {
        // Set brightness to 1, i.e. '0a01'
        strcpy(hex, "0f000c010a0109ff0b0701880288038804880588068807880888");
        writeSegHex(1, hex);
        writeSegHex(2, hex);
        writeSegHex(3, hex);
        // Set brightness to max, i.e. '0a0f'
        strcpy(hex, "0f000c010a0f09ff0b0701880288038804880588068807880888");
        writeSegHex(4, hex);
    }
    else {
        // Set brightness to 4, i.e. '0a04'
        strcpy(hex, "0f000c010a0409ff0b0701880288038804880588068807880888");
        writeSegHex(1, hex);
        writeSegHex(2, hex);
        writeSegHex(3, hex);
        writeSegHex(4, hex);
    }

    // Clear displays after a short delay
    delayMicroseconds(1500000);
    strcpy(hex, "010f020f030f040f050f060f070f080f");
    writeSegHex(1, hex);
    writeSegHex(2, hex);
    writeSegHex(3, hex);
    writeSegHex(4, hex);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            prevDisplay[i][j] = 0x0f;
        }
    }
}

void sevensegment::dim(int display, bool wantDim)
{
    char hex[16];

    if (wantDim) {
        strcpy(hex, "0a01");
    }
    else {
        strcpy(hex, "0a04");
    }

    writeSegHex(display, hex);
}

/// <summary>
// Converts a number to segment display data.
// Leading zeroes are added up to fixedDigits size.
// Minus sign added if number negative and bufSize > fixedDigits.
/// </summary>
void sevensegment::getSegData(unsigned char* buf, int bufSize, int num, int fixedSize)
{
    bool minus = false;

    if (num < 0) {
        minus = true;
        num = -num;
    }

    // Work back from least significant digit
    int pos = bufSize - 1;
    for (; pos >= 0; pos--) {
        buf[pos] = num % 10;
        num /= 10;
        if (num == 0) {
            pos--;
            break;
        }
    }

    // Add leading zeroes if required
    if (pos >= 0 && fixedSize > 0) {
        int firstDigit = bufSize - fixedSize;
        for (; pos >= firstDigit; pos--) {
            // Pad with zero
            buf[pos] = 0;
        }
    }

    // If there is still room add minus sign and blanks
    if (pos >= 0) {
        if (minus) {
            buf[pos] = 0x0a;
            pos--;
        }

        for (; pos >= 0; pos--) {
            // Pad with blank
            buf[pos] = 0x0f;
        }
    }
}

/// <summary>
/// Blanks the display at the specified position.
/// If wantMinus is true minus signs are displayed instead of blanks.
/// </summary>
void sevensegment::blankSegData(unsigned char* buf, int bufSize, bool wantMinus)
{
    for (int pos = 0; pos < bufSize; pos++) {
        if (wantMinus) {
            buf[pos] = 0x0a;
        }
        else {
            buf[pos] = 0x0f;
        }
    }
}

void sevensegment::error(unsigned char *buf, int errNum)
{
    for (int pos = 0; pos < 6; pos++) {
        buf[pos] = 0x0f;
    }

    buf[6] = 0x0b;
    buf[7] = errNum;
}

/// <summary>
/// Adds a decimal point at the specified position.
/// </summary>
void sevensegment::decimalSegData(unsigned char* buf, int pos)
{
    buf[pos] = buf[pos] | 0x80;
}

/// <summary>
/// Write hex data to the specified display.
/// Note: display 1 is the first display.
/// </summary>
void sevensegment::writeSegHex(int display, char* hex)
{
    unsigned char regData[8];
    int strLen = strlen(hex);
    char ch;
    int num;

    display = 5 - display;

    for (int i = 0; i < strLen; i += 4) {
        ch = hex[i + 4];
        hex[i + 4] = '\0';
        num = strtol(&hex[i], NULL, 16);
        hex[i + 4] = ch;

        int dataLen = 0;
        for (int j = 1; j <= 4; j++) {
            if (j == display) {
                regData[dataLen] = (num & 0xff00) >> 8;
                regData[dataLen + 1] = num & 0x00ff;
            }
            else {
                // No-op for this display
                regData[dataLen] = 0;
                regData[dataLen + 1] = 0;
            }

            dataLen += 2;
        }

        wiringPiSPIDataRW(channel, regData, dataLen);
        delayMicroseconds(500);
    }
}

/// <summary>
/// Takes 4 complete data buffers (8 chars each)
/// and writes them to 4 displays concurrently.
/// buf1 = top left display, buf2 = top right display, buf3 = bottom left display, buf4 = bottom right display.
/// </summary>
void sevensegment::writeSegData4(unsigned char* buf1, unsigned char* buf2, unsigned char* buf3, unsigned char* buf4)
{
    writeSegData(0, buf1);
    writeSegData(1, buf2);
    writeSegData(2, buf3);
    writeSegData(3, buf4);
}

/// <summary>
/// Update a display but only write the digits that have changed.
/// Display must be 0 (top left), 1 (top right), 2 (bottom left), 3 (bottom right)
/// </summary>
void sevensegment::writeSegData(int display, unsigned char* buf)
{
    unsigned char regData[8];

    display = 3 - display; 

    for (int i = 0; i < 8; i++) {
        if (buf[i] != prevDisplay[display][i]) {
            prevDisplay[display][i] = buf[i];

            // Set no-op for all displays
            regData[0] = 0;
            regData[2] = 0;
            regData[4] = 0;
            regData[6] = 0;

            // Replace no-op with digit for required display
            // 1 = rightmost digit
            regData[display * 2] = 8 - i;
            regData[display * 2 + 1] = buf[i];

            // Send the data for single digit, single display
            wiringPiSPIDataRW(channel, regData, 8);
            delayMicroseconds(500);
        }
    }
}
