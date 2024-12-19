#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wiringPiSPI.h>

static const int Channel = 0;  // for CE0 (Channel = 1 for CE1)

int main()
{
    // Channel indicates chip select, 500000 indicates bus speed.
    wiringPiSPISetup(Channel, 10000000);

    printf("Enter a hex string in format xxyyxxyyxxyy...\n");
    printf("where xx = register and yy = data, e.g. 09ff0a0f\n\n");
    printf("For register/data info see section 'The MAX7219 registers' of:\n");
    printf("https://www.engineersgarage.com/microcontroller-projects/articles-arduino-seven-segment-multiplexing-max7219-spi/\n\n");

    char hex[256];
    unsigned char regData[2];

    while (1) {
        printf("Hex (register/data): ");
        scanf("%s", hex);
        if (strcmp(hex, "exit") == 0) {
            exit(1);
        }
        else if (strcmp(hex, "init") == 0) {
            strcpy(hex, "0f000c010a0f09ff0b07010f020f030f040f050f060f070f080f");
        }

        int strLen = strlen(hex);
        char ch;
        int num;

        for (int i = 0; i < strLen; i += 4) {
            ch = hex[i+4];
            hex[i+4] = '\0';
            num = strtol(&hex[i], NULL, 16);
            hex[i+4] = ch;

            regData[0] = (num & 0xff00) >> 8;
            regData[1] = num & 0x00ff;
            wiringPiSPIDataRW(Channel, regData, 2);
        }
    }
}
