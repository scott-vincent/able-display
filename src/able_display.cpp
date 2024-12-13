#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sevensegment.h"

sevensegment* sevenSegment;
unsigned char buf1[8], buf2[8], buf3[8], buf4[8];
unsigned char prevBuf1[8], prevBuf2[8], prevBuf3[8], prevBuf4[8];
bool dimmed = false;
bool prevDimmed = false;
char ableData[64];
int errorDuration = 0;

///
// Returns 0 if internet is working, 1 if it isn't or 9 if internal error
///
int checkInternet() {
    FILE *pipe = popen("ping -c1 google.com >/dev/null 2>&1 && echo Yes || echo No", "r");
    if (!pipe) {
        return 9;
    }

    char buf[2];
    fread(buf, 2, 1, pipe);
    pclose(pipe);

    if (buf[0] == 'Y') {
         return 0;
    }

    return 1;
}

///
// Returns 0 on success
// Errors:
//   1 = No internet
//   2 = Failed to connect to Tailscale server (wrong URL?)
//   3 = Tailscale error (funnel not enabled?)
//   4 = File not found (raspi5:/home/pi/flightradar_able/able_data2)
//   9 = Internal error
///
int fetchData() {
    FILE *pipe = popen("/home/pi/able_display2/able_data_client.py", "r");
    if (!pipe) {
        return 9;
    }

    int bytes = fread(ableData, 1, 64, pipe);
    pclose(pipe);

    if (bytes != 36) {
        int err = checkInternet();
        if (err != 0) {
            printf("Internet error: %d\n", err);
            return err;
        }

        printf("Server error: %s\n", ableData);

        if (strncmp(ableData, "Failed to connect", 17) == 0) {
            return 2;
        }
        else if (strncmp(ableData, "Tailscale error", 15) == 0) {
            return 3;
        }
        else if (strncmp(ableData, "File not found", 14) == 0) {
            return 4;
        }

        return 4;
    }

    return 0;
}

void updateDisplay() {
    if (dimmed != prevDimmed) {
        if (dimmed) {
            sevenSegment->dim(4, true);
        }
        else {
            sevenSegment->dim(4, false);
        }

        prevDimmed = dimmed;
    }

    if (strncmp((char*)buf1, (char*)prevBuf1, 8) != 0 || strncmp((char*)buf2, (char*)prevBuf2, 8) != 0 ||
        strncmp((char*)buf3, (char*)prevBuf3, 8) != 0 || strncmp((char*)buf4, (char*)prevBuf4, 8) != 0) {
        sevenSegment->writeSegData4(buf1, buf2, buf3, buf4);
        strncpy((char*)prevBuf1, (char*)buf1,  8);
        strncpy((char*)prevBuf2, (char*)buf2,  8);
        strncpy((char*)prevBuf3, (char*)buf3,  8);
        strncpy((char*)prevBuf4, (char*)buf4,  8);
    }
}

void writeBuffer(unsigned char *buf, char *data) {
    sevenSegment->blankSegData(buf, 8, false);

    if (data[0] != '-') {
        sevenSegment->getSegData(buf, 2, atoi(&data[0]), 2);
    }

    if (data[3] != '-') {
        sevenSegment->getSegData(&buf[3], 2, atoi(&data[3]), 2);
    }

    if (data[6] != '-') {
        sevenSegment->getSegData(&buf[6], 2, atoi(&data[6]), 2);
    }
}

void displayData() {
    if (strncmp(ableData, "--,--,--,--,--,--,--,--,--,--,--,--", 35) == 0) {
        // Display a dot in the bottom right hand corner
        dimmed = true;
        sevenSegment->blankSegData(buf1, 8, false);
        sevenSegment->blankSegData(buf2, 8, false);
        sevenSegment->blankSegData(buf3, 8, false);
        sevenSegment->blankSegData(buf4, 8, false);
        sevenSegment->decimalSegData(buf4, 7);
    }
    else {
        dimmed = false;
        writeBuffer(buf1, ableData);
        writeBuffer(buf2, &ableData[9]);
        writeBuffer(buf3, &ableData[18]);
        writeBuffer(buf4, &ableData[27]);
    }

    updateDisplay();
}

void displayError(int err) {
    dimmed = false;
    sevenSegment->blankSegData(buf1, 8, false);
    sevenSegment->blankSegData(buf2, 8, false);
    sevenSegment->blankSegData(buf3, 8, false);
    sevenSegment->error(buf4, err);

    updateDisplay();
}

///
/// main
///
int main(int argc, char **argv)
{
    printf("Running\n");

    prevBuf1[0] = 'x';
    prevBuf2[0] = 'x';
    prevBuf3[0] = 'x';
    prevBuf4[0] = 'x';

    sevenSegment = new sevensegment(true, 0);

    while (true) {
        int err = fetchData();

        if (err == 0) {
            errorDuration = 0;
        }
        else if (errorDuration < 12) {
            // Don't show temporary error
            strncpy(ableData, "--,--,--,--,--,--,--,--,--,--,--,--", 35);
            err = 0;
        }

        if (err == 0) {
            displayData();
        }
        else {
            displayError(err);
        }

        sleep(3);
        errorDuration += 3;
    }

    return 0;
}
