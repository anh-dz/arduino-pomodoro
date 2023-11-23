#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ThreeWire.h>  
#include <RtcDS1302.h>

ThreeWire myWire(8,9,12); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

#define OLED_RESET -1

Adafruit_SSD1306 display(OLED_RESET);

const int led_work = A0;
const int led_rest = A2;
const int led_plan = A1;
const int buzzerPin = A3;
const int btn_start = 2;
const int btn_next = 4;
const int btn_co = 7;

int planTime = 5 * 60;
int workTime = 25 * 60;
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
  pinMode(btn_co, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);

  turn_off_led();
  Serial.begin(9600);
}

int btnStartValue;
int btnNextValue;
int btnCountValue;
char state = 'w';
int startMillis = 0;
unsigned int session = 0;

bool isStart = false;
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
int t;
RtcDateTime now;

void loop() {
  now = Rtc.GetDateTime();
  if(!counting && !isStart) {
    display.clearDisplay();
    t = printDateTime(now);
    display.setTextSize(1);
    display.setCursor(0, display.height()/2);
    switch (t) {
      case 0 ... 5:
        display.clearDisplay();
        break;
      case 6 ... 8:
        display.print("TIME TO\nGO TO SCHOOL!");
        break;
      case 12 ... 13:
        display.print("SHORT BREAK!");
        break;
      case 17 ... 19:
        display.print("WORK OUT\nPLAY PIANO");
        break;
      case 20 ... 22:
        display.print("DID YOU STUDY?");
        break;
      case 23 ... 24:
        display.print("PREPARE TO SLEEP!");
      default:
        display.print("YOUR FUTURE\nIN YOUR HANDS!");
        break;
    }
  }
  
  unsigned long currentMillis = millis();

  btnStartValue = digitalRead(btn_start);
  if (btnStartValue == LOW && currentMillis - previousMillis > 200) {
    previousMillis = currentMillis;  // reset the debouncing timer
    if (isStart || counting) {
      stop_pom();
    } else {
      session = 0;
      start_pom();
    }
  }

  btnNextValue = digitalRead(btn_next);
  if ((counting || isStart) && btnNextValue == LOW && currentMillis - previousMillis > debounceDelay) {
    previousMillis = currentMillis; // reset the debouncing timer
    if (counting) {
      counting = false;
      start_pom();
    }
    nextState();
  }

  btnCountValue = digitalRead(btn_co);
  if (btnCountValue == LOW && currentMillis - previousMillis > debounceDelay) {
    previousMillis = currentMillis;
    startMillis = millis()/1000; // Reset the start time
    if (counting) {
      counting = false;
    }else {
      if(isStart)
        isStart = false;
      counting = true; // Set counting flag
    }
  }

  if (counting) {
    startcounting();
  }

  myTime = millis()/1000;
  if (isStart && myTime >= endTime) {
    nextState();
  }

  if (isStart) {
    displayTime();
  }

  display.display();
}

void displayTime() {
  display.clearDisplay();
  printDateTime(now); // Move this line here
  unsigned long remainingTime = endTime - myTime;
  int minutes = remainingTime / 60;
  int seconds = remainingTime % 60;
  char buffer[5];
  sprintf(buffer, "%02d:%02d", minutes, seconds);
  display.setTextSize(2);
  display.setCursor(display.width()/2, display.height()/2);
  display.print(buffer);
}

void startcounting() {
  if (counting) {
    display.clearDisplay();
    printDateTime(now); // Move this line here
    int currMillis_ALL = millis()/1000 - startMillis;
    display.setTextSize(2);
    display.setCursor(0, display.height()/2);
    display.print(currMillis_ALL);
    if (currMillis_ALL % 600 == 0) playBuzzer();
  }
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

int printDateTime(const RtcDateTime& dt) {
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u %02u:%02u %02u"),
            dt.Day(),
            dt.Month(),
            dt.Hour(),
            dt.Minute(),
            session);
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(datestring);

    return dt.Hour();
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
  display.clearDisplay();
}

void start_pom() {
  if(isStart) return;
  isStart = true;
  next();
}

void stop_pom() {
  if(!isStart) return;
  isStart = false;
  counting = false;
  state = 'w';
  turn_off_led();
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
      ++session;
      break;
  }
}

void playBuzzer() {
  tone(buzzerPin, 1140, 250);
  delay(250*1.30);
  noTone(buzzerPin);
}