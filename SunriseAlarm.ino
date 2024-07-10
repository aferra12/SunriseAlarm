#include <RTClib.h>
#include <Wire.h>

#define LEDPIN 3
#define ONBUTTON 2

#define HOUR 19
#define MINUTE 44

#define FADESPEED 5000

RTC_DS1307 rtc;

volatile bool is_power_on = true;  // State variable
volatile bool turn_on_light = false;
unsigned long last_toggle_time = 0;  // To debounce the button

void setup() {
  pinMode(LEDPIN, OUTPUT);
  pinMode(ONBUTTON, INPUT_PULLUP);
  Serial.begin(57600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  //analogWrite(LEDPIN, 0);

  attachInterrupt(digitalPinToInterrupt(ONBUTTON), toggleState, FALLING);
}

void loop() {

  if (is_power_on) {

    DateTime now = rtc.now();

    Serial.print(now.hour());
    Serial.println(now.minute());

    Serial.print(analogRead(LEDPIN));


    if ((now.minute() == MINUTE) & (now.hour() == HOUR)) {
      Serial.print("Made it");
      turn_on_light = true;
      for (int i = 0; i < 256; i++) {
        analogWrite(LEDPIN, i);
        delay(FADESPEED);
        if (i == 254) {
          turn_on_light = false;
        }
      }
    } else {
      // wait for one second
      delay(1000);
    }

  } else {
    analogWrite(LEDPIN, 0);
  }
}

void toggleState() {
  // Debounce
  if (millis() - last_toggle_time > 50) {
    is_power_on = !is_power_on;
    last_toggle_time = millis();
  }
}
