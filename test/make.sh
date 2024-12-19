echo Building
g++ -o mytest -I . \
    mytest.cpp \
    -lwiringPi -lpthread || exit
echo Done
