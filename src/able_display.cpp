#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "sevensegment.h"

sevensegment* sevenSegment;
unsigned char buf1[8], buf2[8], buf3[8], buf4[8];
bool dimmed = false;
bool prevDimmed = false;
char ableData[128];
char prevAbleData[128];
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
///
int fetchData() {
    FILE *pipe = popen("/home/pi/able-display/get_able_data.py", "r");
    if (!pipe) {
        return 9;
    }

    int bytes = fread(ableData, 1, 128, pipe);
    pclose(pipe);

    if (bytes > 6 && ableData[0] == '#') {
        // Found eth0 ip address
        return 5;
    }

    if (bytes != 36) {
        int err = checkInternet();
        if (err != 0) {
            sprintf(ableData, "Internet error: %d\n", err);
            printf(ableData);
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

    if (strncmp(prevAbleData, ableData, 35) != 0) {
        sevenSegment->writeSegData4(buf1, buf2, buf3, buf4);
        strncpy(prevAbleData, ableData, 35);
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

void displayIp() {
    dimmed = false;

    sevenSegment->blankSegData(buf1, 8, false);
    sevenSegment->blankSegData(buf2, 8, false);
    sevenSegment->blankSegData(buf3, 8, false);
    sevenSegment->blankSegData(buf4, 8, false);

    int displayPos = 0;
    unsigned char *bufCh;

    for (int pos = 1; ableData[pos] != '\n' && ableData[pos] != '\0'; pos++) {
        if (displayPos < 8) {
            bufCh = &buf1[displayPos];
        }
        else {
            bufCh = &buf2[displayPos - 8];
        }

        sevenSegment->getSegData(bufCh, 1, ableData[pos] - '0', 1);

        if (ableData[pos+1] == '.') {
            sevenSegment->decimalSegData(bufCh, 0);
            pos++;
        }

        displayPos++;
        int nextPos = displayPos % 8;
        if (nextPos == 2 || nextPos == 5) {
            displayPos++;
        }

    }

    updateDisplay();
}

///
/// main
///
int main(int argc, char **argv)
{
    printf("Running\n");

    *prevAbleData = '\0';

    sevenSegment = new sevensegment(true, 0);

    while (true) {
        int err = fetchData();

        if (err == 5) {
            // Connected to eth0 so show ip
            displayIp();
            errorDuration = 12;
            sleep(1);
            continue;
        }

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
