echo Building
g++ -o ../able_display -I . \
    sevensegment.cpp \
    able_display.cpp \
    -lwiringPi -lpthread || exit
echo Done
