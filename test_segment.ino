byte D1 = 7;
byte D2 = 4;
byte D3 = 3;
byte D4 = 8;

byte A = 6;
byte B = 2;
byte C = 10;
byte D = 12;
byte E = 13;
byte F = 5;
byte G = 9;
byte DP = 11; // decimal point

const int btn_start = A5;

#include "SevSeg.h"
SevSeg sevseg; // Initiate a seven-segment controller object

int startMillis = 0; // Variable to store the start time
bool counting = false; // Flag to indicate whether counting is active

void setup() {
    byte numDigits = 4;
    byte digitPins[] = {D1, D2, D3, D4};
    byte segmentPins[] = {A, B, C, D, E, F, G, DP};
    bool resistorsOnSegments = 0; 
    // Variable above indicates that 4 resistors were placed on the digit pins.
    // Set variable to 1 if you want to use 8 resistors on the segment pins.
    sevseg.begin(COMMON_ANODE, numDigits, digitPins, segmentPins, resistorsOnSegments);
    pinMode(btn_start, INPUT_PULLUP);
}

void loop() {
    int btnStartValue = digitalRead(btn_start);

    if (btnStartValue == LOW && !counting) {
        // Button pressed and not currently counting
        startMillis = millis(); // Reset the start time
        counting = true; // Set counting flag
    }
    if (counting) {
        // If counting, update the display with the elapsed time
        int currMillis_ALL = (millis() - startMillis) / 1000;
        sevseg.setNumber(currMillis_ALL, 0);
    }

    sevseg.refreshDisplay();
}
