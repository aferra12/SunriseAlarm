#include <RTClib.h>
#include <Wire.h>

#define LEDPIN 3
#define ONBUTTON 2

#define HOUR 5
#define MINUTE 3

#define RAMP_DURATION 3600000

RTC_DS1307 rtc;

volatile bool turn_on_light = false;
unsigned long ramp_start_time = 0;

void setup() {
  pinMode(LEDPIN, OUTPUT);
  pinMode(ONBUTTON, INPUT_PULLUP);
  Serial.begin(57600);
  //rtc.adjust(DateTime(__DATE__, __TIME__));
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(2024, 7, 22, 22, 7, 0));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

}

void loop() {
  DateTime now = rtc.now();

  Serial.print(now.hour());
  Serial.println(now.minute());

  if ((now.minute() == MINUTE) & (now.hour() == HOUR)) {
    Serial.println("Sunrise Starting");
    turn_on_light = true;
    ramp_start_time = millis();
    
    if (turn_on_light) {
      unsigned long elapsed_time = millis() - ramp_start_time;

      while (elapsed_time < RAMP_DURATION) {
        float progress = (float)elapsed_time / RAMP_DURATION;
        //Serial.print("Progress: ");
        //Serial.println(progress);
        int brightness = exponentialRamp(progress);
        //Serial.print("Brightness: ");
        //Serial.println(brightness);
        analogWrite(LEDPIN, brightness);
        delay(2000);
        elapsed_time = millis() - ramp_start_time;
      }

      turn_on_light = false;
      analogWrite(LEDPIN, 255);
      Serial.println("Sunrise complete");
      }
    } else {
    analogWrite(LEDPIN, 0);
    }
}

int exponentialRamp(float x) {
  // Exponential function to mimic sunrise intensity
  float y = 255.0 * (1.0 - exp(-5 * x));
  return (int)y;
}

