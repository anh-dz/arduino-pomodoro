#include "SevSeg.h"
SevSeg sevseg; //Initiate a seven segment controller object

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

const int led_work = A0;
const int led_rest = A2;
const int led_plan = A1;
const int buzzerPin = A3;
const int btn_start = A5;
const int btn_next = A4;
// const int btn_count = 1;

int planTime = 5 * 60;
int workTime = 30 * 60;
int restTime = 5 * 60;
int minuteAdd = 5 * 60;

unsigned long endTime;
unsigned long myTime;

void setup() {
  pinMode(led_work, OUTPUT);
  pinMode(led_rest, OUTPUT);
  pinMode(led_plan, OUTPUT);
  pinMode(btn_start, INPUT_PULLUP);
  pinMode(btn_next, INPUT_PULLUP);
  // pinMode(btn_count, INPUT_PULLUP);

  turn_off_led();

  byte numDigits = 4;  
  byte digitPins[] = {D1, D2, D3, D4};
  byte segmentPins[] = {A, B, C, D, E, F, G, DP};
  bool resistorsOnSegments = 0;
  sevseg.begin(COMMON_ANODE, numDigits, digitPins, segmentPins, resistorsOnSegments);
  Serial.begin(9600);
}

int btnStartValue;
int btnNextValue;
int isStart = false;
char state = 'w';
int startMillis = 0;
bool counting = false;
/*
state: w: work
       p: plan
       r: rest
when isStart = false => state = w
*/

unsigned long previousMillis = 0; // will store last time the display was updated
const long updateInterval = 200; // interval at which to refresh the display (milliseconds)
const long debounceDelay = 50; // debounce time for buttons

void loop() {
  unsigned long currentMillis = millis();

  btnStartValue = digitalRead(btn_start);
  if (btnStartValue == LOW && currentMillis - previousMillis > 200) {
    previousMillis = currentMillis;  // reset the debouncing timer
    if (isStart) {
      stop_pom();
    } else {
      start_pom();
    }
  }

  btnNextValue = digitalRead(btn_next);
  if (isStart && btnNextValue == LOW && currentMillis - previousMillis > debounceDelay) {
    previousMillis = currentMillis; // reset the debouncing timer
    nextState();
  }

  // int btnCountValue = digitalRead(btn_count);
  // Serial.println(btnCountValue);
  // if (btnCountValue == LOW && currentMillis - previousMillis > 200) {
  //   previousMillis = currentMillis;
  //   if (counting) {
  //     counting = false;
  //   }else {
  //     startMillis = millis(); // Reset the start time
  //     counting = true; // Set counting flag
  //     stop_pom();
  //     startcounting();
  //   }
  // }

  myTime = millis()/1000;
  if (isStart && myTime >= endTime) {
    nextState();
  }

  if (isStart) {
    displayTime();
  }

  sevseg.refreshDisplay();
}

void displayTime() {
  unsigned long remainingTime = endTime - myTime; // endTime is a global variable

  int minutes = remainingTime / 60;
  int seconds = remainingTime % 60;

  char buffer[5]; // Buffer to hold the display string
  sprintf(buffer, "%02d.%02d", minutes, seconds);
  sevseg.setChars(buffer);
}

void startcounting() {
  if (counting) {
    int currMillis_ALL = (millis() - startMillis) / 1000;
    sevseg.setNumber(currMillis_ALL, 0);
  }
}

void turn_on_led(int led_on, int led_off1, int led_off2) {
  digitalWrite(led_on, HIGH);
  digitalWrite(led_off1, LOW);
  digitalWrite(led_off2, LOW);
}

void turn_off_led() {
  digitalWrite(led_work, LOW);
  digitalWrite(led_rest, LOW);
  digitalWrite(led_plan, LOW);
}

void start_pom() {
  if(isStart) return;
  isStart = true;
  next();
}

void stop_pom() {
  if(!isStart) return;
  isStart = false;
  state = 'w';
  turn_off_led();
  sevseg.setChars("");
}

void nextState() {
  switch (state) {
    case 'w':
      Serial.println("Switching to Plan");
      state = 'p';
      break;
    case 'p':
      Serial.println("Switching to Rest");
      state = 'r';
      break;
    case 'r':
      Serial.println("Switching to Work");
      state = 'w';
      turn_off_led();
      playBuzzer();
      turn_on_led(led_rest, led_work, led_plan);
      playBuzzer();
      turn_off_led();
      playBuzzer();
      turn_on_led(led_rest, led_work, led_plan);
      playBuzzer();
      turn_off_led();
      break;
  }
  next(); // Set up the new state
}

void next() {
  playBuzzer();
  switch (state) {
    case 'w':
      turn_on_led(led_work, led_plan, led_rest);
      myTime = millis()/1000;
      endTime = myTime + workTime;
      break;
    case 'p':
      turn_on_led(led_plan, led_work, led_rest);
      myTime = millis()/1000;
      endTime = myTime + planTime;
      break;
    case 'r':
      turn_on_led(led_rest, led_work, led_plan);
      myTime = millis()/1000;
      endTime = myTime + restTime;
      break;
  }
}

void addminute(int m) {
  /*
  1. break the count time while in next, keep state, keep led
  2. Get the time now and end time from the count
  3. count(now+50/1000, end + m)
  */
}

void playBuzzer() {
  tone(buzzerPin, 1140, 250);
  delay(250*1.30);
  noTone(buzzerPin);
}